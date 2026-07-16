// ============================================================
// File: UEF/UEF_Drivers/UEF_SPI/UEF_spi.h
// ============================================================
// PURPOSE: Universal SPI driver interface
//          Works on ANY MCU through ChipLink.h
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#ifndef UEF_SPI_H
#define UEF_SPI_H

#include <stdint.h>
#include <stdbool.h>
#include "ChipLink.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// SPI Configuration Types
// ============================================================

typedef enum {
    UEF_SPI_BAUD_DIV_2 = 0,
    UEF_SPI_BAUD_DIV_4 = 1,
    UEF_SPI_BAUD_DIV_8 = 2,
    UEF_SPI_BAUD_DIV_16 = 3,
    UEF_SPI_BAUD_DIV_32 = 4,
    UEF_SPI_BAUD_DIV_64 = 5,
    UEF_SPI_BAUD_DIV_128 = 6,
    UEF_SPI_BAUD_DIV_256 = 7
} UEF_SPI_BaudRate_t;

typedef enum {
    UEF_SPI_MODE_0 = 0,  // CPOL=0, CPHA=0
    UEF_SPI_MODE_1 = 1,  // CPOL=0, CPHA=1
    UEF_SPI_MODE_2 = 2,  // CPOL=1, CPHA=0
    UEF_SPI_MODE_3 = 3   // CPOL=1, CPHA=1
} UEF_SPI_Mode_t;

typedef enum {
    UEF_SPI_DATA_8BIT = 0,
    UEF_SPI_DATA_16BIT = 1
} UEF_SPI_DataWidth_t;

typedef struct {
    uint8_t instance;           // SPI number (1, 2, 3, etc.)
    bool is_master;             // Master or slave
    UEF_SPI_Mode_t mode;        // SPI mode (0-3)
    UEF_SPI_BaudRate_t baudrate;// Baud rate divisor
    UEF_SPI_DataWidth_t data_width; // 8-bit or 16-bit
    bool lsb_first;             // LSB or MSB first
    bool enable_interrupts;     // Enable interrupts
} UEF_SPI_Config_t;

// Forward declaration
typedef struct UEF_SPI_Handle UEF_SPI_Handle_t;

struct UEF_SPI_Handle {
    void* instance;             // Hardware instance
    uint8_t instance_num;
    bool is_initialized;
    UEF_SPI_Config_t config;
    
    // State
    volatile bool tx_busy;
    volatile bool rx_busy;
    volatile bool busy;
    volatile uint32_t error_flags;
    
    // Callbacks
    void (*tx_complete_cb)(UEF_SPI_Handle_t* handle);
    void (*rx_complete_cb)(UEF_SPI_Handle_t* handle);
    void (*error_cb)(UEF_SPI_Handle_t* handle, uint32_t error);
};

// ============================================================
// SPI API - UNIVERSAL FUNCTIONS
// ============================================================

// Initialization
int UEF_SPI_Init(UEF_SPI_Handle_t* handle, const UEF_SPI_Config_t* config);
int UEF_SPI_DeInit(UEF_SPI_Handle_t* handle);

// Configuration
void UEF_SPI_Enable(UEF_SPI_Handle_t* handle);
void UEF_SPI_Disable(UEF_SPI_Handle_t* handle);
void UEF_SPI_SetBaudRate(UEF_SPI_Handle_t* handle, UEF_SPI_BaudRate_t baudrate);

// Transmit/Receive
int UEF_SPI_Transmit(UEF_SPI_Handle_t* handle, const uint8_t* data, uint16_t size);
int UEF_SPI_Receive(UEF_SPI_Handle_t* handle, uint8_t* data, uint16_t size);
int UEF_SPI_TransmitReceive(UEF_SPI_Handle_t* handle, const uint8_t* tx_data, uint8_t* rx_data, uint16_t size);

// Single byte
int UEF_SPI_TransmitByte(UEF_SPI_Handle_t* handle, uint8_t byte);
uint8_t UEF_SPI_ReceiveByte(UEF_SPI_Handle_t* handle);
uint8_t UEF_SPI_TransceiveByte(UEF_SPI_Handle_t* handle, uint8_t byte);

// 16-bit operations
int UEF_SPI_TransmitWord(UEF_SPI_Handle_t* handle, uint16_t word);
uint16_t UEF_SPI_ReceiveWord(UEF_SPI_Handle_t* handle);
uint16_t UEF_SPI_TransceiveWord(UEF_SPI_Handle_t* handle, uint16_t word);

// Status
bool UEF_SPI_IsBusy(UEF_SPI_Handle_t* handle);
bool UEF_SPI_IsTxReady(UEF_SPI_Handle_t* handle);
bool UEF_SPI_IsRxReady(UEF_SPI_Handle_t* handle);
uint32_t UEF_SPI_GetErrorFlags(UEF_SPI_Handle_t* handle);
void UEF_SPI_ClearErrorFlags(UEF_SPI_Handle_t* handle);

// Callback registration
void UEF_SPI_RegisterCallbacks(UEF_SPI_Handle_t* handle,
                              void (*tx_cb)(UEF_SPI_Handle_t*),
                              void (*rx_cb)(UEF_SPI_Handle_t*),
                              void (*err_cb)(UEF_SPI_Handle_t*, uint32_t));

// Interrupt handler
void UEF_SPI_IRQHandler(UEF_SPI_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif // UEF_SPI_H