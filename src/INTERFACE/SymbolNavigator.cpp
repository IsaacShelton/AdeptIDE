
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include "UTIL/strings.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "INTERFACE/SymbolWeight.h"
#include "INTERFACE/SymbolNavigator.h"

SymbolNavigator::SymbolNavigator(){
    this->container = NULL;
    this->constantText = "Go to Symbol: ";
}

void SymbolNavigator::getWhere(compiler_t *compiler, int *outLineNumber, std::string *outFilename){
    // NOTE: Returns 0 when not valid line number
    
    std::vector<SymbolWeight> potentialSymbols;
    nearestSymbols(compiler, this->getInput(), &potentialSymbols);

    if(potentialSymbols.size() == 0){
        *outLineNumber = 0;
        *outFilename = "";
        return;
    }

    source_t source = potentialSymbols[0].source;
    object_t *object = compiler->objects[source.object_index];
    *outFilename = std::string(object->full_filename);

    int line, column;
    lex_get_location(object->buffer, source.index, &line, &column);

    *outLineNumber = line;
}
