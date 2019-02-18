
#include "INTERFACE/SyntaxColorPalette.h"

SyntaxColorPalette::SyntaxColorPalette(){}

SyntaxColorPalette::SyntaxColorPalette(SyntaxColorPalette::Defaults defaults){
    this->generate(defaults);
}

void SyntaxColorPalette::generate(SyntaxColorPalette::Defaults defaults){
    switch(defaults){
    case FRUIT_SMOOTHIE:
        this->plain = Vector3f(0.83, 0.83, 0.83);
        this->keyword = Vector3f(0.5f, 0.5f, 1.0f);
        this->comment = Vector3f(0.729411765f, 0.458823529f, 1.0f);
        this->type = Vector3f(1.0f, 0.5f, 0.75f);
        this->string = Vector3f(1.0f, 0.5f, 0.75f);
        this->number = Vector3f(1.0f, 0.5f, 0.75f);
        this->operation = Vector3f(0.0f, 0.74f, 1.0f);
        this->compile_time = Vector3f(1.0f, 0.5f, 1.0f);
        break;
    case VISUAL_STUDIO:
        this->plain = Vector3f(0.83, 0.83, 0.83);
        this->keyword = Vector3f(0.337, 0.612, 0.839);
        this->comment = Vector3f(0.416, 0.6, 0.333);
        this->type = Vector3f(0.306, 0.788, 0.69);
        this->string = Vector3f(0.808, 0.569, 0.471);
        this->number = Vector3f(0.306, 0.788, 0.69);
        this->operation = Vector3f(0.831, 0.831, 0.831);
        this->compile_time = Vector3f(0.698, 0.404, 0.902);
        break;
    case TROPICAL_OCEAN:
        this->plain = Vector3f(0.83, 0.83, 0.83);
        this->keyword = Vector3f(0.10, 0.72, 0.65);
        this->comment = Vector3f(0.33, 0.45, 0.54);
        this->type = Vector3f(0.28, 0.85, 0.60);
        this->string = Vector3f(0.56, 0.83, 0.49);
        this->number = Vector3f(0.56, 0.83, 0.49);
        this->operation = Vector3f(0.83, 0.83, 0.83);
        this->compile_time = Vector3f(0.33, 0.45, 0.54);
        break;
    case ISLAND_CAMPFIRE:
        this->plain = Vector3f(0.83, 0.83, 0.83);
        this->keyword = Vector3f(0.00, 0.57, 0.95);
        this->comment = Vector3f(0.90, 0.33, 0.11);
        this->type = Vector3f(0.93, 0.35, 0.04);
        this->string = Vector3f(0.94, 0.63, 0.04);
        this->number = Vector3f(0.94, 0.63, 0.04);
        this->operation = Vector3f(0.31, 0.67, 0.99);
        this->compile_time = Vector3f(0.90, 0.33, 0.11);
        break;
    case ONE_DARK:
        this->plain = Vector3f(0.83, 0.83, 0.83);
        this->keyword = Vector3f(0.47, 0.56, 0.87);
        this->comment = Vector3f(0.5, 0.5, 0.5);
        this->type = Vector3f(0.34, 0.71, 0.76);
        this->string = Vector3f(0.60, 0.76, 0.47);
        this->number = Vector3f(0.82, 0.60, 0.40);
        this->operation = Vector3f(0.88, 0.42, 0.46);
        this->compile_time = Vector3f(0.78, 0.47, 0.87);
        break;
    }
}
