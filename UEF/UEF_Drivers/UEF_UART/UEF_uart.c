// ============================================================
// File: UEF/UEF_Drivers/UEF_UART/UEF_uart.c
// ============================================================
// PURPOSE: Universal UART driver implementation
//          Uses ONLY ChipLink.h - 100% portable
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#include "UEF_uart.h"
#include "ChipLink.h"
#include <string.h>

// ============================================================
// PRIVATE HELPERS - Call ONLY ChipLink functions
// ============================================================

static int UEF_UART_WaitForFlag(void* uart, uint32_t flag, uint32_t timeout_ms) {
    uint32_t start = UEF_GetTick();
    while (!UEF_UART_GET_FLAG(uart, flag)) {
        if ((UEF_GetTick() - start) > timeout_ms) {
            return -1;
        }
    }
    return 0;
}

static int UEF_UART_WaitForTXReady(void* uart, uint32_t timeout_ms) {
    return UEF_UART_WaitForFlag(uart, UEF_UART_FLAG_TXE, timeout_ms);
}

static int UEF_UART_WaitForRXReady(void* uart, uint32_t timeout_ms) {
    return UEF_UART_WaitForFlag(uart, UEF_UART_FLAG_RXNE, timeout_ms);
}

static int UEF_UART_WaitForTXComplete(void* uart, uint32_t timeout_ms) {
    return UEF_UART_WaitForFlag(uart, UEF_UART_FLAG_TC, timeout_ms);
}

// ============================================================
// INITIALIZATION
// ============================================================

int UEF_UART_Init(UEF_UART_Handle_t* handle, const UEF_UART_Config_t* config) {
    if (!handle || !config) return -1;
    if (config->instance == 0) return -2;

    // Get hardware instance via ChipLink
    void* uart = UEF_UART_GET(config->instance);
    if (!uart) return -3;

    // Store config
    handle->instance = uart;
    handle->instance_num = config->instance;
    handle->config = *config;
    handle->is_initialized = false;
    handle->tx_busy = false;
    handle->rx_busy = false;
    handle->error_flags = 0;
    handle->tx_complete_cb = NULL;
    handle->rx_complete_cb = NULL;
    handle->error_cb = NULL;
    handle->rx_byte_cb = NULL;

    // Enable clock via ChipLink
    UEF_UART_CLK_ENABLE(config->instance);

    // Configure pins via ChipLink
    if (UEF_UART_ConfigurePins(config->instance) != 0) {
        return -4;
    }

    // Disable UART for configuration via ChipLink
    UEF_UART_DISABLE(uart);

    // Configure baud rate via ChipLink
    uint32_t pclk = UEF_UART_GetClockFreq(config->instance);
    uint32_t brr = UEF_UART_CalcBRR(pclk, config->baudrate);
    UEF_UART_SET_BAUD(uart, brr);

    // Configure data bits via ChipLink
    UEF_UART_SET_DATA_BITS(uart, config->data_bits);

    // Configure parity via ChipLink
    UEF_UART_SET_PARITY(uart, config->parity);

    // Configure stop bits via ChipLink
    UEF_UART_SET_STOP_BITS(uart, config->stop_bits);

    // Configure TX/RX via ChipLink
    if (config->enable_tx) {
        UEF_UART_ENABLE_TX(uart);
    }
    if (config->enable_rx) {
        UEF_UART_ENABLE_RX(uart);
    }

    // Enable interrupts via ChipLink
    if (config->enable_interrupts) {
        UEF_UART_ENABLE_RX_INTERRUPT(uart);
        UEF_UART_ENABLE_ERROR_INTERRUPT(uart);
        UEF_UART_ENABLE_IRQ(config->instance);
    }

    // Enable UART via ChipLink
    UEF_UART_ENABLE(uart);

    handle->is_initialized = true;
    return 0;
}

int UEF_UART_DeInit(UEF_UART_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;

    void* uart = handle->instance;

    // Disable UART via ChipLink
    UEF_UART_DISABLE(uart);

    // Disable interrupts via ChipLink
    UEF_UART_DISABLE_IRQ(handle->instance_num);

    // Disable clock via ChipLink
    UEF_UART_CLK_DISABLE(handle->instance_num);

    handle->is_initialized = false;
    return 0;
}

// ============================================================
// CONFIGURATION
// ============================================================

int UEF_UART_SetBaudRate(UEF_UART_Handle_t* handle, uint32_t baudrate) {
    if (!handle || !handle->is_initialized) return -1;
    if (baudrate == 0) return -2;

    void* uart = handle->instance;

    // Disable UART via ChipLink
    UEF_UART_DISABLE(uart);

    // Calculate and set baud rate via ChipLink
    uint32_t pclk = UEF_UART_GetClockFreq(handle->instance_num);
    uint32_t brr = UEF_UART_CalcBRR(pclk, baudrate);
    UEF_UART_SET_BAUD(uart, brr);

    // Re-enable UART via ChipLink
    UEF_UART_ENABLE(uart);

    handle->config.baudrate = baudrate;
    return 0;
}

void UEF_UART_Enable(UEF_UART_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_UART_ENABLE(handle->instance);
}

void UEF_UART_Disable(UEF_UART_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;
    UEF_UART_DISABLE(handle->instance);
}

// ============================================================
// TRANSMIT FUNCTIONS
// ============================================================

int UEF_UART_SendByte(UEF_UART_Handle_t* handle, uint8_t byte) {
    if (!handle || !handle->is_initialized) return -1;

    void* uart = handle->instance;

    // Wait for TX buffer empty via ChipLink
    if (UEF_UART_WaitForTXReady(uart, 1000) != 0) {
        return -2;
    }

    // Send byte via ChipLink
    UEF_UART_SEND_BYTE(uart, byte);

    // Wait for transmission complete via ChipLink
    if (UEF_UART_WaitForTXComplete(uart, 1000) != 0) {
        return -3;
    }

    return 0;
}

int UEF_UART_SendBytes(UEF_UART_Handle_t* handle, const uint8_t* data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!data || size == 0) return -2;

    for (uint16_t i = 0; i < size; i++) {
        if (UEF_UART_SendByte(handle, data[i]) != 0) {
            return -3;
        }
    }

    return 0;
}

int UEF_UART_SendString(UEF_UART_Handle_t* handle, const char* str) {
    if (!handle || !handle->is_initialized) return -1;
    if (!str) return -2;

    return UEF_UART_SendBytes(handle, (const uint8_t*)str, strlen(str));
}

int UEF_UART_SendByte_IT(UEF_UART_Handle_t* handle, uint8_t byte) {
    if (!handle || !handle->is_initialized) return -1;
    if (handle->tx_busy) return -2;

    void* uart = handle->instance;

    // Enable TX interrupt via ChipLink
    UEF_UART_ENABLE_TX_INTERRUPT(uart);

    // Send byte via ChipLink
    UEF_UART_SEND_BYTE(uart, byte);
    handle->tx_busy = true;

    return 0;
}

int UEF_UART_SendBytes_IT(UEF_UART_Handle_t* handle, const uint8_t* data, uint16_t size) {
    // This would require a buffer and state machine
    // For simplicity, we'll use blocking for now
    return UEF_UART_SendBytes(handle, data, size);
}

// ============================================================
// RECEIVE FUNCTIONS
// ============================================================

int UEF_UART_ReceiveByte(UEF_UART_Handle_t* handle, uint8_t* byte) {
    if (!handle || !handle->is_initialized) return -1;
    if (!byte) return -2;

    void* uart = handle->instance;

    // Wait for RX buffer ready via ChipLink
    if (UEF_UART_WaitForRXReady(uart, 1000) != 0) {
        return -3;
    }

    // Read byte via ChipLink
    *byte = UEF_UART_RECV_BYTE(uart);

    return 0;
}

int UEF_UART_ReceiveBytes(UEF_UART_Handle_t* handle, uint8_t* buffer, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!buffer || size == 0) return -2;

    for (uint16_t i = 0; i < size; i++) {
        if (UEF_UART_ReceiveByte(handle, &buffer[i]) != 0) {
            return -3;
        }
    }

    return 0;
}

int UEF_UART_ReceiveByte_IT(UEF_UART_Handle_t* handle, uint8_t* byte) {
    if (!handle || !handle->is_initialized) return -1;
    if (!byte) return -2;

    void* uart = handle->instance;

    // Enable RX interrupt via ChipLink
    UEF_UART_ENABLE_RX_INTERRUPT(uart);
    handle->rx_busy = true;

    return 0;
}

// ============================================================
// STATUS FUNCTIONS
// ============================================================

bool UEF_UART_IsTxReady(UEF_UART_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_UART_GET_FLAG(handle->instance, UEF_UART_FLAG_TXE);
}

bool UEF_UART_IsRxReady(UEF_UART_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_UART_GET_FLAG(handle->instance, UEF_UART_FLAG_RXNE);
}

bool UEF_UART_IsTxComplete(UEF_UART_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_UART_GET_FLAG(handle->instance, UEF_UART_FLAG_TC);
}

uint32_t UEF_UART_GetErrorFlags(UEF_UART_Handle_t* handle) {
    if (!handle) return 0;
    return handle->error_flags;
}

void UEF_UART_ClearErrorFlags(UEF_UART_Handle_t* handle) {
    if (!handle) return;
    handle->error_flags = 0;
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void UEF_UART_RegisterCallbacks(UEF_UART_Handle_t* handle,
                               void (*tx_cb)(UEF_UART_Handle_t*),
                               void (*rx_cb)(UEF_UART_Handle_t*),
                               void (*err_cb)(UEF_UART_Handle_t*, uint32_t)) {
    if (!handle) return;
    handle->tx_complete_cb = tx_cb;
    handle->rx_complete_cb = rx_cb;
    handle->error_cb = err_cb;
}

// ============================================================
// INTERRUPT HANDLER
// ============================================================

void UEF_UART_IRQHandler(UEF_UART_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;

    void* uart = handle->instance;

    // Get status via ChipLink
    uint32_t status = UEF_UART_GET_STATUS(uart);

    // Check for errors via ChipLink
    uint32_t errors = UEF_UART_GET_ERRORS(status);
    if (errors) {
        handle->error_flags = errors;
        UEF_UART_CLEAR_ERRORS(uart);
        if (handle->error_cb) {
            handle->error_cb(handle, handle->error_flags);
        }
    }

    // RX interrupt via ChipLink
    if (UEF_UART_RX_FLAG_SET(status) && UEF_UART_RX_INTERRUPT_ENABLED(uart)) {
        uint8_t data = UEF_UART_RECV_BYTE(uart);
        if (handle->rx_byte_cb) {
            handle->rx_byte_cb(handle, data);
        }
        if (handle->rx_complete_cb) {
            handle->rx_complete_cb(handle);
        }
        handle->rx_busy = false;
    }

    // TX interrupt via ChipLink
    if (UEF_UART_TX_FLAG_SET(status) && UEF_UART_TX_INTERRUPT_ENABLED(uart)) {
        // Disable TX interrupt via ChipLink
        UEF_UART_DISABLE_TX_INTERRUPT(uart);
        handle->tx_busy = false;
        if (handle->tx_complete_cb) {
            handle->tx_complete_cb(handle);
        }
    }
}