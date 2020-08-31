
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "INTERFACE/quicktype.h"

void quicktype(AdeptIDE *adeptide, int key){
    TextEditor *editor = adeptide->getCurrentEditorAsTextEditor();
    if(!editor) return;

    switch(editor->getFileType()){
    case PLAIN_TEXT:
        break;
    case ADEPT:
        quicktype_adept(adeptide, key);
        break;
    case JAVA:
        quicktype_java(adeptide, key);
        break;
    case HTML:
        break;
    case JSON:
        break;
    case PAINTING:
        break;
    }
}

void quicktype_adept(AdeptIDE *adeptide, int key){
    bool shift = (glfwGetKey(adeptide->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(adeptide->window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    bool control = (glfwGetKey(adeptide->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(adeptide->window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

    if(control && shift){
        switch(key){
        case GLFW_KEY_A: adeptide->type("and "); return;
        case GLFW_KEY_B: adeptide->type("byte"); return;
        case GLFW_KEY_C: adeptide->type("ubyte"); return;
        case GLFW_KEY_D: adeptide->type("def "); return;
        case GLFW_KEY_F: adeptide->type("fallthrough"); return;
        case GLFW_KEY_I: adeptide->type("int"); return;
        case GLFW_KEY_J: adeptide->type("uint"); return;
        case GLFW_KEY_L: adeptide->type("long"); return;
        case GLFW_KEY_M: adeptide->type("ulong"); return;
        case GLFW_KEY_N: adeptide->type("null"); return;
        case GLFW_KEY_O: adeptide->type("or "); return;
        case GLFW_KEY_P: adeptide->type("ptr"); return;
        case GLFW_KEY_S: adeptide->type("short"); return;
        case GLFW_KEY_T: adeptide->type("ushort"); return;
        case GLFW_KEY_U: adeptide->type("undef"); return;
        }
    }

    if(control){
        switch(key){
        case GLFW_KEY_A: adeptide->type("as "); return;
        case GLFW_KEY_E: adeptide->type("each "); return;
        case GLFW_KEY_I: adeptide->type("in "); return;
        case GLFW_KEY_O: adeptide->type("out "); return;
        case GLFW_KEY_S: adeptide->type("successful "); return;
        case GLFW_KEY_U: adeptide->type("inout "); return;
        }
    }

    if(shift){
        switch(key){
        case GLFW_KEY_A: adeptide->type("alias "); return;
        case GLFW_KEY_B: adeptide->type("break "); return;
        case GLFW_KEY_C: adeptide->type("continue "); return;
        case GLFW_KEY_D: adeptide->type("defer "); return;
        case GLFW_KEY_E: adeptide->type("enum "); return;
        case GLFW_KEY_F: adeptide->type("foreign "); return;
        case GLFW_KEY_I: adeptide->type("import "); return;
        case GLFW_KEY_N: adeptide->type("null"); return;
        case GLFW_KEY_P: adeptide->type("pragma "); return;
        case GLFW_KEY_R: adeptide->type("repeat "); return;
        case GLFW_KEY_S: adeptide->type("struct "); return;
        case GLFW_KEY_U: adeptide->type("until "); return;
        }
    }

    switch(key){
    case GLFW_KEY_A: adeptide->type("as "); return;
    case GLFW_KEY_C: adeptide->type("cast "); return;
    case GLFW_KEY_D: adeptide->type("delete "); return;
    case GLFW_KEY_E: adeptide->type("else "); return;
    case GLFW_KEY_F: adeptide->type("func "); return;
    case GLFW_KEY_I: adeptide->type("if "); return;
    case GLFW_KEY_N: adeptide->type("new "); return;
    case GLFW_KEY_P: adeptide->type("pragma "); return;
    case GLFW_KEY_R: adeptide->type("return "); return;
    case GLFW_KEY_S: adeptide->type("sizeof "); return;
    case GLFW_KEY_T: adeptide->type("this"); return;
    case GLFW_KEY_U: adeptide->type("unless "); return;
    case GLFW_KEY_V: adeptide->type("void "); return;
    case GLFW_KEY_W: adeptide->type("while "); return;
    }
}

void quicktype_java(AdeptIDE *adeptide, int key){
    bool shift = (glfwGetKey(adeptide->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(adeptide->window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    bool control = (glfwGetKey(adeptide->window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(adeptide->window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

    if(control && shift){
        switch(key){
        case GLFW_KEY_B: adeptide->type("boolean "); return;
        case GLFW_KEY_C: adeptide->type("char "); return;
        case GLFW_KEY_D: adeptide->type("double "); return;
        case GLFW_KEY_F: adeptide->type("false"); return;
        case GLFW_KEY_H: adeptide->type("short "); return;
        case GLFW_KEY_I: adeptide->type("int "); return;
        case GLFW_KEY_L: adeptide->type("long "); return;
        case GLFW_KEY_N: adeptide->type("null"); return;
        case GLFW_KEY_O: adeptide->type("Object"); return;
        case GLFW_KEY_P: adeptide->type("protected "); return;
        case GLFW_KEY_S: adeptide->type("String"); return;
        case GLFW_KEY_T: adeptide->type("true"); return;

        case GLFW_KEY_K: adeptide->type("Scanner"); return;
        case GLFW_KEY_M: adeptide->type("Math"); return;
        case GLFW_KEY_Z:
            adeptide->type("System.out.println(\"\");");
            adeptide->moveCaretLeft();
            adeptide->moveCaretLeft();
            adeptide->moveCaretLeft();
            return;
        case GLFW_KEY_X:
            adeptide->type("System.out.printf(\"\");");
            adeptide->moveCaretLeft();
            adeptide->moveCaretLeft();
            adeptide->moveCaretLeft();
            return;
        case GLFW_KEY_Q:
            adeptide->type("System.out.print(\"\");");
            adeptide->moveCaretLeft();
            adeptide->moveCaretLeft();
            adeptide->moveCaretLeft();
            return;
        case GLFW_KEY_R: adeptide->type("Random"); return;
        }
    }

    if(control){
        switch(key){
        case GLFW_KEY_E: adeptide->type("enum "); return;
        case GLFW_KEY_C: adeptide->type("continue"); return;
        case GLFW_KEY_F: adeptide->type("finally "); return;
        case GLFW_KEY_I: adeptide->type("implements "); return;
        case GLFW_KEY_P: adeptide->type("package "); return;
        case GLFW_KEY_S: adeptide->type("super"); return;
        case GLFW_KEY_T: adeptide->type("throws "); return;

        case GLFW_KEY_N: adeptide->type("interface "); return;
        case GLFW_KEY_K: adeptide->type("catch"); return;
        }
    }

    if(shift){
        switch(key){
        case GLFW_KEY_A: adeptide->type("abstract "); return;
        case GLFW_KEY_C: adeptide->type("class "); return;
        case GLFW_KEY_D: adeptide->type("do"); return;
        case GLFW_KEY_E: adeptide->type("extends "); return;
        case GLFW_KEY_F: adeptide->type("final "); return;
        case GLFW_KEY_I: adeptide->type("import "); return;
        case GLFW_KEY_P: adeptide->type("public "); return;
        case GLFW_KEY_S: adeptide->type("static "); return;
        case GLFW_KEY_T: adeptide->type("try"); return;
        case GLFW_KEY_V: adeptide->type("volatile "); return;
        }
    }

    switch(key){
    case GLFW_KEY_B: adeptide->type("break"); return;
    case GLFW_KEY_C: adeptide->type("case "); return;
    case GLFW_KEY_E: adeptide->type("else "); return;
    case GLFW_KEY_D: adeptide->type("default"); return;
    case GLFW_KEY_F: adeptide->type("for"); return;
    case GLFW_KEY_I: adeptide->type("if"); return;
    case GLFW_KEY_N: adeptide->type("new "); return;
    case GLFW_KEY_P: adeptide->type("private "); return;
    case GLFW_KEY_R: adeptide->type("return "); return;
    case GLFW_KEY_S: adeptide->type("switch"); return;
    case GLFW_KEY_T: adeptide->type("this"); return;
    case GLFW_KEY_V: adeptide->type("void "); return;
    case GLFW_KEY_W: adeptide->type("while"); return;
    }
}
