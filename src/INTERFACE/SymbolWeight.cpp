
#include <algorithm>

#include "UTIL/levenshtein.h"
#include "INTERFACE/SymbolWeight.h"

SymbolWeight::SymbolWeight(const std::string& name, const std::string& label, int weight, Kind kind, source_t source){
    this->name = name;
    this->label = label;
    this->weight = weight;
    this->kind = kind;
    this->source = source;
}

bool SymbolWeight::operator<(const SymbolWeight& other) const {
    return this->label.length() != other.label.length() ? this->label.length() < other.label.length() : weight < other.weight;
}

void nearestSymbols(compiler_t *compiler, std::string optional_object_string, std::string target, std::vector<SymbolWeight> *outSymbols){
    if(outSymbols == NULL) return;
    outSymbols->clear();

    // Require compiler insight to find nearest symbols
    if(compiler == NULL) return;

    // Symbol weights that may or may not replace our current symbol weights
    ast_t *ast = &compiler->objects[0]->ast;

    // Generate possible symbol weights
    for(size_t i = 0; i != ast->funcs_length; i++){
        const char *name = ast->funcs[i].name;
        std::string args;

        for(size_t j = 0; j != ast->funcs[i].arity; j++){
            if(ast->funcs[i].arg_names){
                args += std::string(ast->funcs[i].arg_names[j]) + (ast->funcs[i].arg_defaults && ast->funcs[i].arg_defaults[j] ? "? " : " ");
            }

            char *a = ast_type_str(&ast->funcs[i].arg_types[j]);
            args += std::string(a);
            ::free(a);

            // Put question mark after type if no name to put it after
            if(!ast->funcs[i].arg_names && ast->funcs[i].arg_defaults && ast->funcs[i].arg_defaults[j]){
                args += "?";
            }

            if(j + 1 != ast->funcs[i].arity) args += ", ";
        }

        if(ast->funcs[i].traits & AST_FUNC_VARARG) args += ", ...";

        char *r = ast_type_str(&ast->funcs[i].return_type);
        // Is a function
        std::string label = std::string(name) + "(" + args + ") " + r;
        ::free(r);

        // Select by label mode
        if(label.length() < target.length() || strncmp(label.c_str(), target.c_str(), target.length()) != 0) continue;

        outSymbols->push_back(SymbolWeight(name, label, levenshtein_overlapping(target.c_str(), name), SymbolWeight::Kind::FUNCTION, ast->funcs[i].source));
    }
    
    for(size_t i = 0; i != ast->structs_length; i++){
        const char *name = ast->structs[i].name;
        if(strlen(name) < target.length() || strncmp(name, target.c_str(), target.length()) != 0) continue;

        // Is a struct
        outSymbols->push_back(SymbolWeight(name, name, levenshtein_overlapping(target.c_str(), name), SymbolWeight::Kind::STRUCT, ast->structs[i].source));
    }

    for(size_t i = 0; i != ast->globals_length; i++){
        const char *name = ast->globals[i].name;
        if(strlen(name) < target.length() || strncmp(name, target.c_str(), target.length()) != 0) continue;

        // Is a global
        outSymbols->push_back(SymbolWeight(name, name, levenshtein_overlapping(target.c_str(), name), SymbolWeight::Kind::GLOBAL, ast->globals[i].source));
    }

    for(size_t i = 0; i != ast->constants_length; i++){
        const char *name = ast->constants[i].name;
        if(strlen(name) < target.length() || strncmp(name, target.c_str(), target.length()) != 0) continue;
        // Is a constant expression
        outSymbols->push_back(SymbolWeight(name, name, levenshtein_overlapping(target.c_str(), name), SymbolWeight::Kind::CONSTANT, ast->constants[i].source));
    }
    
    // Sort the possible new symbol weights by weight
    std::stable_sort(outSymbols->begin(), outSymbols->end());
}
