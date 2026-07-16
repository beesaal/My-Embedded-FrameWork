// ============================================================
// File: App/main.c - UART TEST WITH UEF DRIVERS
// ============================================================

#include "UEF_clock.h"
#include "UEF_uart.h"
#include "UEF_gpio.h"
#include <string.h>

// ============================================================
// HANDLES
// ============================================================

static UEF_UART_Handle_t uart_handle;
static UEF_GPIO_Handle_t led_handle;

// ============================================================
// DELAY
// ============================================================

static void delay_ms(uint32_t ms) {
    UEF_DelayMs(ms);
}

// ============================================================
// MAIN - ULTRA SIMPLE UART TEST
// ============================================================

int main(void) {
    // STEP 1: Configure system clock
    UEF_Clock_ConfigHSI();
    
    // STEP 2: Initialize LED (PA5 on Nucleo)
    UEF_GPIO_Config_t led_config = {
        .port = GPIOA,
        .pin = 5,
        .mode = UEF_GPIO_MODE_OUTPUT_PP,
        .pull = UEF_GPIO_PULL_NONE,
        .speed = UEF_GPIO_SPEED_LOW,
        .alternate_function = 0
    };
    UEF_GPIO_Init(&led_handle, &led_config);
    
    // STEP 3: Blink LED to show we're alive
    for (int i = 0; i < 3; i++) {
        UEF_GPIO_Set(&led_handle);
        delay_ms(200);
        UEF_GPIO_Reset(&led_handle);
        delay_ms(200);
    }
    
    // STEP 4: Initialize UART
    UEF_UART_Config_t config = {
        .instance = 2,           // USART2 on Nucleo
        .baudrate = 115200,
        .data_bits = UEF_UART_DATA_8,
        .stop_bits = UEF_UART_STOP_1,
        .parity = UEF_UART_PARITY_NONE,
        .enable_tx = true,
        .enable_rx = true,
        .enable_interrupts = false
    };
    
    if (UEF_UART_Init(&uart_handle, &config) != 0) {
        // UART failed - fast blink forever
        while (1) {
            UEF_GPIO_Set(&led_handle);
            delay_ms(50);
            UEF_GPIO_Reset(&led_handle);
            delay_ms(50);
        }
    }
    
    // STEP 5: Send startup message
    delay_ms(500);  // Wait for serial terminal to connect
    
    UEF_UART_SendString(&uart_handle, "\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "UEF UART TEST v1.0\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "Baud: 115200, 8N1\r\n");
    UEF_UART_SendString(&uart_handle, "LED: PA5 (blinking = alive)\r\n");
    UEF_UART_SendString(&uart_handle, "========================================\r\n");
    UEF_UART_SendString(&uart_handle, "\r\n");
    UEF_UART_SendString(&uart_handle, "Type something and it will echo back!\r\n");
    UEF_UART_SendString(&uart_handle, "\r\n");
    
    // STEP 6: Main loop - echo and LED blink
    uint32_t counter = 0;
    uint8_t rx_byte;
    
    while (1) {
        // Blink LED every second
        counter++;
        if (counter >= 1000) {
            counter = 0;
            UEF_GPIO_Toggle(&led_handle);
            UEF_UART_SendString(&uart_handle, "Alive!\r\n");
        }
        
        // Check for incoming data (non-blocking with timeout)
        if (UEF_UART_ReceiveByte(&uart_handle, &rx_byte) == 0) {
            // Echo back the character
            UEF_UART_SendByte(&uart_handle, rx_byte);
            
            // Special commands
            if (rx_byte == '1') {
                UEF_GPIO_Set(&led_handle);
                UEF_UART_SendString(&uart_handle, "LED ON\r\n");
            } else if (rx_byte == '0') {
                UEF_GPIO_Reset(&led_handle);
                UEF_UART_SendString(&uart_handle, "LED OFF\r\n");
            } else if (rx_byte == '?' || rx_byte == 'h') {
                UEF_UART_SendString(&uart_handle, "\r\nCommands:\r\n");
                UEF_UART_SendString(&uart_handle, "  '1' - LED ON\r\n");
                UEF_UART_SendString(&uart_handle, "  '0' - LED OFF\r\n");
                UEF_UART_SendString(&uart_handle, "  '?' - Show help\r\n");
                UEF_UART_SendString(&uart_handle, "\r\n");
            }
        }
        
        delay_ms(1);
    }
}