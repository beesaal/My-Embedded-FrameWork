// ============================================================
// File: UEF/UEF_Drivers/UEF_CLOCK/UEF_clock.h
// ============================================================
// PURPOSE: Universal clock driver interface
//          Works on ANY MCU through ChipLink.h
//          NO HARDWARE-SPECIFIC CODE
// ============================================================

#ifndef UEF_CLOCK_H
#define UEF_CLOCK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// CLOCK CONFIGURATION TYPES
// ============================================================

typedef enum {
    UEF_CLOCK_SRC_HSI = 0,     // Internal RC oscillator
    UEF_CLOCK_SRC_HSE = 1,     // External crystal
    UEF_CLOCK_SRC_PLL = 2      // PLL (high frequency)
} UEF_ClockSource_t;

typedef struct {
    UEF_ClockSource_t source;
    uint32_t frequency;
    uint32_t hclk;
    uint32_t apb1;
    uint32_t apb2;
} UEF_ClockConfig_t;

// ============================================================
// CLOCK API - UNIVERSAL FUNCTIONS
// ============================================================

// System clock configuration
int  UEF_Clock_ConfigHSI(void);
int  UEF_Clock_ConfigHSE(void);
int  UEF_Clock_ConfigPLL(uint32_t hse_freq);
int  UEF_Clock_ConfigSysTick(uint32_t sysclk, uint32_t ticks_per_second);

// Get current clock frequencies
uint32_t UEF_Clock_GetSystemFreq(void);
uint32_t UEF_Clock_GetHCLKFreq(void);
uint32_t UEF_Clock_GetAPB1Freq(void);
uint32_t UEF_Clock_GetAPB2Freq(void);

// Get current clock configuration
void UEF_Clock_GetConfig(UEF_ClockConfig_t* config);

// Get system tick counter
uint32_t UEF_Clock_GetTick(void);

// Delay functions
void UEF_DelayMs(uint32_t ms);
void UEF_DelayUs(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif // UEF_CLOCK_H