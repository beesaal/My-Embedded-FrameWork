// ============================================================
// File: App/test_adc.c
// PURPOSE: Test ADC driver (read analog voltage from a pot)
// ============================================================
// NOTE: Connect a potentiometer to PA0 (ADC1 channel 0)
//       VCC -> 3.3V, GND -> GND, wiper -> PA0
// ============================================================

#include "UEF_clock.h"
#include "UEF_adc.h"
#include "UEF_gpio.h"
#include <stdio.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_ADC_Handle_t adc_handle;
static UEF_GPIO_Handle_t led_handle;

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // Step 1: Configure system clock
    UEF_Clock_ConfigHSI();
    
    // Step 2: Configure LED for status
    UEF_GPIO_Config_t led_config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&led_handle, &led_config);
    
    // Step 3: Initialize ADC
    UEF_ADC_Config_t adc_config = {
        .instance = 1,              // ADC1
        .channel = 0,               // Channel 0 (PA0)
        .gpio_port = GPIOA,
        .gpio_pin = 0,
        .resolution = UEF_ADC_RESOLUTION_12BIT,
        .align = UEF_ADC_ALIGN_RIGHT,
        .sample_time = UEF_ADC_SAMPLE_56_CYCLES,
        .enable_interrupts = false
    };
    
    if (UEF_ADC_Init(&adc_handle, &adc_config) != 0) {
        while (1); // Error - halt
    }
    
    // Step 4: Blink LED to show initialization complete
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(100);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(100);
    }
    
    // Step 5: Main loop - read ADC and control LED brightness
    uint16_t threshold = 2048;  // ~3.3V/2
    
    while (1) {
        // Read ADC value
        uint16_t adc_value = UEF_ADC_ReadChannel(&adc_handle);
        
        // Convert to voltage (assuming 3.3V reference)
        float voltage = (adc_value * 3.3f) / 4095.0f;
        
        // Control LED based on ADC value
        if (adc_value > threshold) {
            UEF_GPIO_Set(&led_handle);
        } else {
            UEF_GPIO_Reset(&led_handle);
        }
        
        // Simple LED pattern based on ADC value
        // Blink faster when voltage is higher
        uint32_t blink_delay = 1000 - (adc_value / 4);  // 0-1000ms
        
        // Only blink if we have a valid reading
        if (adc_value > 100) {
            UEF_GPIO_Toggle(&led_handle);
            delay_ms(blink_delay / 10);  // Scale down for visible blinking
        }
        
        delay_ms(50);
    }
}