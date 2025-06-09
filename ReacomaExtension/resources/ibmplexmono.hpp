#pragma once

#include <stdint.h>

#ifndef RESOURCE_T_DEFINED
#define RESOURCE_T_DEFINED
struct resource_t {
    resource_t(const char *name, const uint8_t *data, uint32_t size)
        : name(name), data(data), size(size) {}
    const char *name;
    const uint8_t *data;
    const uint32_t size;
};
#endif

extern const uint8_t IBMPLEXMONO[133720];
extern const int IBMPLEXMONO_length;
