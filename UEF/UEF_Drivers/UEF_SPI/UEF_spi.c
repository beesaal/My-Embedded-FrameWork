// ============================================================
// File: UEF/UEF_Drivers/UEF_SPI/UEF_spi.c
// ============================================================
// PURPOSE: Universal SPI driver implementation
//          Uses ONLY ChipLink.h - 100% portable
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#include "UEF_spi.h"
#include "ChipLink.h"
#include <string.h>

// ============================================================
// PRIVATE HELPERS - Call ONLY ChipLink functions
// ============================================================

static int UEF_SPI_WaitForFlag(void* spi, uint32_t flag, uint32_t timeout_ms) {
    uint32_t start = UEF_GetTick();
    while (!UEF_SPI_GET_FLAG(spi, flag)) {
        if ((UEF_GetTick() - start) > timeout_ms) {
            return -1;
        }
    }
    return 0;
}

static int UEF_SPI_WaitForTXReady(void* spi, uint32_t timeout_ms) {
    return UEF_SPI_WaitForFlag(spi, UEF_SPI_FLAG_TXE, timeout_ms);
}

static int UEF_SPI_WaitForRXReady(void* spi, uint32_t timeout_ms) {
    return UEF_SPI_WaitForFlag(spi, UEF_SPI_FLAG_RXNE, timeout_ms);
}

static int UEF_SPI_WaitForNotBusy(void* spi, uint32_t timeout_ms) {
    uint32_t start = UEF_GetTick();
    while (UEF_SPI_IS_BUSY(spi)) {
        if ((UEF_GetTick() - start) > timeout_ms) {
            return -1;
        }
    }
    return 0;
}

// ============================================================
// INITIALIZATION
// ============================================================

int UEF_SPI_Init(UEF_SPI_Handle_t* handle, const UEF_SPI_Config_t* config) {
    if (!handle || !config) return -1;
    if (config->instance == 0) return -2;

    // Get hardware instance via ChipLink
    void* spi = UEF_SPI_GET(config->instance);
    if (!spi) return -3;

    // Store config
    handle->instance = spi;
    handle->instance_num = config->instance;
    handle->config = *config;
    handle->is_initialized = false;
    handle->tx_busy = false;
    handle->rx_busy = false;
    handle->busy = false;
    handle->error_flags = 0;
    handle->tx_complete_cb = NULL;
    handle->rx_complete_cb = NULL;
    handle->error_cb = NULL;

    // Enable clock via ChipLink
    UEF_SPI_CLK_ENABLE(config->instance);

    // Configure pins via ChipLink
    if (UEF_SPI_ConfigurePins(config->instance) != 0) {
        return -4;
    }

    // Disable SPI for configuration via ChipLink
    UEF_SPI_DISABLE(spi);
    UEF_SPI_CLEAR_ERRORS(spi);

    // Configure master/slave via ChipLink
    UEF_SPI_SET_MASTER(spi, config->is_master);

    // Configure mode via ChipLink
    UEF_SPI_SET_MODE(spi, config->mode);

    // Configure baud rate via ChipLink
    UEF_SPI_SET_BAUD_DIV(spi, config->baudrate);

    // Configure data width via ChipLink
    UEF_SPI_SET_DATA_WIDTH(spi, config->data_width == UEF_SPI_DATA_16BIT);

    // Configure bit order via ChipLink
    UEF_SPI_SET_LSB_FIRST(spi, config->lsb_first);

    // Enable interrupts via ChipLink
    if (config->enable_interrupts) {
        UEF_SPI_ENABLE_RX_INTERRUPT(spi);
        UEF_SPI_ENABLE_ERROR_INTERRUPT(spi);
        UEF_SPI_ENABLE_IRQ(config->instance);
    }

    // Enable SPI via ChipLink
    UEF_SPI_ENABLE(spi);

    handle->is_initialized = true;
    return 0;
}

int UEF_SPI_DeInit(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;

    void* spi = handle->instance;

    // Disable SPI via ChipLink
    UEF_SPI_DISABLE(spi);
    UEF_SPI_DISABLE_IRQ(handle->instance_num);
    UEF_SPI_CLK_DISABLE(handle->instance_num);

    handle->is_initialized = false;
    return 0;
}

// ============================================================
// CONFIGURATION
// ============================================================

void UEF_SPI_Enable(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_SPI_ENABLE(handle->instance);
}

void UEF_SPI_Disable(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_SPI_DISABLE(handle->instance);
}

void UEF_SPI_SetBaudRate(UEF_SPI_Handle_t* handle, UEF_SPI_BaudRate_t baudrate) {
    if (!handle || !handle->is_initialized) return;
    UEF_SPI_SET_BAUD_DIV(handle->instance, baudrate);
    handle->config.baudrate = baudrate;
}

// ============================================================
// TRANSMIT/RECEIVE FUNCTIONS
// ============================================================

int UEF_SPI_TransmitByte(UEF_SPI_Handle_t* handle, uint8_t byte) {
    if (!handle || !handle->is_initialized) return -1;

    void* spi = handle->instance;

    // Wait for TX buffer empty via ChipLink
    if (UEF_SPI_WaitForTXReady(spi, 1000) != 0) {
        return -2;
    }

    // Send byte via ChipLink
    UEF_SPI_WRITE_BYTE(spi, byte);

    // Wait for RX buffer (dummy read) via ChipLink
    if (UEF_SPI_WaitForRXReady(spi, 1000) != 0) {
        return -3;
    }
    UEF_SPI_READ_BYTE(spi);  // Discard

    // Wait for busy flag to clear via ChipLink
    if (UEF_SPI_WaitForNotBusy(spi, 1000) != 0) {
        return -4;
    }

    return 0;
}

uint8_t UEF_SPI_ReceiveByte(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return 0;

    void* spi = handle->instance;

    // Send dummy byte to generate clock via ChipLink
    UEF_SPI_WRITE_BYTE(spi, 0xFF);

    // Wait for RX buffer via ChipLink
    if (UEF_SPI_WaitForRXReady(spi, 1000) != 0) {
        return 0;
    }

    return UEF_SPI_READ_BYTE(spi);
}

uint8_t UEF_SPI_TransceiveByte(UEF_SPI_Handle_t* handle, uint8_t byte) {
    if (!handle || !handle->is_initialized) return 0;

    void* spi = handle->instance;

    // Wait for TX buffer empty via ChipLink
    if (UEF_SPI_WaitForTXReady(spi, 1000) != 0) {
        return 0;
    }

    // Send byte via ChipLink
    UEF_SPI_WRITE_BYTE(spi, byte);

    // Wait for RX buffer via ChipLink
    if (UEF_SPI_WaitForRXReady(spi, 1000) != 0) {
        return 0;
    }

    return UEF_SPI_READ_BYTE(spi);
}

int UEF_SPI_Transmit(UEF_SPI_Handle_t* handle, const uint8_t* data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!data || size == 0) return -2;

    for (uint16_t i = 0; i < size; i++) {
        if (UEF_SPI_TransmitByte(handle, data[i]) != 0) {
            return -3;
        }
    }

    return 0;
}

int UEF_SPI_Receive(UEF_SPI_Handle_t* handle, uint8_t* data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!data || size == 0) return -2;

    for (uint16_t i = 0; i < size; i++) {
        data[i] = UEF_SPI_ReceiveByte(handle);
    }

    return 0;
}

int UEF_SPI_TransmitReceive(UEF_SPI_Handle_t* handle, const uint8_t* tx_data, 
                            uint8_t* rx_data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!tx_data || !rx_data || size == 0) return -2;

    for (uint16_t i = 0; i < size; i++) {
        rx_data[i] = UEF_SPI_TransceiveByte(handle, tx_data[i]);
    }

    return 0;
}

// ============================================================
// 16-BIT OPERATIONS
// ============================================================

int UEF_SPI_TransmitWord(UEF_SPI_Handle_t* handle, uint16_t word) {
    if (!handle || !handle->is_initialized) return -1;
    if (handle->config.data_width != UEF_SPI_DATA_16BIT) return -2;

    void* spi = handle->instance;

    if (UEF_SPI_WaitForTXReady(spi, 1000) != 0) return -3;
    UEF_SPI_WRITE_WORD(spi, word);

    if (UEF_SPI_WaitForRXReady(spi, 1000) != 0) return -4;
    UEF_SPI_READ_WORD(spi);  // Discard

    if (UEF_SPI_WaitForNotBusy(spi, 1000) != 0) return -5;

    return 0;
}

uint16_t UEF_SPI_ReceiveWord(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return 0;
    if (handle->config.data_width != UEF_SPI_DATA_16BIT) return 0;

    void* spi = handle->instance;

    UEF_SPI_WRITE_WORD(spi, 0xFFFF);
    if (UEF_SPI_WaitForRXReady(spi, 1000) != 0) return 0;

    return UEF_SPI_READ_WORD(spi);
}

uint16_t UEF_SPI_TransceiveWord(UEF_SPI_Handle_t* handle, uint16_t word) {
    if (!handle || !handle->is_initialized) return 0;
    if (handle->config.data_width != UEF_SPI_DATA_16BIT) return 0;

    void* spi = handle->instance;

    if (UEF_SPI_WaitForTXReady(spi, 1000) != 0) return 0;
    UEF_SPI_WRITE_WORD(spi, word);

    if (UEF_SPI_WaitForRXReady(spi, 1000) != 0) return 0;

    return UEF_SPI_READ_WORD(spi);
}

// ============================================================
// STATUS FUNCTIONS
// ============================================================

bool UEF_SPI_IsBusy(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_SPI_IS_BUSY(handle->instance) || handle->busy;
}

bool UEF_SPI_IsTxReady(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_SPI_GET_FLAG(handle->instance, UEF_SPI_FLAG_TXE);
}

bool UEF_SPI_IsRxReady(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_SPI_GET_FLAG(handle->instance, UEF_SPI_FLAG_RXNE);
}

uint32_t UEF_SPI_GetErrorFlags(UEF_SPI_Handle_t* handle) {
    if (!handle) return 0;
    return handle->error_flags;
}

void UEF_SPI_ClearErrorFlags(UEF_SPI_Handle_t* handle) {
    if (!handle) return;
    handle->error_flags = 0;
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void UEF_SPI_RegisterCallbacks(UEF_SPI_Handle_t* handle,
                              void (*tx_cb)(UEF_SPI_Handle_t*),
                              void (*rx_cb)(UEF_SPI_Handle_t*),
                              void (*err_cb)(UEF_SPI_Handle_t*, uint32_t)) {
    if (!handle) return;
    handle->tx_complete_cb = tx_cb;
    handle->rx_complete_cb = rx_cb;
    handle->error_cb = err_cb;
}

// ============================================================
// INTERRUPT HANDLER - Updated
// ============================================================

void UEF_SPI_IRQHandler(UEF_SPI_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;

    void* spi = handle->instance;

    // Get status via ChipLink
    uint32_t status = UEF_SPI_GET_STATUS(spi);
    uint32_t cr2 = UEF_SPI_READ_CR2(spi);

    // Check for errors via ChipLink
    uint32_t errors = UEF_SPI_GET_ERRORS(status);
    if (errors) {
        handle->error_flags = errors;
        UEF_SPI_CLEAR_ERRORS(spi);
        if (handle->error_cb) {
            handle->error_cb(handle, handle->error_flags);
        }
        return;  // Exit after error handling
    }

    // RX interrupt via ChipLink
    if (UEF_SPI_RX_FLAG_SET(status) && UEF_SPI_RX_INTERRUPT_ENABLED(cr2)) {
        if (handle->config.data_width == UEF_SPI_DATA_16BIT) {
            uint16_t rx_data = UEF_SPI_READ_WORD(spi);
            (void)rx_data;  // Suppress unused warning
            // Handle 16-bit data if needed
        } else {
            uint8_t rx_data = UEF_SPI_READ_BYTE(spi);
            (void)rx_data;  // Suppress unused warning
            // Handle 8-bit data if needed
        }
        handle->rx_busy = false;
        if (handle->rx_complete_cb) {
            handle->rx_complete_cb(handle);
        }
    }

    // TX interrupt via ChipLink
    if (UEF_SPI_TX_FLAG_SET(status) && UEF_SPI_TX_INTERRUPT_ENABLED(cr2)) {
        handle->tx_busy = false;
        if (handle->tx_complete_cb) {
            handle->tx_complete_cb(handle);
        }
    }
}