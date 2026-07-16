// ============================================================
// File: App/test_i2c.c
// PURPOSE: Test I2C driver (scan for devices)
// ============================================================
// NOTE: For this test, connect an I2C device (like an OLED,
//       EEPROM, or sensor) to the I2C pins (PB6=SCL, PB7=SDA).
// ============================================================

#include "UEF_clock.h"
#include "UEF_i2c.h"
#include "UEF_gpio.h"
#include <string.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_I2C_Handle_t i2c_handle;
static UEF_GPIO_Handle_t led_handle;

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// I2C SCAN FUNCTION
// ============================================================

static void i2c_scan(void) {
    uint8_t found_devices[128] = {0};
    uint8_t count = 0;
    
    // Test addresses 0x01 to 0x7F
    for (uint8_t addr = 1; addr < 128; addr++) {
        // Try to write 1 byte to the address
        uint8_t test_byte = 0x00;
        
        // Use a timeout to avoid hanging
        // For this test, we'll just try to write
        int result = UEF_I2C_MasterWrite(&i2c_handle, addr, &test_byte, 1);
        
        if (result == 0) {
            found_devices[count++] = addr;
        }
    }
    
    // LED pattern to indicate number of devices found
    if (count > 0) {
        // Blink LED 'count' times
        for (int i = 0; i < count && i < 10; i++) {
            UEF_GPIO_Set(&led_handle);
            delay_ms(200);
            UEF_GPIO_Reset(&led_handle);
            delay_ms(200);
        }
        delay_ms(1000);  // Pause before repeating
    } else {
        // No devices found - fast blink
        for (int i = 0; i < 5; i++) {
            UEF_GPIO_Set(&led_handle);
            delay_ms(50);
            UEF_GPIO_Reset(&led_handle);
            delay_ms(50);
        }
        delay_ms(1000);
    }
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // Step 1: Configure system clock
    UEF_Clock_ConfigHSI();
    
    // Step 2: Configure LED for status indication
    UEF_GPIO_Config_t led_config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&led_handle, &led_config);
    
    // Step 3: Initialize I2C
    UEF_I2C_Config_t i2c_config = {
        .instance = 1,           // I2C1 (PB6=SCL, PB7=SDA)
        .speed = UEF_I2C_SPEED_STANDARD,  // 100 kHz
        .own_address = 0x00,
        .is_master = true,
        .enable_interrupts = false
    };
    
    if (UEF_I2C_Init(&i2c_handle, &i2c_config) != 0) {
        // Error - fast blink forever
        while (1) {
            UEF_GPIO_Set(&led_handle);
            delay_ms(100);
            UEF_GPIO_Reset(&led_handle);
            delay_ms(100);
        }
    }
    
    // Step 4: Initial LED blink - alive
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(200);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(200);
    }
    delay_ms(500);
    
    // Step 5: Main loop - I2C scanner
    while (1) {
        // Wait a moment
        delay_ms(100);
        
        // Run I2C scan
        i2c_scan();
    }
}