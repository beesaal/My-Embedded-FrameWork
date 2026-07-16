// ============================================================
// File: UEF/UEF_Services/Logger/UEF_logger.h
// ============================================================
// PURPOSE: Universal Logger Service

#ifndef UEF_LOGGER_H
#define UEF_LOGGER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// Logger Types
// ============================================================

typedef enum {
    UEF_LOG_LEVEL_NONE = 0,
    UEF_LOG_LEVEL_ERROR = 1,
    UEF_LOG_LEVEL_WARN = 2,
    UEF_LOG_LEVEL_INFO = 3,
    UEF_LOG_LEVEL_DEBUG = 4,
    UEF_LOG_LEVEL_TRACE = 5
} UEF_LogLevel_t;

typedef struct {
    UEF_LogLevel_t level;           // Current log level
    bool enable_timestamp;          // Include timestamps
    bool enable_colors;             // Enable colored output
    void (*output_cb)(const char* str, uint16_t len);  // Output callback
    
    // Internal
    char buffer[256];  // Buffer for formatting
} UEF_Logger_t;

// ============================================================
// Logger API
// ============================================================

// Initialization
void UEF_Logger_Init(UEF_Logger_t* logger);
void UEF_Logger_SetOutputCallback(UEF_Logger_t* logger, void (*cb)(const char*, uint16_t));
void UEF_Logger_SetLevel(UEF_Logger_t* logger, UEF_LogLevel_t level);
void UEF_Logger_EnableTimestamp(UEF_Logger_t* logger, bool enable);
void UEF_Logger_EnableColors(UEF_Logger_t* logger, bool enable);

// Logging functions
void UEF_Logger_Error(UEF_Logger_t* logger, const char* format, ...);
void UEF_Logger_Warn(UEF_Logger_t* logger, const char* format, ...);
void UEF_Logger_Info(UEF_Logger_t* logger, const char* format, ...);
void UEF_Logger_Debug(UEF_Logger_t* logger, const char* format, ...);
void UEF_Logger_Trace(UEF_Logger_t* logger, const char* format, ...);

// Low-level logging
void UEF_Logger_Log(UEF_Logger_t* logger, UEF_LogLevel_t level, const char* format, ...);
void UEF_Logger_LogString(UEF_Logger_t* logger, UEF_LogLevel_t level, const char* str);
void UEF_Logger_LogHex(UEF_Logger_t* logger, UEF_LogLevel_t level, const uint8_t* data, uint16_t len);

// Utilities
void UEF_Logger_Flush(UEF_Logger_t* logger);

// ============================================================
// MACROS for convenient logging
// ============================================================

#define UEF_LOGE(logger, ...) UEF_Logger_Error(logger, __VA_ARGS__)
#define UEF_LOGW(logger, ...) UEF_Logger_Warn(logger, __VA_ARGS__)
#define UEF_LOGI(logger, ...) UEF_Logger_Info(logger, __VA_ARGS__)
#define UEF_LOGD(logger, ...) UEF_Logger_Debug(logger, __VA_ARGS__)
#define UEF_LOGT(logger, ...) UEF_Logger_Trace(logger, __VA_ARGS__)

// Conditional logging based on level
#define UEF_LOG_IF(logger, level, condition, ...) \
    do { \
        if (condition) { \
            UEF_Logger_Log(logger, level, __VA_ARGS__); \
        } \
    } while(0)

// Log with custom level
#define UEF_LOG(logger, level, ...) UEF_Logger_Log(logger, level, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // UEF_LOGGER_H