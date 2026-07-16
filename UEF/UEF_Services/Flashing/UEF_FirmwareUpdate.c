// ============================================================
// ONE FILE FIRMWARE UPDATE - Everything you need!
// ============================================================

#include <stdint.h>
#include <string.h>
#include "UEF_uart.h"
#include "ChipLink_config.h"

// ============================================================
// SECTION 1: FLASH HARDWARE FUNCTIONS (Direct register access)
// ============================================================

#define FLASH_KEY1              0x45670123UL
#define FLASH_KEY2              0xCDEF89ABUL

// Unlock flash for programming
static void Flash_Unlock(void) {
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

// Lock flash after programming
static void Flash_Lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

// Wait for flash operation to complete
static int Flash_Wait(uint32_t timeout_ms) {
    uint32_t start = UEF_GetTick();  // Changed from HAL_GetTick to UEF_GetTick
    while (FLASH->SR & FLASH_SR_BSY) {
        if ((UEF_GetTick() - start) > timeout_ms) {  // Changed from HAL_GetTick to UEF_GetTick
            return -1;  // Timeout
        }
    }
    return 0;
}

// Erase a flash sector (16KB on STM32F4)
static int Flash_EraseSector(uint32_t sector_num) {
    Flash_Unlock();
    
    // Configure for sector erase
    FLASH->CR |= FLASH_CR_SER;
    FLASH->CR &= ~FLASH_CR_SNB_Msk;
    FLASH->CR |= (sector_num << FLASH_CR_SNB_Pos);
    FLASH->CR |= FLASH_CR_STRT;
    
    // Wait for completion
    if (Flash_Wait(10000) != 0) {
        return -1;
    }
    
    // Clear flags
    FLASH->SR = 0;
    FLASH->CR &= ~FLASH_CR_SER;
    Flash_Lock();
    
    return 0;
}

// Program a 32-bit word to flash
static int Flash_ProgramWord(uint32_t address, uint32_t data) {
    Flash_Unlock();
    
    // Check if erased (optional)
    if (*(uint32_t*)address != 0xFFFFFFFF) {
        return -1;  // Not erased
    }
    
    FLASH->CR |= FLASH_CR_PG;
    *(uint32_t*)address = data;
    
    if (Flash_Wait(1000) != 0) {
        return -1;
    }
    
    FLASH->CR &= ~FLASH_CR_PG;
    Flash_Lock();
    
    return 0;
}

// ============================================================
// SECTION 2: FIRMWARE UPDATE LOGIC
// ============================================================

// Firmware header structure
typedef struct {
    uint32_t magic;      // Should be 0xDEADBEEF
    uint32_t version;
    uint32_t size;
    uint32_t checksum;   // Simple CRC32
} Firmware_Header_t;

#define FIRMWARE_MAGIC  0xDEADBEEF
#define APP_START_ADDR  0x08008000  // Application start address

// State machine states
typedef enum {
    STATE_IDLE = 0,
    STATE_WAITING_HEADER,
    STATE_RECEIVING_DATA,
    STATE_PROGRAMMING,
    STATE_COMPLETE,
    STATE_ERROR
} UpdateState_t;

// Update context
typedef struct {
    UpdateState_t state;
    Firmware_Header_t header;
    uint32_t received_bytes;
    uint32_t programmed_bytes;
    uint8_t* buffer;
    uint32_t buffer_size;
    uint32_t error_code;
    
    // UART handle for sending ACK/NAK
    UEF_UART_Handle_t* uart_handle;
    
    // Callbacks
    void (*on_progress)(uint32_t percent);
    void (*on_complete)(int success);
} UpdateContext_t;

static UpdateContext_t ctx;

// ============================================================
// SECTION 3: PUBLIC API (Only 5 functions!)
// ============================================================

/**
 * @brief Initialize firmware update system
 * @param buffer Buffer to hold incoming data
 * @param buffer_size Size of buffer
 */
void UEF_FirmwareUpdate_Init(uint8_t* buffer, uint32_t buffer_size) {
    ctx.state = STATE_IDLE;
    ctx.received_bytes = 0;
    ctx.programmed_bytes = 0;
    ctx.buffer = buffer;
    ctx.buffer_size = buffer_size;
    ctx.error_code = 0;
    ctx.uart_handle = NULL;
    
    // Clear application area
    memset((void*)APP_START_ADDR, 0xFF, 0x78000);  // 480KB
}

/**
 * @brief Set UART handle for sending ACK/NAK
 * @param handle UART handle
 */
void UEF_FirmwareUpdate_SetUartHandle(UEF_UART_Handle_t* handle) {
    ctx.uart_handle = handle;
}

/**
 * @brief Start a firmware update
 * @param total_size Expected size of firmware
 */
void UEF_FirmwareUpdate_Start(uint32_t total_size) {
    ctx.state = STATE_WAITING_HEADER;
    ctx.received_bytes = 0;
    ctx.header.size = total_size;
    
    // Erase flash sectors where app lives
    for (int sector = 2; sector <= 11; sector++) {
        Flash_EraseSector(sector);
        if (ctx.on_progress) {
            ctx.on_progress((sector - 1) * 10);  // 0-100%
        }
    }
}

/**
 * @brief Process incoming UART data (call from UART callback)
 * @param data Pointer to received data
 * @param length Number of bytes received
 */
void UEF_FirmwareUpdate_ProcessData(const uint8_t* data, uint32_t length) {
    if (ctx.state == STATE_ERROR) return;
    
    uint8_t ack = 0x06;
    uint8_t nak = 0x15;
    
    // State machine
    switch (ctx.state) {
        case STATE_WAITING_HEADER:
            // Wait for header (16 bytes)
            if (ctx.received_bytes < sizeof(Firmware_Header_t)) {
                uint32_t remaining = sizeof(Firmware_Header_t) - ctx.received_bytes;
                uint32_t copy = (length < remaining) ? length : remaining;
                memcpy((uint8_t*)&ctx.header + ctx.received_bytes, data, copy);
                ctx.received_bytes += copy;
                
                // Check if header complete
                if (ctx.received_bytes >= sizeof(Firmware_Header_t)) {
                    if (ctx.header.magic == FIRMWARE_MAGIC) {
                        ctx.state = STATE_RECEIVING_DATA;
                        ctx.received_bytes = 0;
                        
                        // Send ACK
                        if (ctx.uart_handle) {
                            UEF_UART_Send(ctx.uart_handle, &ack, 1, 100);
                        }
                    } else {
                        ctx.state = STATE_ERROR;
                        ctx.error_code = 1;  // Invalid magic
                        
                        // Send NAK
                        if (ctx.uart_handle) {
                            UEF_UART_Send(ctx.uart_handle, &nak, 1, 100);
                        }
                    }
                }
            }
            break;
            
        case STATE_RECEIVING_DATA: {
            // Store data in buffer
            uint32_t remaining = ctx.header.size - ctx.received_bytes;
            uint32_t copy = (length < remaining) ? length : remaining;
            
            if (ctx.buffer) {
                memcpy(ctx.buffer + ctx.received_bytes, data, copy);
            }
            
            ctx.received_bytes += copy;
            
            // Check if all data received
            if (ctx.received_bytes >= ctx.header.size) {
                ctx.state = STATE_PROGRAMMING;
                ctx.programmed_bytes = 0;
                
                // Send ACK
                if (ctx.uart_handle) {
                    UEF_UART_Send(ctx.uart_handle, &ack, 1, 100);
                }
            }
            
            // Progress callback (every 10%)
            if (ctx.on_progress && (ctx.received_bytes * 100 / ctx.header.size) % 10 == 0) {
                ctx.on_progress(ctx.received_bytes * 100 / ctx.header.size);
            }
            break;
        }
            
        case STATE_PROGRAMMING: {
            // Program flash in chunks
            uint32_t chunk_size = 512;  // 512 bytes at a time
            uint32_t remaining = ctx.header.size - ctx.programmed_bytes;
            uint32_t chunk = (remaining < chunk_size) ? remaining : chunk_size;
            
            // Program words (4 bytes at a time)
            for (uint32_t i = 0; i < chunk; i += 4) {
                uint32_t address = APP_START_ADDR + ctx.programmed_bytes + i;
                uint32_t word_data = *(uint32_t*)(ctx.buffer + ctx.programmed_bytes + i);
                Flash_ProgramWord(address, word_data);
            }
            
            ctx.programmed_bytes += chunk;
            
            // Check if programming complete
            if (ctx.programmed_bytes >= ctx.header.size) {
                ctx.state = STATE_COMPLETE;
                
                // Send success
                if (ctx.uart_handle) {
                    UEF_UART_Send(ctx.uart_handle, &ack, 1, 100);
                }
                
                if (ctx.on_complete) {
                    ctx.on_complete(1);
                }
            }
            break;
        }
            
        default:
            break;
    }
}

/**
 * @brief Get current progress (0-100)
 */
uint32_t UEF_FirmwareUpdate_GetProgress(void) {
    if (ctx.state == STATE_IDLE) return 0;
    if (ctx.state == STATE_COMPLETE) return 100;
    if (ctx.state == STATE_ERROR) return 0;
    
    return (ctx.received_bytes * 100) / ctx.header.size;
}

/**
 * @brief Register callbacks
 */
void UEF_FirmwareUpdate_RegisterCallbacks(
    void (*on_progress)(uint32_t percent),
    void (*on_complete)(int success)) {
    ctx.on_progress = on_progress;
    ctx.on_complete = on_complete;
}

/**
 * @brief Get error code
 */
uint32_t UEF_FirmwareUpdate_GetError(void) {
    return ctx.error_code;
}