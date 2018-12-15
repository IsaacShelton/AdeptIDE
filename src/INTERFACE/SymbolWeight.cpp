
#include "INTERFACE/SymbolWeight.h"

SymbolWeight::SymbolWeight(const std::string& name, const std::string& label, int weight, bool isFunction){
    this->name = name;
    this->label = label;
    this->weight = weight;
    this->isFunction = isFunction;
}

bool SymbolWeight::operator<(const SymbolWeight& other) const {
    return weight < other.weight;
}
