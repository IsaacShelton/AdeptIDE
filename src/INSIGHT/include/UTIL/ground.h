
#ifndef _ISAAC_GROUND_H
#define _ISAAC_GROUND_H

#ifdef __cplusplus
extern "C" {
#endif

/*
    ================================= ground.h =================================
    Module containing basic #includes and type definitions
    ----------------------------------------------------------------------------
*/

#define ADEPT_VERSION_STRING "2.4"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif

#include "UTIL/memory.h"

#ifdef ADEPT_INSIGHT_BUILD
#include "UTIL/__insight_overloads.h"
#endif

// ---------------- errorcode_t ----------------
#define SUCCESS     0
#define FAILURE     1
#define ALT_FAILURE 2
typedef int errorcode_t;

// ---------------- successful_t ----------------
#define SUCCESSFUL   true
#define UNSUCCESSFUL false
typedef bool successful_t;

// ---------------- strong_cstr_t ----------------
typedef char *strong_cstr_t;            // Owned C-String
typedef char *maybe_null_strong_cstr_t; // Maybe NULL Variant

// ---------------- weak_cstr_t ----------------
typedef char *weak_cstr_t;            // Unowned C-String
typedef char *maybe_null_weak_cstr_t; // Maybe NULL Variant

// ---------------- length_t ----------------
typedef size_t length_t;

// ---------------- source_t ----------------
typedef struct {
    length_t index;
    length_t object_index;
    length_t stride;
} source_t;

#define NULL_SOURCE (source_t){0, 0, 0}

// ---------------- maybe_index_t ----------------
typedef long long maybe_index_t;

// ---------------- troolean ----------------
// 3-state value
typedef signed char troolean;

// ---------------- lenstr_t and friends ----------------
// C-String with cached length
typedef struct {
    char *cstr;
    length_t length;
} lenstr_t, strong_lenstr_t, weak_lenstr_t;

#define TROOLEAN_TRUE 1
#define TROOLEAN_FALSE 0
#define TROOLEAN_UNKNOWN 2

// ---------------- special characters ----------------
#ifdef __APPLE__
#define USE_UTF8_INSTEAD_OF_EXTENDED_ASCII_FOR_SPECIAL_CHARACTERS
#endif

#ifdef USE_UTF8_INSTEAD_OF_EXTENDED_ASCII_FOR_SPECIAL_CHARACTERS
#define BOX_DRAWING_UP_RIGHT "\xE2\x94\x94"
#else
#define BOX_DRAWING_UP_RIGHT "\xC0"
#endif // USE_UTF8_INSTEAD_OF_EXTENDED_ASCII_FOR_SPECIAL_CHARACTERS

#ifdef __cplusplus
}
#endif

#endif // _ISAAC_GROUND_H
