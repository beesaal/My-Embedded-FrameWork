// ============================================================
// File: syscalls.c - Stub implementations for newlib syscalls
// ============================================================
// PURPOSE: Provide minimal syscall stubs for embedded environment

#include <sys/stat.h>
#include <sys/types.h>   // For caddr_t
#include <errno.h>
#include <stdint.h>

// ============================================================
// STUB IMPLEMENTATIONS
// ============================================================

// Use weak attributes for all stubs so they can be overridden
#pragma weak _sbrk
#pragma weak _write
#pragma weak _read
#pragma weak _close
#pragma weak _fstat
#pragma weak _isatty
#pragma weak _lseek
#pragma weak _exit
#pragma weak _kill
#pragma weak _getpid
#pragma weak _raise
#pragma weak _unlink
#pragma weak _time
#pragma weak _times

/**
 * @brief Increase program break (heap)
 * @param incr Amount to increase heap by
 * @return Previous heap end address, or (caddr_t)-1 on failure
 */
caddr_t _sbrk(int incr) {
    extern char _end;      /* Defined by linker */
    static char *heap_end = NULL;
    char *prev_heap_end;
    
    if (heap_end == NULL) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;
    
    // For simplicity, we don't implement heap growth
    // Return the current heap end (which is just the end of .bss)
    // This effectively disables malloc
    (void)incr;  // Suppress unused parameter warning
    
    return (caddr_t)prev_heap_end;
}

/**
 * @brief Write to a file descriptor
 * @param file File descriptor
 * @param ptr Data buffer
 * @param len Number of bytes to write
 * @return Number of bytes written, or -1 on error
 */
int _write(int file, char *ptr, int len) {
    // No default output - applications should override this
    // Return success so printf doesn't crash
    (void)file;
    (void)ptr;
    return len;
}

/**
 * @brief Read from a file descriptor
 * @param file File descriptor
 * @param ptr Buffer to store data
 * @param len Number of bytes to read
 * @return Number of bytes read, or -1 on error
 */
int _read(int file, char *ptr, int len) {
    // No default input - applications should override this
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

/**
 * @brief Close a file descriptor
 * @param file File descriptor to close
 * @return 0 on success, -1 on error
 */
int _close(int file) {
    (void)file;
    return -1;
}

/**
 * @brief Get file status
 * @param file File descriptor
 * @param st Pointer to stat structure
 * @return 0 on success, -1 on error
 */
int _fstat(int file, struct stat *st) {
    (void)file;
    st->st_mode = S_IFCHR;
    return 0;
}

/**
 * @brief Check if file descriptor is a terminal
 * @param file File descriptor
 * @return 1 if terminal, 0 otherwise
 */
int _isatty(int file) {
    (void)file;
    return 1;
}

/**
 * @brief Reposition read/write file offset
 * @param file File descriptor
 * @param ptr Offset
 * @param dir Seek direction (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return New offset, or -1 on error
 */
int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

/**
 * @brief Terminate program
 * @param status Exit status
 */
void _exit(int status) {
    (void)status;
    while (1) {
        // Infinite loop on exit
        __asm volatile("bkpt 1");  // Breakpoint for debugger
    }
}

/**
 * @brief Send signal to process
 * @param pid Process ID
 * @param sig Signal number
 */
void _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    return;
}

/**
 * @brief Get process ID
 * @return Process ID (always 1 for embedded)
 */
int _getpid(void) {
    return 1;
}

// ============================================================
// ADDITIONAL STUBS FOR MATH AND OTHER FUNCTIONS
// ============================================================

/**
 * @brief Raise a signal
 * @param sig Signal number
 * @return 0 on success, -1 on error
 */
__attribute__((weak)) int _raise(int sig) {
    (void)sig;
    return 0;
}

/**
 * @brief Remove a file
 * @param name File name
 * @return 0 on success, -1 on error
 */
__attribute__((weak)) int _unlink(const char *name) {
    (void)name;
    return -1;
}

/**
 * @brief Get time in seconds since 1970-01-01
 * @param tloc Pointer to store time
 * @return Time in seconds, or -1 on error
 */
__attribute__((weak)) uint32_t _time(uint32_t *tloc) {
    if (tloc) {
        *tloc = 0;  // Return 0 time
    }
    return 0;
}

/**
 * @brief Get system clock ticks
 * @param buf Pointer to store tick info
 * @return Number of ticks
 */
__attribute__((weak)) uint32_t _times(uint32_t *buf) {
    (void)buf;
    return 0;
}

// ============================================================
// ALTERNATIVE: Define caddr_t if sys/types.h is not available
// ============================================================
// If you don't have sys/types.h, uncomment this:
//
// typedef char* caddr_t;