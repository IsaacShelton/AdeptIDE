
#ifndef META_DIRECTIVES_H
#define META_DIRECTIVES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "UTIL/ground.h"

#define META_EXPR_UNDEF 0x00
#define META_EXPR_TRUE  0x01
#define META_EXPR_FALSE 0x02
#define META_EXPR_AND   0x03
#define META_EXPR_OR    0x04
#define META_EXPR_XOR   0x05
#define META_EXPR_STR   0x06
#define META_EXPR_VAR   0x07
#define META_EXPR_INT   0x08
#define META_EXPR_FLOAT 0x09
#define META_EXPR_ADD   0x0A
#define META_EXPR_SUB   0x0B
#define META_EXPR_MUL   0x0C
#define META_EXPR_DIV   0x0D
#define META_EXPR_MOD   0x0E
#define META_EXPR_POW   0x0F
#define META_EXPR_EQ    0x10
#define META_EXPR_NEQ   0x11
#define META_EXPR_GT    0x12
#define META_EXPR_GTE   0x13
#define META_EXPR_LT    0x15
#define META_EXPR_LTE   0x16
#define META_EXPR_NOT   0x17

#define IS_META_EXPR_ID_COLLAPSED(m) ( \
    m == META_EXPR_UNDEF || m == META_EXPR_TRUE || m == META_EXPR_FALSE || m == META_EXPR_STR || \
    m == META_EXPR_INT || m == META_EXPR_FLOAT \
)

typedef struct {
    unsigned int id;
} meta_expr_t;

typedef struct {
    unsigned int id;
    meta_expr_t *a, *b;
} meta_expr_math_t, meta_expr_logical_t, meta_expr_and_t, meta_expr_or_t, meta_expr_xor_t;

typedef struct {
    unsigned int id;
    strong_cstr_t value;
} meta_expr_str_t;

typedef struct {
    unsigned int id;
    strong_cstr_t name;
} meta_expr_var_t;

typedef struct {
    unsigned int id;
    long long value;
} meta_expr_int_t;

typedef struct {
    unsigned int id;
    double value;
} meta_expr_float_t;

typedef struct {
    weak_cstr_t name;
    meta_expr_t *value;
} meta_definition_t;

strong_cstr_t meta_expr_str(meta_expr_t *meta);
void meta_expr_free(meta_expr_t *expr);
void meta_expr_free_fully(meta_expr_t *expr);

meta_expr_t *meta_expr_clone(meta_expr_t *expr);

void meta_collapse(meta_definition_t *definitions, length_t definitions_length, meta_expr_t **expr);
bool meta_expr_into_bool(meta_definition_t *definitions, length_t definitions_length, meta_expr_t **expr);
long long meta_expr_into_int(meta_definition_t *definitions, length_t definitions_length, meta_expr_t **expr);
double meta_expr_into_float(meta_definition_t *definitions, length_t definitions_length, meta_expr_t **expr);
strong_cstr_t meta_expr_into_string(meta_definition_t *definitions, length_t definitions_length, meta_expr_t **expr);

void meta_definition_add(meta_definition_t **definitions, length_t *length, length_t *capacity, weak_cstr_t name, meta_expr_t *value);
void meta_definition_add_bool(meta_definition_t **definitions, length_t *length, length_t *capacity, weak_cstr_t name, bool boolean);

meta_definition_t *meta_definition_find(meta_definition_t *definitions, length_t length, weak_cstr_t name);

#ifdef __cplusplus
}
#endif

#endif // META_DIRECTIVES_H
