// ============================================================
// File: UEF/UEF_Drivers/UEF_UART/UEF_uart.h
// ============================================================
// PURPOSE: Universal UART driver interface
//          Works on ANY MCU through ChipLink.h
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#ifndef UEF_UART_H
#define UEF_UART_H

#include <stdint.h>
#include <stdbool.h>
#include "ChipLink.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// UART Configuration Types
// ============================================================

typedef enum {
    UEF_UART_PARITY_NONE = 0,
    UEF_UART_PARITY_EVEN = 1,
    UEF_UART_PARITY_ODD = 2
} UEF_UART_Parity_t;

typedef enum {
    UEF_UART_STOP_1 = 0,
    UEF_UART_STOP_2 = 1
} UEF_UART_StopBits_t;

typedef enum {
    UEF_UART_DATA_7 = 0,
    UEF_UART_DATA_8 = 1,
    UEF_UART_DATA_9 = 2
} UEF_UART_DataBits_t;

typedef struct {
    uint8_t instance;           // UART number (1, 2, 3, etc.)
    uint32_t baudrate;          // Baud rate (9600, 115200, etc.)
    UEF_UART_DataBits_t data_bits;
    UEF_UART_StopBits_t stop_bits;
    UEF_UART_Parity_t parity;
    bool enable_tx;
    bool enable_rx;
    bool enable_interrupts;
} UEF_UART_Config_t;

// Forward declaration
typedef struct UEF_UART_Handle UEF_UART_Handle_t;

struct UEF_UART_Handle {
    void* instance;             // Hardware instance
    uint8_t instance_num;       // UART number
    bool is_initialized;
    UEF_UART_Config_t config;
    
    // State
    volatile bool tx_busy;
    volatile bool rx_busy;
    volatile uint32_t error_flags;
    
    // Callbacks
    void (*tx_complete_cb)(UEF_UART_Handle_t* handle);
    void (*rx_complete_cb)(UEF_UART_Handle_t* handle);
    void (*error_cb)(UEF_UART_Handle_t* handle, uint32_t error);
    void (*rx_byte_cb)(UEF_UART_Handle_t* handle, uint8_t byte);
};

// ============================================================
// UART API - UNIVERSAL FUNCTIONS
// ============================================================

// Initialization
int UEF_UART_Init(UEF_UART_Handle_t* handle, const UEF_UART_Config_t* config);
int UEF_UART_DeInit(UEF_UART_Handle_t* handle);

// Configuration
int UEF_UART_SetBaudRate(UEF_UART_Handle_t* handle, uint32_t baudrate);
void UEF_UART_Enable(UEF_UART_Handle_t* handle);
void UEF_UART_Disable(UEF_UART_Handle_t* handle);

// Transmit functions
int UEF_UART_SendByte(UEF_UART_Handle_t* handle, uint8_t byte);
int UEF_UART_SendBytes(UEF_UART_Handle_t* handle, const uint8_t* data, uint16_t size);
int UEF_UART_SendString(UEF_UART_Handle_t* handle, const char* str);
int UEF_UART_SendByte_IT(UEF_UART_Handle_t* handle, uint8_t byte);
int UEF_UART_SendBytes_IT(UEF_UART_Handle_t* handle, const uint8_t* data, uint16_t size);

// Receive functions
int UEF_UART_ReceiveByte(UEF_UART_Handle_t* handle, uint8_t* byte);
int UEF_UART_ReceiveBytes(UEF_UART_Handle_t* handle, uint8_t* buffer, uint16_t size);
int UEF_UART_ReceiveByte_IT(UEF_UART_Handle_t* handle, uint8_t* byte);

// Status
bool UEF_UART_IsTxReady(UEF_UART_Handle_t* handle);
bool UEF_UART_IsRxReady(UEF_UART_Handle_t* handle);
bool UEF_UART_IsTxComplete(UEF_UART_Handle_t* handle);
uint32_t UEF_UART_GetErrorFlags(UEF_UART_Handle_t* handle);
void UEF_UART_ClearErrorFlags(UEF_UART_Handle_t* handle);

// Callback registration
void UEF_UART_RegisterCallbacks(UEF_UART_Handle_t* handle,
                               void (*tx_cb)(UEF_UART_Handle_t*),
                               void (*rx_cb)(UEF_UART_Handle_t*),
                               void (*err_cb)(UEF_UART_Handle_t*, uint32_t));

// Interrupt handler (to be called from ISR)
void UEF_UART_IRQHandler(UEF_UART_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif // UEF_UART_H