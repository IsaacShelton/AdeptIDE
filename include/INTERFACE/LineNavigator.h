
#ifndef LINE_NAVIGATOR_H_INCLUDED
#define LINE_NAVIGATOR_H_INCLUDED

#include "INTERFACE/TextBar.h"

class LineNavigator : public TextBar {
public:
    LineNavigator();
    int getLineNumber();
};

#endif
