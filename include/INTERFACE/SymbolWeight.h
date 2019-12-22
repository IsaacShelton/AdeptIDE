
#ifndef SYMBOL_WEIGHT_H
#define SYMBOL_WEIGHT_H

#include <string>

struct SymbolWeight {
    enum Kind {
        FUNCTION, STRUCT, GLOBAL
    };

    std::string name;
    std::string label;
    int weight;
    Kind kind;

    SymbolWeight(const std::string& name, const std::string& label, int weight, Kind kind);
    bool operator<(const SymbolWeight& other) const;
};

#endif // SYMBOL_WEIGHT_H