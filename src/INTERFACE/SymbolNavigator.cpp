
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include "UTIL/strings.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "INTERFACE/AdeptIDE.h"
#include "INTERFACE/SymbolWeight.h"
#include "INTERFACE/SymbolNavigator.h"

SymbolNavigator::SymbolNavigator(void *adeptideRef){
    assert(adeptideRef != NULL);

    this->container = NULL;
    this->constantText = "Go to Symbol: ";
    this->adeptideRef = adeptideRef;
}

void SymbolNavigator::onType(){
    std::vector<SymbolWeight> potentialSymbols;

    // Borrow insight information from current text editor
    AdeptIDE *adeptide = static_cast<AdeptIDE*>(this->adeptideRef);

    if(TextEditor *textEditor = adeptide->getCurrentEditorAsTextEditor()){
        compiler_t *compiler = textEditor->borrowCompiler();
        nearestSymbols(compiler, "", this->getInput(), &potentialSymbols);
        textEditor->returnCompiler();
    }

    std::string result;
    bool requireNewline = false;

    for(size_t i = 0; i < potentialSymbols.size() && i < 10; i++){
        if(requireNewline) result += "\n";
        else               requireNewline = true;

        result += potentialSymbols[i].label.length() > 60 ? potentialSymbols[i].label.substr(0, 58) + "..": potentialSymbols[i].label;
    }

    this->setAdditionalText(result);
}

void SymbolNavigator::getWhere(compiler_t *compiler, int *outLineNumber, std::string *outFilename){
    // NOTE: Returns 0 when not valid line number
    
    std::vector<SymbolWeight> potentialSymbols;
    nearestSymbols(compiler, "", this->getInput(), &potentialSymbols);

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
