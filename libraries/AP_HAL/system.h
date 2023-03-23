#pragma once

#include <stdint.h>

#include <AP_Common/AP_Common.h>

#include "AP_HAL_Macros.h"

namespace AP_HAL {

void init();

void panic(const char *errormsg, ...) FMT_PRINTF(1, 2) NORETURN;

union micros_t {
    uint32_t micros;

    operator uint32_t() { return micros; }
    // allow construction from a uint32_t:
    micros_t(uint32_t x) { micros = x; }

    uint32_t operator +(uint32_t other) = delete;
    uint32_t operator +=(uint32_t other) = delete;
};

uint16_t micros16();
micros_t micros();
uint32_t millis();
uint16_t millis16();
uint64_t micros64();
uint64_t millis64();

uint32_t native_micros();
uint32_t native_millis();
uint16_t native_millis16();
uint64_t native_micros64();
uint64_t native_millis64();

void dump_stack_trace();
void dump_core_file();

} // namespace AP_HAL
