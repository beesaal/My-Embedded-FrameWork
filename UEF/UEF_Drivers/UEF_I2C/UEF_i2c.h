// ============================================================
// File: UEF/UEF_Drivers/UEF_I2C/UEF_i2c.h
// ============================================================
// PURPOSE: Universal I2C driver interface
//          Works on ANY MCU through ChipLink.h
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#ifndef UEF_I2C_H
#define UEF_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "ChipLink.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// I2C Configuration Types
// ============================================================

typedef enum {
    UEF_I2C_SPEED_STANDARD = 100000,   // 100 kHz
    UEF_I2C_SPEED_FAST = 400000,       // 400 kHz
    UEF_I2C_SPEED_FAST_PLUS = 1000000  // 1 MHz
} UEF_I2C_Speed_t;

typedef struct {
    uint8_t instance;           // I2C number (1, 2, 3, etc.)
    UEF_I2C_Speed_t speed;      // Bus speed
    uint8_t own_address;        // Own address (for slave mode)
    bool is_master;             // Master or slave mode
    bool enable_interrupts;     // Enable interrupts
} UEF_I2C_Config_t;

// Forward declaration
typedef struct UEF_I2C_Handle UEF_I2C_Handle_t;

struct UEF_I2C_Handle {
    void* instance;             // Hardware instance
    uint8_t instance_num;
    bool is_initialized;
    UEF_I2C_Config_t config;
    
    // State
    volatile bool busy;
    volatile bool tx_busy;
    volatile bool rx_busy;
    volatile uint32_t error_flags;
    
    // Callbacks
    void (*tx_complete_cb)(UEF_I2C_Handle_t* handle);
    void (*rx_complete_cb)(UEF_I2C_Handle_t* handle);
    void (*error_cb)(UEF_I2C_Handle_t* handle, uint32_t error);
};

// ============================================================
// I2C API - UNIVERSAL FUNCTIONS
// ============================================================

// Initialization
int UEF_I2C_Init(UEF_I2C_Handle_t* handle, const UEF_I2C_Config_t* config);
int UEF_I2C_DeInit(UEF_I2C_Handle_t* handle);

// Master operations
int UEF_I2C_MasterWrite(UEF_I2C_Handle_t* handle, uint8_t dev_addr, const uint8_t* data, uint16_t size);
int UEF_I2C_MasterRead(UEF_I2C_Handle_t* handle, uint8_t dev_addr, uint8_t* data, uint16_t size);
int UEF_I2C_MasterWriteRead(UEF_I2C_Handle_t* handle, uint8_t dev_addr, 
                            const uint8_t* tx_data, uint16_t tx_size,
                            uint8_t* rx_data, uint16_t rx_size);

// Slave operations
int UEF_I2C_SlaveWrite(UEF_I2C_Handle_t* handle, const uint8_t* data, uint16_t size);
int UEF_I2C_SlaveRead(UEF_I2C_Handle_t* handle, uint8_t* data, uint16_t size);

// Status
bool UEF_I2C_IsBusy(UEF_I2C_Handle_t* handle);
bool UEF_I2C_IsTxReady(UEF_I2C_Handle_t* handle);
bool UEF_I2C_IsRxReady(UEF_I2C_Handle_t* handle);
uint32_t UEF_I2C_GetErrorFlags(UEF_I2C_Handle_t* handle);
void UEF_I2C_ClearErrorFlags(UEF_I2C_Handle_t* handle);

// Callback registration
void UEF_I2C_RegisterCallbacks(UEF_I2C_Handle_t* handle,
                              void (*tx_cb)(UEF_I2C_Handle_t*),
                              void (*rx_cb)(UEF_I2C_Handle_t*),
                              void (*err_cb)(UEF_I2C_Handle_t*, uint32_t));

// Interrupt handler
void UEF_I2C_IRQHandler(UEF_I2C_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif // UEF_I2C_H