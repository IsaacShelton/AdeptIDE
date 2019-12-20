
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include "UTIL/strings.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "INTERFACE/LineNavigator.h"

LineNavigator::LineNavigator(){
    this->container = NULL;
    this->constantText = "Go to Line: ";
}

int LineNavigator::getLineNumber(){
    // NOTE: Returns 0 when not valid line number
    int line = to_int(this->getInput());
    return line < 0 ? 0 : line;
}
