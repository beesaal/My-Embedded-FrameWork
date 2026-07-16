// ============================================================
// File: UEF/UEF_Services/Logger/UEF_logger.c
// ============================================================
// PURPOSE: Universal Logger Service Implementation

#include "UEF_logger.h"
#include "UEF_clock.h"  // For UEF_Clock_GetTick()
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// ============================================================
// PRIVATE HELPERS
// ============================================================

static const char* UEF_Logger_GetLevelString(UEF_LogLevel_t level) {
    switch(level) {
        case UEF_LOG_LEVEL_ERROR: return "ERROR";
        case UEF_LOG_LEVEL_WARN:  return "WARN";
        case UEF_LOG_LEVEL_INFO:  return "INFO";
        case UEF_LOG_LEVEL_DEBUG: return "DEBUG";
        case UEF_LOG_LEVEL_TRACE: return "TRACE";
        default: return "UNKNOWN";
    }
}

static const char* UEF_Logger_GetLevelColor(UEF_LogLevel_t level) {
    // ANSI color codes (if colors enabled)
    // Red, Yellow, Green, Blue, Cyan
    switch(level) {
        case UEF_LOG_LEVEL_ERROR: return "\033[0;31m";  // Red
        case UEF_LOG_LEVEL_WARN:  return "\033[0;33m";  // Yellow
        case UEF_LOG_LEVEL_INFO:  return "\033[0;32m";  // Green
        case UEF_LOG_LEVEL_DEBUG: return "\033[0;34m";  // Blue
        case UEF_LOG_LEVEL_TRACE: return "\033[0;36m";  // Cyan
        default: return "\033[0m";  // Reset
    }
}

// ============================================================
// INITIALIZATION
// ============================================================

void UEF_Logger_Init(UEF_Logger_t* logger) {
    if (!logger) return;
    
    logger->level = UEF_LOG_LEVEL_INFO;
    logger->enable_timestamp = true;
    logger->enable_colors = false;  // Disable by default (no terminal)
    logger->output_cb = NULL;
    memset(logger->buffer, 0, sizeof(logger->buffer));
}

void UEF_Logger_SetOutputCallback(UEF_Logger_t* logger, void (*cb)(const char*, uint16_t)) {
    if (logger) {
        logger->output_cb = cb;
    }
}

void UEF_Logger_SetLevel(UEF_Logger_t* logger, UEF_LogLevel_t level) {
    if (logger) {
        logger->level = level;
    }
}

void UEF_Logger_EnableTimestamp(UEF_Logger_t* logger, bool enable) {
    if (logger) {
        logger->enable_timestamp = enable;
    }
}

void UEF_Logger_EnableColors(UEF_Logger_t* logger, bool enable) {
    if (logger) {
        logger->enable_colors = enable;
    }
}

// ============================================================
// CORE LOGGING
// ============================================================

void UEF_Logger_Log(UEF_Logger_t* logger, UEF_LogLevel_t level, const char* format, ...) {
    if (!logger || !logger->output_cb) return;
    if (level > logger->level) return;
    
    va_list args;
    va_start(args, format);
    
    // Format message
    int len = 0;
    char* buffer = logger->buffer;
    
    // Add timestamp
    if (logger->enable_timestamp) {
        // Simple timestamp - could be improved with RTC
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[%lu] ", (unsigned long)UEF_Clock_GetTick());
    }
    
    // Add color (if enabled)
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "%s", UEF_Logger_GetLevelColor(level));
    }
    
    // Add level
    len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[%s] ", UEF_Logger_GetLevelString(level));
    
    // Add message
    len += vsnprintf(buffer + len, sizeof(logger->buffer) - len, format, args);
    
    // Add color reset (if enabled)
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "\033[0m");
    }
    
    // Add newline
    if (len < (int)sizeof(logger->buffer) - 1) {
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    
    // Output
    logger->output_cb(buffer, len);
    
    va_end(args);
}

void UEF_Logger_LogString(UEF_Logger_t* logger, UEF_LogLevel_t level, const char* str) {
    if (!logger || !logger->output_cb) return;
    if (level > logger->level) return;
    
    int len = snprintf(logger->buffer, sizeof(logger->buffer), "[%s] %s\n", 
                       UEF_Logger_GetLevelString(level), str);
    
    logger->output_cb(logger->buffer, len);
}

void UEF_Logger_LogHex(UEF_Logger_t* logger, UEF_LogLevel_t level, const uint8_t* data, uint16_t len) {
    if (!logger || !logger->output_cb) return;
    if (level > logger->level) return;
    if (!data || len == 0) return;
    
    // Format as hex dump
    const uint16_t bytes_per_line = 16;
    char* buffer = logger->buffer;
    int pos = 0;
    
    // Add level
    pos += snprintf(buffer + pos, sizeof(logger->buffer) - pos, "[%s] Hex Dump (%u bytes):\n",
                    UEF_Logger_GetLevelString(level), len);
    
    // Output hex
    for (uint16_t i = 0; i < len; i++) {
        if (i % bytes_per_line == 0) {
            // New line
            pos += snprintf(buffer + pos, sizeof(logger->buffer) - pos, "%04X: ", i);
        }
        
        pos += snprintf(buffer + pos, sizeof(logger->buffer) - pos, "%02X ", data[i]);
        
        if (i % bytes_per_line == bytes_per_line - 1 || i == len - 1) {
            // End of line
            pos += snprintf(buffer + pos, sizeof(logger->buffer) - pos, "\n");
        }
        
        if (pos >= (int)sizeof(logger->buffer) - 50) {
            // Buffer full, send and reset
            logger->output_cb(buffer, pos);
            pos = 0;
        }
    }
    
    // Send remaining
    if (pos > 0) {
        logger->output_cb(buffer, pos);
    }
}

void UEF_Logger_Flush(UEF_Logger_t* logger) {
    // For UART-based logging, this would ensure all data is sent
    // For now, just a placeholder
    (void)logger;
}

// ============================================================
// CONVENIENCE FUNCTIONS
// ============================================================

void UEF_Logger_Error(UEF_Logger_t* logger, const char* format, ...) {
    if (!logger || !logger->output_cb) return;
    if (UEF_LOG_LEVEL_ERROR > logger->level) return;
    
    va_list args;
    va_start(args, format);
    
    // Format message
    int len = 0;
    char* buffer = logger->buffer;
    
    if (logger->enable_timestamp) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[%lu] ", (unsigned long)UEF_Clock_GetTick());
    }
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "%s", UEF_Logger_GetLevelColor(UEF_LOG_LEVEL_ERROR));
    }
    
    len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[ERROR] ");
    len += vsnprintf(buffer + len, sizeof(logger->buffer) - len, format, args);
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "\033[0m");
    }
    
    if (len < (int)sizeof(logger->buffer) - 1) {
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    
    logger->output_cb(buffer, len);
    
    va_end(args);
}

void UEF_Logger_Warn(UEF_Logger_t* logger, const char* format, ...) {
    if (!logger || !logger->output_cb) return;
    if (UEF_LOG_LEVEL_WARN > logger->level) return;
    
    va_list args;
    va_start(args, format);
    
    // Similar to error but with WARN level
    int len = 0;
    char* buffer = logger->buffer;
    
    if (logger->enable_timestamp) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[%lu] ", (unsigned long)UEF_Clock_GetTick());
    }
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "%s", UEF_Logger_GetLevelColor(UEF_LOG_LEVEL_WARN));
    }
    
    len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[WARN] ");
    len += vsnprintf(buffer + len, sizeof(logger->buffer) - len, format, args);
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "\033[0m");
    }
    
    if (len < (int)sizeof(logger->buffer) - 1) {
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    
    logger->output_cb(buffer, len);
    
    va_end(args);
}

void UEF_Logger_Info(UEF_Logger_t* logger, const char* format, ...) {
    if (!logger || !logger->output_cb) return;
    if (UEF_LOG_LEVEL_INFO > logger->level) return;
    
    va_list args;
    va_start(args, format);
    
    int len = 0;
    char* buffer = logger->buffer;
    
    if (logger->enable_timestamp) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[%lu] ", (unsigned long)UEF_Clock_GetTick());
    }
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "%s", UEF_Logger_GetLevelColor(UEF_LOG_LEVEL_INFO));
    }
    
    len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[INFO] ");
    len += vsnprintf(buffer + len, sizeof(logger->buffer) - len, format, args);
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "\033[0m");
    }
    
    if (len < (int)sizeof(logger->buffer) - 1) {
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    
    logger->output_cb(buffer, len);
    
    va_end(args);
}

void UEF_Logger_Debug(UEF_Logger_t* logger, const char* format, ...) {
    if (!logger || !logger->output_cb) return;
    if (UEF_LOG_LEVEL_DEBUG > logger->level) return;
    
    va_list args;
    va_start(args, format);
    
    int len = 0;
    char* buffer = logger->buffer;
    
    if (logger->enable_timestamp) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[%lu] ", (unsigned long)UEF_Clock_GetTick());
    }
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "%s", UEF_Logger_GetLevelColor(UEF_LOG_LEVEL_DEBUG));
    }
    
    len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[DEBUG] ");
    len += vsnprintf(buffer + len, sizeof(logger->buffer) - len, format, args);
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "\033[0m");
    }
    
    if (len < (int)sizeof(logger->buffer) - 1) {
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    
    logger->output_cb(buffer, len);
    
    va_end(args);
}

void UEF_Logger_Trace(UEF_Logger_t* logger, const char* format, ...) {
    if (!logger || !logger->output_cb) return;
    if (UEF_LOG_LEVEL_TRACE > logger->level) return;
    
    va_list args;
    va_start(args, format);
    
    int len = 0;
    char* buffer = logger->buffer;
    
    if (logger->enable_timestamp) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[%lu] ", (unsigned long)UEF_Clock_GetTick());
    }
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "%s", UEF_Logger_GetLevelColor(UEF_LOG_LEVEL_TRACE));
    }
    
    len += snprintf(buffer + len, sizeof(logger->buffer) - len, "[TRACE] ");
    len += vsnprintf(buffer + len, sizeof(logger->buffer) - len, format, args);
    
    if (logger->enable_colors) {
        len += snprintf(buffer + len, sizeof(logger->buffer) - len, "\033[0m");
    }
    
    if (len < (int)sizeof(logger->buffer) - 1) {
        buffer[len++] = '\n';
        buffer[len] = '\0';
    }
    
    logger->output_cb(buffer, len);
    
    va_end(args);
}