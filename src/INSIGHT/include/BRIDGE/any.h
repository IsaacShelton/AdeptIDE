
#ifndef ANY_H
#define ANY_H

#ifdef __cplusplus
extern "C" {
#endif

/*
    ================================== any.h ==================================
    Module for bridging the gap for Any types
    ---------------------------------------------------------------------------
*/

//#include "IR/ir.h"
#include "AST/ast.h"
//#include "IRGEN/ir_builder.h"

#define ANY_TYPE_KIND_VOID        0x00
#define ANY_TYPE_KIND_BOOL        0x01
#define ANY_TYPE_KIND_BYTE        0x02
#define ANY_TYPE_KIND_UBYTE       0x03
#define ANY_TYPE_KIND_SHORT       0x04
#define ANY_TYPE_KIND_USHORT      0x05
#define ANY_TYPE_KIND_INT         0x06
#define ANY_TYPE_KIND_UINT        0x07
#define ANY_TYPE_KIND_LONG        0x08
#define ANY_TYPE_KIND_ULONG       0x09
#define ANY_TYPE_KIND_FLOAT       0x0A
#define ANY_TYPE_KIND_DOUBLE      0x0B
#define ANY_TYPE_KIND_PTR         0x0C
#define ANY_TYPE_KIND_STRUCT      0x0D
#define ANY_TYPE_KIND_FUNC_PTR    0x0E
#define ANY_TYPE_KIND_FIXED_ARRAY 0x0F

// ---------------- any_inject_ast ----------------
// Injects required data structures and enums
// for interfacing with 'Any' types
void any_inject_ast(ast_t *ast);

// ---------------- any_inject_ir ----------------
// Injects runtime type information
//void any_inject_ir(ir_builder_t *builder);

// ---------------- any_inject_ast_Any ----------------
// Injects AST struct 'Any'
void any_inject_ast_Any(ast_t *ast);

// ---------------- any_inject_ast_AnyType ----------------
// Injects AST struct 'AnyType'
void any_inject_ast_AnyType(ast_t *ast);

// ---------------- any_inject_ast_AnyTypeKind ----------------
// Injects AST enum 'AnyTypeKind'
void any_inject_ast_AnyTypeKind(ast_t *ast);

// ---------------- any_inject_ast_AnyPtrType ----------------
// Injects AST struct 'AnyPtrType'
void any_inject_ast_AnyPtrType(ast_t *ast);

// ---------------- any_inject_ast_AnyStructType ----------------
// Injects AST struct 'AnyStructType'
void any_inject_ast_AnyStructType(ast_t *ast);

// ---------------- any_inject_ast_AnyFuncPtrType ----------------
// Injects AST struct 'AnyFuncPtrType'
void any_inject_ast_AnyFuncPtrType(ast_t *ast);

// ---------------- any_inject_ast_AnyFixedArrayType ----------------
// Injects AST struct 'AnyFixedArrayType'
void any_inject_ast_AnyFixedArrayType(ast_t *ast);

// ---------------- any_inject_ast___types__ ----------------
// Injects AST global variable '__types__'
void any_inject_ast___types__(ast_t *ast);

// ---------------- any_inject_ast___types_length__ ----------------
// Injects AST global variable '__types_length__'
void any_inject_ast___types_length__(ast_t *ast);

#ifdef __cplusplus
}
#endif

#endif // ANY_H
