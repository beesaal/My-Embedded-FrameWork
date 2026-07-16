// ============================================================
// File: App/Examples/test_spi.c (DEBUG VERSION)
// PURPOSE: Comprehensive SPI driver test with debug LED blinks
// ============================================================

#include "UEF_clock.h"
#include "UEF_spi.h"
#include "UEF_gpio.h"
#include "UEF_uart.h"
#include "UEF_logger.h"
#include <stdio.h>
#include <string.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_SPI_Handle_t spi_handle;
static UEF_GPIO_Handle_t led_handle;
static UEF_UART_Handle_t uart_handle;
static UEF_Logger_t logger;

// ============================================================
// DEBUG LED HELPER
// ============================================================

static void debug_blink(uint32_t count, uint32_t delay_ms) {
    for (uint32_t i = 0; i < count; i++) {
        UEF_GPIO_Toggle(&led_handle);
        UEF_DelayMs(delay_ms);
    }
    UEF_GPIO_Set(&led_handle);
}

static void debug_indicate_step(uint32_t step_num) {
    // Blink step number
    for (uint32_t i = 0; i < step_num; i++) {
        UEF_GPIO_Reset(&led_handle);
        UEF_DelayMs(100);
        UEF_GPIO_Set(&led_handle);
        UEF_DelayMs(100);
    }
    UEF_DelayMs(500);
}

// ============================================================
// LOGGER OUTPUT
// ============================================================

static void logger_output_callback(const char* str, uint16_t len) {
    (void)len;
    if (uart_handle.is_initialized) {
        UEF_UART_SendString(&uart_handle, str);
    }
}

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// TEST: 8-BIT LOOPBACK (SIMPLIFIED)
// ============================================================

static bool test_8bit_loopback(void) {
    UEF_LOGI(&logger, "=== Testing 8-bit Loopback ===");
    
    // Simple test: just send a single byte
    uint8_t tx = 0x55;
    UEF_LOGI(&logger, "Sending 0x%02X...", tx);
    
    debug_blink(1, 200);  // Blink once before send
    
    uint8_t rx = UEF_SPI_TransceiveByte(&spi_handle, tx);
    
    debug_blink(2, 200);  // Blink twice after receive
    
    UEF_LOGI(&logger, "Received 0x%02X", rx);
    
    if (rx == tx) {
        UEF_LOGI(&logger, "✅ PASSED");
        return true;
    } else {
        UEF_LOGE(&logger, "❌ FAILED (expected 0x%02X, got 0x%02X)", tx, rx);
        return false;
    }
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    // Step 1: Configure system clock
    UEF_Clock_ConfigHSI();
    UEF_Clock_ConfigSysTick(UEF_Clock_GetSystemFreq(), 1000);
    debug_blink(1, 200);  // Step 1: Clock OK
    
    // Step 2: Initialize LED
    UEF_GPIO_Config_t led_config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&led_handle, &led_config);
    debug_blink(2, 200);  // Step 2: LED OK
    UEF_GPIO_Set(&led_handle);
    
    // Step 3: Initialize UART for logging
    UEF_UART_Config_t uart_config = {
        .instance = 2,
        .baudrate = 115200,
        .data_bits = UEF_UART_DATA_8,
        .stop_bits = UEF_UART_STOP_1,
        .parity = UEF_UART_PARITY_NONE,
        .enable_tx = true,
        .enable_rx = false,
        .enable_interrupts = false
    };
    UEF_UART_Init(&uart_handle, &uart_config);
    debug_blink(3, 200);  // Step 3: UART OK
    
    // Step 4: Initialize Logger
    UEF_Logger_Init(&logger);
    UEF_Logger_SetOutputCallback(&logger, logger_output_callback);
    UEF_Logger_SetLevel(&logger, UEF_LOG_LEVEL_INFO);
    debug_blink(4, 200);  // Step 4: Logger OK
    
    // Step 5: Print header
    UEF_UART_SendString(&uart_handle, "\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "UEF SPI DRIVER TEST v1.0 (DEBUG)\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "Mode: Master, CPOL=0, CPHA=0\r\n");
    UEF_UART_SendString(&uart_handle, "Baud: 1MHz (16MHz/16)\r\n");
    UEF_UART_SendString(&uart_handle, "NOTE: Connect MOSI (PA7) to MISO (PA6)!\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n\r\n");
    
    debug_blink(5, 200);  // Step 5: Header printed
    
    // Step 6: Initialize SPI with debug
    UEF_LOGI(&logger, "Initializing SPI...");
    debug_blink(6, 200);  // Before SPI init
    
    UEF_SPI_Config_t spi_config = {
        .instance = 1,
        .is_master = true,
        .mode = UEF_SPI_MODE_0,
        .baudrate = UEF_SPI_BAUD_DIV_16,
        .data_width = UEF_SPI_DATA_8BIT,
        .lsb_first = false,
        .enable_interrupts = false
    };
    
    UEF_LOGI(&logger, "Calling UEF_SPI_Init...");
    int spi_result = UEF_SPI_Init(&spi_handle, &spi_config);
    
    if (spi_result != 0) {
        UEF_LOGE(&logger, "SPI init failed! Error code: %d", spi_result);
        while (1) {
            UEF_GPIO_Set(&led_handle);
            delay_ms(100);
            UEF_GPIO_Reset(&led_handle);
            delay_ms(100);
        }
    }
    
    UEF_LOGI(&logger, "SPI initialized successfully!");
    debug_blink(7, 200);  // Step 7: SPI init OK
    
    // Step 8: Boot LED pattern
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(100);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(100);
    }
    UEF_GPIO_Set(&led_handle);
    delay_ms(500);
    
    // Step 9: Run tests
    UEF_LOGI(&logger, "Starting tests...");
    debug_blink(8, 200);  // Before tests
    
    bool all_passed = true;
    
    // Test 1: 8-bit loopback
    UEF_LOGI(&logger, "Running 8-bit loopback test...");
    debug_blink(9, 200);
    if (!test_8bit_loopback()) all_passed = false;
    delay_ms(500);
    
    // Step 10: Final message
    UEF_UART_SendString(&uart_handle, "\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    if (all_passed) {
        UEF_UART_SendString(&uart_handle, "✅ ALL SPI TESTS PASSED!\r\n");
    } else {
        UEF_UART_SendString(&uart_handle, "❌ SOME SPI TESTS FAILED!\r\n");
        UEF_UART_SendString(&uart_handle, "Check jumper: MOSI (PA7) to MISO (PA6)\r\n");
    }
    UEF_UART_SendString(&uart_handle, "========================================\r\n\r\n");
    
    // Step 11: Main loop - heartbeat
    uint32_t counter = 0;
    while (1) {
        counter++;
        if (counter >= 1000) {
            counter = 0;
            UEF_GPIO_Toggle(&led_handle);
        }
        delay_ms(1);
    }
}

int _write(int file, char *ptr, int len) {
    (void)file;
    if (uart_handle.is_initialized && len > 0) {
        UEF_UART_SendBytes(&uart_handle, (const uint8_t*)ptr, len);
    }
    return len;
}