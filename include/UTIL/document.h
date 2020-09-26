
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>

#define charIsWhitespace(a) (a == ' ' or a == '\n' or a == '\t')
#define charIsOperator(a) (a == '(' or a == ')' or a == '[' or a == ']' or a == '{' or a == '}' or a == ',' or a == '.' or a == ';' or a == '&' or a == '+' or a == '-' or a == '*' or a == '/' or a == '%' or a == '=' or a == ':' or a == '!' or a == '<' or a == '>' or a == '?' or a == '~' or a == '^' or a == '|' or a == '\\')
#define charIsWhitespaceOrOperator(a) (charIsWhitespace(a) || charIsOperator(a))
#define charIsNumeric(a) (a >= '0' and a <= '9')
#define charIsAlphabetical(a) ( (a >= 'a' and a <= 'z') or (a >= 'A' and a <= 'Z') )
#define charIsIdentifier(a) ( (a >= 'a' and a <= 'z') or (a >= 'A' and a <= 'Z') or a == '_' )

size_t getLineNumber(const std::string& text, size_t position);
size_t getLineLength(const std::string& text, size_t beginning);
size_t getCurrentLineLength(const std::string& text, size_t position);
size_t getLineBeginning(const std::string& text, size_t position);
size_t getLineEnd(const std::string& text, size_t position);

#endif // DOCUMENT_H