// ============================================================
// File: UEF/UEF_Drivers/UEF_GPIO/UEF_gpio.h
// ============================================================
// PURPOSE: Universal GPIO driver interface
//          Works on ANY MCU through ChipLink.h
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#ifndef UEF_GPIO_H
#define UEF_GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "ChipLink.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// GPIO Configuration Types
// ============================================================

typedef enum {
    UEF_GPIO_MODE_INPUT = 0,
    UEF_GPIO_MODE_OUTPUT_PP = 1,
    UEF_GPIO_MODE_OUTPUT_OD = 2,
    UEF_GPIO_MODE_AF_PP = 3,
    UEF_GPIO_MODE_AF_OD = 4,
    UEF_GPIO_MODE_ANALOG = 5
} UEF_GPIO_Mode_t;

typedef enum {
    UEF_GPIO_PULL_NONE = 0,
    UEF_GPIO_PULL_UP = 1,
    UEF_GPIO_PULL_DOWN = 2
} UEF_GPIO_Pull_t;

typedef enum {
    UEF_GPIO_SPEED_LOW = 0,
    UEF_GPIO_SPEED_MEDIUM = 1,
    UEF_GPIO_SPEED_HIGH = 2,
    UEF_GPIO_SPEED_VERY_HIGH = 3
} UEF_GPIO_Speed_t;

typedef struct {
    void* port;                 // GPIO port (GPIOA, GPIOB, etc.)
    uint16_t pin;               // Pin number (0-15)
    UEF_GPIO_Mode_t mode;       // Input, output, AF, analog
    UEF_GPIO_Pull_t pull;       // Pull-up, pull-down, or none
    UEF_GPIO_Speed_t speed;     // Output speed
    uint8_t alternate_function; // AF number (0-15)
} UEF_GPIO_Config_t;

typedef struct {
    void* port;
    uint16_t pin;
    bool is_initialized;
    UEF_GPIO_Config_t config;
} UEF_GPIO_Handle_t;

// ============================================================
// GPIO API - UNIVERSAL FUNCTIONS
// ============================================================

// Initialization
int UEF_GPIO_Init(UEF_GPIO_Handle_t* handle, const UEF_GPIO_Config_t* config);
int UEF_GPIO_DeInit(UEF_GPIO_Handle_t* handle);

// Write operations
void UEF_GPIO_Write(UEF_GPIO_Handle_t* handle, bool state);
void UEF_GPIO_Set(UEF_GPIO_Handle_t* handle);
void UEF_GPIO_Reset(UEF_GPIO_Handle_t* handle);
void UEF_GPIO_Toggle(UEF_GPIO_Handle_t* handle);

// Read operations
bool UEF_GPIO_Read(UEF_GPIO_Handle_t* handle);

// Configuration
void UEF_GPIO_SetMode(UEF_GPIO_Handle_t* handle, UEF_GPIO_Mode_t mode);
void UEF_GPIO_SetPull(UEF_GPIO_Handle_t* handle, UEF_GPIO_Pull_t pull);

// Port operations
void UEF_GPIO_WritePort(void* port, uint32_t value);
uint32_t UEF_GPIO_ReadPort(void* port);

#ifdef __cplusplus
}
#endif

#endif // UEF_GPIO_H