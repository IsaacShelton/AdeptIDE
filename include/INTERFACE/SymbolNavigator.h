
#ifndef SYMBOL_NAVIGATOR_H_INCLUDED
#define SYMBOL_NAVIGATOR_H_INCLUDED

#include "INSIGHT/insight.h"
#include "INTERFACE/TextBar.h"

class SymbolNavigator : public TextBar {
private:
    void *adeptideRef;
    
public:
    SymbolNavigator(void *adeptideRef);
    void onType();
    void getWhere(compiler_t *compiler, int *outLineNumber, std::string *outFilename);
};

#endif

