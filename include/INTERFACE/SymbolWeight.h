
#ifndef SYMBOL_WEIGHT_H
#define SYMBOL_WEIGHT_H

#include <string>

struct SymbolWeight {
    std::string name;
    std::string label;
    int weight;
    bool isFunction;

    SymbolWeight(const std::string& name, const std::string& label, int weight, bool isFunction);
    bool operator<(const SymbolWeight& other);
};

#endif // SYMBOL_WEIGHT_H