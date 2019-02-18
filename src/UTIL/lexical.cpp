
#include "UTIL/lexical.h"

bool isIdentifier(char character){
    return (
        character == '_'  ||
        (character >= 'A' && character <= 'Z') ||
        (character >= 'a' && character <= 'z') ||
        (character >= '0' && character <= '9')
    );
}

bool isWhitespace(char character){
    return (
        character == ' '  ||
        character == '\n' ||
        character == '\r' ||
        character == '\t'
    );
}

bool isOperator(char character){
    return (
        character == '(' ||
        character == ')' ||
        character == '[' ||
        character == ']' ||
        character == '{' ||
        character == '}' ||
        character == '=' ||
        character == '+' ||
        character == '-' ||
        character == '*' ||
        character == '/' ||
        character == '!' ||
        character == '%' ||
        character == '&' ||
        character == '<' ||
        character == '>' ||
        character == ',' ||
        character == '.' ||
        character == ';' ||
        character == ':' ||
        character == '"' ||
        character == '\''
    );
}
