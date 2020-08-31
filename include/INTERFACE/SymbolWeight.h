
#ifndef SYMBOL_WEIGHT_H
#define SYMBOL_WEIGHT_H

#include <string>
#include <vector>
#include "INSIGHT/insight.h"

struct SymbolWeight {
    enum Kind {
        FUNCTION, STRUCT, GLOBAL, CONSTANT, ENUM, FILE
    };

    std::string name;
    std::string label;
    int weight;
    Kind kind;
    source_t source;

    SymbolWeight(const std::string& name, const std::string& label, int weight, Kind kind, source_t source);
    bool operator<(const SymbolWeight& other) const;
};

void nearestSymbols(compiler_t *compiler, std::string optional_object_string, std::string target, std::vector<SymbolWeight> *outSymbols);

#endif // SYMBOL_WEIGHT_H