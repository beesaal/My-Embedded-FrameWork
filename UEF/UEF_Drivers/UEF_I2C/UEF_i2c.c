// ============================================================
// File: UEF/UEF_Drivers/UEF_I2C/UEF_i2c.c
// ============================================================
// PURPOSE: Universal I2C driver implementation
//          Uses ONLY ChipLink.h - 100% portable
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#include "UEF_i2c.h"
#include "ChipLink.h"
#include <string.h>

// ============================================================
// PRIVATE HELPERS - Call ONLY ChipLink functions
// ============================================================

static int UEF_I2C_WaitForFlag(void* i2c, uint32_t flag, uint32_t timeout_ms) {
    uint32_t start = UEF_GetTick();
    while (!UEF_I2C_GET_FLAG(i2c, flag)) {
        if ((UEF_GetTick() - start) > timeout_ms) {
            return -1;
        }
    }
    return 0;
}

static int UEF_I2C_WaitForStart(void* i2c, uint32_t timeout_ms) {
    return UEF_I2C_WaitForFlag(i2c, UEF_I2C_FLAG_SB, timeout_ms);
}

static int UEF_I2C_WaitForAddr(void* i2c, uint32_t timeout_ms) {
    return UEF_I2C_WaitForFlag(i2c, UEF_I2C_FLAG_ADDR, timeout_ms);
}

static int UEF_I2C_WaitForTXReady(void* i2c, uint32_t timeout_ms) {
    return UEF_I2C_WaitForFlag(i2c, UEF_I2C_FLAG_TXE, timeout_ms);
}

static int UEF_I2C_WaitForRXReady(void* i2c, uint32_t timeout_ms) {
    return UEF_I2C_WaitForFlag(i2c, UEF_I2C_FLAG_RXNE, timeout_ms);
}

static int UEF_I2C_WaitForBTF(void* i2c, uint32_t timeout_ms) {
    return UEF_I2C_WaitForFlag(i2c, UEF_I2C_FLAG_BTF, timeout_ms);
}

// ============================================================
// INITIALIZATION
// ============================================================

int UEF_I2C_Init(UEF_I2C_Handle_t* handle, const UEF_I2C_Config_t* config) {
    if (!handle || !config) return -1;
    if (config->instance == 0) return -2;

    // Get hardware instance via ChipLink
    void* i2c = UEF_I2C_GET(config->instance);
    if (!i2c) return -3;

    // Store config
    handle->instance = i2c;
    handle->instance_num = config->instance;
    handle->config = *config;
    handle->is_initialized = false;
    handle->busy = false;
    handle->tx_busy = false;
    handle->rx_busy = false;
    handle->error_flags = 0;
    handle->tx_complete_cb = NULL;
    handle->rx_complete_cb = NULL;
    handle->error_cb = NULL;

    // Enable clock via ChipLink
    UEF_I2C_CLK_ENABLE(config->instance);

    // Configure pins via ChipLink
    if (UEF_I2C_ConfigurePins(config->instance) != 0) {
        return -4;
    }

    // Disable I2C for configuration via ChipLink
    UEF_I2C_DISABLE(i2c);

    // Configure frequency via ChipLink
    uint32_t pclk = UEF_Clock_GetAPB1Freq();
    uint32_t freq_mhz = pclk / 1000000;
    UEF_I2C_SET_FREQ(i2c, freq_mhz);

    // Configure speed via ChipLink
    UEF_I2C_SET_SPEED(i2c, pclk, config->speed);

    // Configure rise time via ChipLink
    UEF_I2C_SET_RISE_TIME(i2c, freq_mhz + 1);

    // Set own address (if slave mode) via ChipLink
    if (!config->is_master) {
        UEF_I2C_SET_OWN_ADDRESS(i2c, config->own_address);
    }

    // Enable I2C via ChipLink
    UEF_I2C_ENABLE(i2c);

    handle->is_initialized = true;
    return 0;
}

int UEF_I2C_DeInit(UEF_I2C_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return -1;

    void* i2c = handle->instance;

    // Disable I2C via ChipLink
    UEF_I2C_DISABLE(i2c);

    // Disable clock via ChipLink
    UEF_I2C_CLK_DISABLE(handle->instance_num);

    handle->is_initialized = false;
    return 0;
}

// ============================================================
// MASTER OPERATIONS
// ============================================================

int UEF_I2C_MasterWrite(UEF_I2C_Handle_t* handle, uint8_t dev_addr, 
                        const uint8_t* data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!data || size == 0) return -2;
    if (handle->busy) return -3;

    void* i2c = handle->instance;
    handle->busy = true;

    // Clear errors via ChipLink
    UEF_I2C_CLEAR_ERRORS(i2c);

    // Generate START via ChipLink
    UEF_I2C_GENERATE_START(i2c);
    if (UEF_I2C_WaitForStart(i2c, 1000) != 0) {
        handle->busy = false;
        return -4;
    }

    // Send device address (write) via ChipLink
    UEF_I2C_SEND_BYTE(i2c, (dev_addr << 1) & ~1);
    if (UEF_I2C_WaitForAddr(i2c, 1000) != 0) {
        handle->busy = false;
        return -5;
    }

    // Clear ADDR flag via ChipLink
    UEF_I2C_CLEAR_ADDR(i2c);

    // Send data via ChipLink
    for (uint16_t i = 0; i < size; i++) {
        if (UEF_I2C_WaitForTXReady(i2c, 1000) != 0) {
            handle->busy = false;
            return -6;
        }
        UEF_I2C_SEND_BYTE(i2c, data[i]);
    }

    // Wait for byte transfer finished via ChipLink
    if (UEF_I2C_WaitForBTF(i2c, 1000) != 0) {
        handle->busy = false;
        return -7;
    }

    // Generate STOP via ChipLink
    UEF_I2C_GENERATE_STOP(i2c);

    handle->busy = false;
    if (handle->tx_complete_cb) {
        handle->tx_complete_cb(handle);
    }

    return 0;
}

int UEF_I2C_MasterRead(UEF_I2C_Handle_t* handle, uint8_t dev_addr, 
                       uint8_t* data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!data || size == 0) return -2;
    if (handle->busy) return -3;

    void* i2c = handle->instance;
    handle->busy = true;

    // Clear errors via ChipLink
    UEF_I2C_CLEAR_ERRORS(i2c);

    // Generate START via ChipLink
    UEF_I2C_GENERATE_START(i2c);
    if (UEF_I2C_WaitForStart(i2c, 1000) != 0) {
        handle->busy = false;
        return -4;
    }

    // Send device address (read) via ChipLink
    UEF_I2C_SEND_BYTE(i2c, (dev_addr << 1) | 1);

    // Handle single-byte read specially (hardware-specific handled in ChipLink)
    if (size == 1) {
        if (UEF_I2C_WaitForAddr(i2c, 1000) != 0) {
            handle->busy = false;
            return -5;
        }
        // ChipLink handles the ACK/STOP ordering for single-byte reads
        UEF_I2C_SETUP_SINGLE_BYTE_READ(i2c);
    } else {
        if (UEF_I2C_WaitForAddr(i2c, 1000) != 0) {
            handle->busy = false;
            return -5;
        }
        // ChipLink handles ACK enable and ADDR clear
        UEF_I2C_CLEAR_ADDR_WITH_ACK(i2c);
    }

    // Read data via ChipLink
    for (uint16_t i = 0; i < size; i++) {
        // For last byte, ChipLink handles the STOP generation
        if (i == size - 1) {
            UEF_I2C_SETUP_LAST_BYTE_READ(i2c);
        }

        if (UEF_I2C_WaitForRXReady(i2c, 1000) != 0) {
            handle->busy = false;
            return -6;
        }
        data[i] = UEF_I2C_RECV_BYTE(i2c);
    }

    handle->busy = false;
    if (handle->rx_complete_cb) {
        handle->rx_complete_cb(handle);
    }

    return 0;
}

int UEF_I2C_MasterWriteRead(UEF_I2C_Handle_t* handle, uint8_t dev_addr,
                            const uint8_t* tx_data, uint16_t tx_size,
                            uint8_t* rx_data, uint16_t rx_size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!tx_data || tx_size == 0) return -2;
    if (!rx_data || rx_size == 0) return -3;
    if (handle->busy) return -4;

    int result = UEF_I2C_MasterWrite(handle, dev_addr, tx_data, tx_size);
    if (result != 0) return result;

    return UEF_I2C_MasterRead(handle, dev_addr, rx_data, rx_size);
}

// ============================================================
// SLAVE OPERATIONS
// ============================================================

int UEF_I2C_SlaveWrite(UEF_I2C_Handle_t* handle, const uint8_t* data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!data || size == 0) return -2;
    if (handle->busy) return -3;
    if (handle->config.is_master) return -4;

    void* i2c = handle->instance;
    handle->busy = true;

    // Clear errors via ChipLink
    UEF_I2C_CLEAR_ERRORS(i2c);

    // Enable ACK via ChipLink
    UEF_I2C_ENABLE_ACK(i2c);

    // Wait for address match via ChipLink
    if (UEF_I2C_WaitForAddr(i2c, 5000) != 0) {
        handle->busy = false;
        return -4;
    }

    // Clear ADDR flag via ChipLink
    UEF_I2C_CLEAR_ADDR(i2c);

    // Send data via ChipLink
    for (uint16_t i = 0; i < size; i++) {
        if (UEF_I2C_WaitForTXReady(i2c, 1000) != 0) {
            handle->busy = false;
            return -5;
        }
        UEF_I2C_SEND_BYTE(i2c, data[i]);
    }

    // Wait for master to finish via ChipLink
    // The AF flag indicates NACK from master, or STOPF indicates stop
    uint32_t start = UEF_GetTick();
    while (!(UEF_I2C_GET_FLAG(i2c, I2C_SR1_AF) || UEF_I2C_GET_FLAG(i2c, I2C_SR1_STOPF))) {
        if ((UEF_GetTick() - start) > 1000) {
            handle->busy = false;
            return -6;
        }
    }
    if (UEF_I2C_GET_FLAG(i2c, I2C_SR1_AF)) {
        UEF_I2C_CLEAR_ERRORS(i2c);  // Clear AF flag
    }
    if (UEF_I2C_GET_FLAG(i2c, I2C_SR1_STOPF)) {
        (void)UEF_I2C_GET_FLAG(i2c, I2C_SR1_STOPF);  // Read to clear
    }

    handle->busy = false;
    if (handle->tx_complete_cb) {
        handle->tx_complete_cb(handle);
    }
    return 0;
}

int UEF_I2C_SlaveRead(UEF_I2C_Handle_t* handle, uint8_t* data, uint16_t size) {
    if (!handle || !handle->is_initialized) return -1;
    if (!data || size == 0) return -2;
    if (handle->busy) return -3;
    if (handle->config.is_master) return -4;

    void* i2c = handle->instance;
    handle->busy = true;

    // Clear errors via ChipLink
    UEF_I2C_CLEAR_ERRORS(i2c);

    // Enable ACK via ChipLink
    UEF_I2C_ENABLE_ACK(i2c);

    // Wait for address match via ChipLink
    if (UEF_I2C_WaitForAddr(i2c, 5000) != 0) {
        handle->busy = false;
        return -4;
    }

    // Clear ADDR flag via ChipLink
    UEF_I2C_CLEAR_ADDR(i2c);

    // Read data via ChipLink
    for (uint16_t i = 0; i < size; i++) {
        if (UEF_I2C_WaitForRXReady(i2c, 1000) != 0) {
            handle->busy = false;
            return -5;
        }
        data[i] = UEF_I2C_RECV_BYTE(i2c);
    }

    handle->busy = false;
    if (handle->rx_complete_cb) {
        handle->rx_complete_cb(handle);
    }
    return 0;
}

// ============================================================
// STATUS FUNCTIONS
// ============================================================

bool UEF_I2C_IsBusy(UEF_I2C_Handle_t* handle) {
    if (!handle) return false;
    return handle->busy;
}

bool UEF_I2C_IsTxReady(UEF_I2C_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_I2C_GET_FLAG(handle->instance, UEF_I2C_FLAG_TXE);
}

bool UEF_I2C_IsRxReady(UEF_I2C_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return false;
    return UEF_I2C_GET_FLAG(handle->instance, UEF_I2C_FLAG_RXNE);
}

uint32_t UEF_I2C_GetErrorFlags(UEF_I2C_Handle_t* handle) {
    if (!handle) return 0;
    return handle->error_flags;
}

void UEF_I2C_ClearErrorFlags(UEF_I2C_Handle_t* handle) {
    if (!handle) return;
    handle->error_flags = 0;
}

// ============================================================
// CALLBACK REGISTRATION
// ============================================================

void UEF_I2C_RegisterCallbacks(UEF_I2C_Handle_t* handle,
                              void (*tx_cb)(UEF_I2C_Handle_t*),
                              void (*rx_cb)(UEF_I2C_Handle_t*),
                              void (*err_cb)(UEF_I2C_Handle_t*, uint32_t)) {
    if (!handle) return;
    handle->tx_complete_cb = tx_cb;
    handle->rx_complete_cb = rx_cb;
    handle->error_cb = err_cb;
}

// ============================================================
// INTERRUPT HANDLER
// ============================================================

void UEF_I2C_IRQHandler(UEF_I2C_Handle_t* handle) {
    if (!handle || !handle->is_initialized) return;

    void* i2c = handle->instance;

    // Get status via ChipLink
    uint32_t status = UEF_I2C_GET_STATUS(i2c);

    // Check for errors via ChipLink
    uint32_t errors = UEF_I2C_GET_ERRORS(status);
    if (errors) {
        handle->error_flags = errors;
        UEF_I2C_CLEAR_ERRORS(i2c);
        if (handle->error_cb) {
            handle->error_cb(handle, handle->error_flags);
        }
    }

    // RX interrupt via ChipLink
    if (UEF_I2C_RX_FLAG_SET(status)) {
        (void)UEF_I2C_RECV_BYTE(i2c);  // Read to clear flag
        handle->rx_busy = false;
        if (handle->rx_complete_cb) {
            handle->rx_complete_cb(handle);
        }
    }

    // TX interrupt via ChipLink
    if (UEF_I2C_TX_FLAG_SET(status)) {
        handle->tx_busy = false;
        if (handle->tx_complete_cb) {
            handle->tx_complete_cb(handle);
        }
    }
}