
#include "LEX/lex.h"
#include "LEX/pkg.h"
#include "UTIL/util.h"
#include "UTIL/color.h"
#include "UTIL/search.h"
#include "UTIL/filename.h"

errorcode_t lex(compiler_t *compiler, object_t *object){
    if(!file_text_contents(object->filename, &object->buffer, &object->buffer_length, true)){
        redprintf("The file '%s' doesn't exist or can't be accessed\n", object->filename);
        return FAILURE;
    }

    return lex_buffer(compiler, object);
}

errorcode_t lex_buffer(compiler_t *compiler, object_t *object){
    token_t *t;
    char *buffer = object->buffer;
    int line, column;
    length_t buffer_size = strlen(buffer);
    lex_state_t lex_state;
    tokenlist_t *tokenlist = &object->tokenlist;

    length_t estimate = object->buffer_length / 3;
    if(estimate < 1024) estimate = 1024;

    tokenlist->tokens = malloc(sizeof(token_t) * estimate);
    tokenlist->length = 0;
    tokenlist->capacity = estimate;
    tokenlist->sources = malloc(sizeof(source_t) * estimate);

    // By this point we have a buffer and a tokenlist
    object->compilation_stage = COMPILATION_STAGE_TOKENLIST;

    lex_state_init(&lex_state); // NOTE: lex_state_free() must be called before exiting this function

    lex_state.buildup = malloc(256);
    lex_state.buildup_capacity = 256;

    token_t **tokens = &tokenlist->tokens;
    source_t **sources = &tokenlist->sources;
    length_t object_index = object->index;
    char tmp;

    for(length_t i = 0; i != buffer_size; i++){
        coexpand((void**) &tokenlist->tokens, sizeof(token_t), (void**) &tokenlist->sources,
                sizeof(source_t), tokenlist->length, &tokenlist->capacity, 1, estimate);

        // Macro to map a character to a single token
        #define LEX_SINGLE_TOKEN_MAPPING_MACRO(_token_id, _token_index, _token_stride) { \
            (*sources)[tokenlist->length].index = _token_index; \
            (*sources)[tokenlist->length].object_index = object_index; \
            (*sources)[tokenlist->length].stride = _token_stride; \
            t = &((*tokens)[tokenlist->length++]); \
            t->id = _token_id; \
            t->data = NULL; \
        }

        // Macro to map a character to a single lex state
        #define LEX_SINGLE_STATE_MAPPING_MACRO(state_mapping) { \
            (*sources)[tokenlist->length].index = i; \
            (*sources)[tokenlist->length].object_index = object_index; \
            (*sources)[tokenlist->length].stride = 0; \
            lex_state.state = state_mapping; \
        }

        // Macro to add a token depending on whether an optional character is present
        // (can be used in non LEX_STATE_IDLE states)
        #define LEX_OPTIONAL_MOD_TOKEN_MAPPING(optional_character, if_mod_present, if_mod_absent) { \
            t = &((*tokens)[tokenlist->length]); \
            lex_state.state = LEX_STATE_IDLE; \
            t->data = NULL; \
            if(buffer[i] == optional_character){ \
                t->id = if_mod_present; \
                (*sources)[tokenlist->length++].stride = 2; \
            } else { \
                t->id = if_mod_absent; \
                (*sources)[tokenlist->length++].stride = 1; \
                i--; \
            } \
        }

        #define LEX_OPTIONAL_2MODS_TOKEN_MAPPING(optional_character1, if_mod1_present, optional_character2, if_mod2_present, if_mods_absent) { \
            t = &((*tokens)[tokenlist->length]); \
            lex_state.state = LEX_STATE_IDLE; \
            t->data = NULL; \
            if(buffer[i] == optional_character1){ \
                t->id = if_mod1_present; \
                (*sources)[tokenlist->length++].stride = 2; \
            } else if(buffer[i] == optional_character2){ \
                t->id = if_mod2_present; \
                (*sources)[tokenlist->length++].stride = 2; \
            } else { \
                t->id = if_mods_absent; \
                (*sources)[tokenlist->length++].stride = 1; \
                i--; \
            } \
        }

        switch(lex_state.state){
        case LEX_STATE_IDLE:
            switch(buffer[i]){
            case ' ': case '\t': break;
            case '+': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_ADD);              break;
            case '-': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_SUBTRACT);         break;
            case '*': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_MULTIPLY);         break;
            case '/': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_DIVIDE);           break;
            case '%': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_MODULUS);          break;
            case '&': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_UBERAND);          break;
            case '|': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_UBEROR);           break;
            case '=': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_EQUALS);           break;
            case '<': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_LESS);             break;
            case '>': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_GREATER);          break;
            case '!': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_NOT);              break;
            case ':': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_COLON);            break;
            case '^': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_BIT_XOR);          break;
            case '~': LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_COMPLEMENT);       break;
            //--------------------------------------------------------------------------
            case '(': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_OPEN, i, 1);           break;
            case ')': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_CLOSE, i, 1);          break;
            case '{': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BEGIN, i, 1);          break;
            case '}': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_END, i, 1);            break;
            case ',': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_NEXT, i, 1);           break;
            case '[': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BRACKET_OPEN, i, 1);   break;
            case ']': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BRACKET_CLOSE, i, 1);  break;
            case ';': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_TERMINATE_JOIN, i, 1); break;
            case '?': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_MAYBE, i, 1);          break;
            case '\n': LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_NEWLINE, i, 1);       break;
            case '.':
                (*sources)[tokenlist->length].index = i;
                (*sources)[tokenlist->length].object_index = object_index;
                t = &((*tokens)[tokenlist->length]);
                t->data = NULL;
                if(buffer[i + 1] == '.'){
                    if(buffer[i + 2] == '.'){
                        t->id = TOKEN_ELLIPSIS;
                        i += 2;
                        (*sources)[tokenlist->length++].stride = 3;
                    } else {
                        t->id = TOKEN_RANGE;
                        i += 1;
                        (*sources)[tokenlist->length++].stride = 2;
                    }
                } else {
                    t->id = TOKEN_MEMBER;
                    (*sources)[tokenlist->length++].stride = 1;
                }
                break;
            case '"':
                LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_STRING);
                lex_state.buildup_length = 0;
                lex_state.buildup_inner_stride = 0;
                break;
            case '\'':
                LEX_SINGLE_STATE_MAPPING_MACRO(LEX_STATE_CSTRING);
                lex_state.buildup_length = 0;
                lex_state.buildup_inner_stride = 0;
                break;
            case '#':
                (*sources)[tokenlist->length].index = i;
                (*sources)[tokenlist->length].object_index = object_index;
                (*sources)[tokenlist->length].stride = 0;
                lex_state.buildup_length = 0;
                lex_state.state = LEX_STATE_META;
                break;
            case '$':
                (*sources)[tokenlist->length].index = i;
                (*sources)[tokenlist->length].object_index = object_index;
                (*sources)[tokenlist->length].stride = 0;
                lex_state.buildup_length = 0;
                lex_state.state = LEX_STATE_POLYMORPH;
                break;
            default:
                // Test for word
                if(buffer[i] == '_' || (buffer[i] >= 65 && buffer[i] <= 90) || (buffer[i] >= 97 && buffer[i] <= 122) || buffer[i] == '\\'){
                    (*sources)[tokenlist->length].index = i;
                    (*sources)[tokenlist->length].object_index = object_index;
                    (*sources)[tokenlist->length].stride = 0;
                    lex_state.buildup_length = 1;
                    lex_state.buildup[0] = buffer[i];
                    lex_state.state = LEX_STATE_WORD;
                    break;
                }

                // Test for number
                if(buffer[i] >= '0' && buffer[i] <= '9'){
                    if(lex_state.buildup == NULL){
                        lex_state.buildup = malloc(256);
                        lex_state.buildup_capacity = 256;
                    }
                    (*sources)[tokenlist->length].index = i;
                    (*sources)[tokenlist->length].object_index = object_index;
                    (*sources)[tokenlist->length].stride = 0;
                    lex_state.buildup_length = 1;
                    lex_state.buildup[0] = buffer[i];
                    lex_state.state = LEX_STATE_NUMBER;
                    lex_state.is_hexadecimal = false;
                    lex_state.can_exp = true;
                    lex_state.can_exp_neg = true;
                    break;
                }

                lex_get_location(buffer, i, &line, &column);
                redprintf("%s:%d:%d: Unrecognized symbol '%c' (0x%02X)\n", filename_name_const(object->filename), line, column, buffer[i], (int) buffer[i]);
                
                source_t here;
                here.index = i;
                here.object_index = object->index;
                here.stride = 0;
                compiler_print_source(compiler, line, here);
                lex_state_free(&lex_state);
                return FAILURE;
            }
            break;
        case LEX_STATE_WORD:
            tmp = buffer[i];

            if(tmp == '_' || (tmp >= 65 && tmp <= 90) || (tmp >= 97 && tmp <= 122) || (tmp >= '0' && tmp <= '9') || tmp == '\\'){
                expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);
                lex_state.buildup[lex_state.buildup_length++] = buffer[i];
                break;
            } else if(tmp == ':' && i + 1 < buffer_size){
                tmp = buffer[i + 1];
                if(tmp == '_' || (tmp >= 65 && tmp <= 90) || (tmp >= 97 && tmp <= 122) || (tmp >= '0' && tmp <= '9') || tmp == '\\'){
                    expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);
                    lex_state.buildup[lex_state.buildup_length++] = '\\';
                    break;
                }
            }

            // We have reached the end of the word

            // Terminate string buildup buffer
            expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);
            lex_state.buildup[lex_state.buildup_length] = '\0';

            // Search for string inside keyword list
            maybe_index_t array_index = binary_string_search(global_token_keywords_list, global_token_keywords_list_length, lex_state.buildup);

            if(array_index != -1){
                // Is a keyword, figure out token index from array index
                t = &((*tokens)[tokenlist->length]);
                t->id = BEGINNING_OF_KEYWORD_TOKENS + (unsigned int) array_index; // Values 0x00000050..0x0000009F are reserved for keywords
                t->data = NULL;
            } else if(strcmp(lex_state.buildup, "elif") == 0){
                // Is a shorthand keyword
                t = &((*tokens)[tokenlist->length]);
                t->id = TOKEN_ELSE;
                t->data = NULL;
                (*sources)[tokenlist->length++].stride = lex_state.buildup_length;

                coexpand((void**) &tokenlist->tokens, sizeof(token_t), (void**) &tokenlist->sources,
                    sizeof(source_t), tokenlist->length, &tokenlist->capacity, 1, estimate);

                t = &((*tokens)[tokenlist->length]);
                t->id = TOKEN_IF;
                t->data = NULL;
            } else {
                // Isn't a keyword, just an identifier
                t = &((*tokens)[tokenlist->length]);
                t->id = TOKEN_WORD;
                t->data = malloc(lex_state.buildup_length + 1);
                memcpy(t->data, lex_state.buildup, lex_state.buildup_length);
                ((char*) t->data)[lex_state.buildup_length] = '\0';
            }

            (*sources)[tokenlist->length++].stride = lex_state.buildup_length;
            lex_state.state = LEX_STATE_IDLE;
            i--;
            break;
        case LEX_STATE_STRING:
            if(buffer[i] == '\"'){
                // End of string literal
                t = &((*tokens)[tokenlist->length]);
                t->id = TOKEN_STRING;

                t->data = malloc(sizeof(token_string_data_t));
                ((token_string_data_t*) t->data)->array = malloc(lex_state.buildup_length + 1);
                ((token_string_data_t*) t->data)->length = lex_state.buildup_length;
                memcpy(((token_string_data_t*) t->data)->array, lex_state.buildup, lex_state.buildup_length);

                // Will null terminate for convenience of converting to c-string
                ((token_string_data_t*) t->data)->array[lex_state.buildup_length] = '\0';
                (*sources)[tokenlist->length++].stride = lex_state.buildup_inner_stride + 2;
                lex_state.state = LEX_STATE_IDLE;
                break;
            }

            expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);

            if(buffer[i] == '\\'){
                lex_state.buildup_inner_stride += 2;

                switch(buffer[++i]){
                case 'n': lex_state.buildup[lex_state.buildup_length++] = '\n';  break;
                case 'r': lex_state.buildup[lex_state.buildup_length++] = '\r';  break;
                case 't': lex_state.buildup[lex_state.buildup_length++] = '\t';  break;
                case 'b': lex_state.buildup[lex_state.buildup_length++] = '\b';  break;
                case 'e': lex_state.buildup[lex_state.buildup_length++] = '\e';  break;
                case '0': lex_state.buildup[lex_state.buildup_length++] = '\0';  break;
                case '"': lex_state.buildup[lex_state.buildup_length++] = '"';   break;
                case '\'': lex_state.buildup[lex_state.buildup_length++] = '\''; break;
                case '\\': lex_state.buildup[lex_state.buildup_length++] = '\\'; break;
                default:
                    lex_get_location(buffer, i, &line, &column);
                    redprintf("%s:%d:%d: Unknown escape sequence '\\%c'\n", filename_name_const(object->filename), line, column, buffer[i]);
                    compiler_print_source(compiler, line, (source_t){i - 1, object_index, 2});
                    lex_state_free(&lex_state);
                    return FAILURE;
                }
            } else {
                lex_state.buildup[lex_state.buildup_length++] = buffer[i];
                lex_state.buildup_inner_stride++;
            }
            break;
        case LEX_STATE_CSTRING:
            if(buffer[i] == '\''){
                // End of string literal
                t = &((*tokens)[tokenlist->length]);

                // Character literals
                if(lex_state.buildup_length == 1){
                    if(buffer[i + 1] == 'u' && buffer[i + 2] == 'b'){
                        t->id = TOKEN_UBYTE;
                        t->data = malloc(sizeof(unsigned char));
                        *((unsigned char*) t->data) = lex_state.buildup[0];
                        lex_state.state = LEX_STATE_IDLE;
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_inner_stride + 4;
                        i += 2;
                        break;
                    } else if(buffer[i + 1] == 's' && buffer[i + 2] == 'b'){
                        t->id = TOKEN_BYTE;
                        t->data = malloc(sizeof(char));
                        *((char*) t->data) = lex_state.buildup[0];
                        lex_state.state = LEX_STATE_IDLE;
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_inner_stride + 4;
                        i += 2;
                        break;
                    }
                }


                // Regular cstring
                t->id = TOKEN_CSTRING;
                t->data = malloc(lex_state.buildup_length + 1);
                memcpy(t->data, lex_state.buildup, lex_state.buildup_length);
                ((char*) t->data)[lex_state.buildup_length] = '\0';
                lex_state.state = LEX_STATE_IDLE;
                (*sources)[tokenlist->length++].stride = lex_state.buildup_inner_stride + 2;
                break;
            }

            expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);

            if(buffer[i] == '\\'){
                lex_state.buildup_inner_stride += 2;
                switch(buffer[++i]){
                case 'n': lex_state.buildup[lex_state.buildup_length++] = '\n'; break;
                case 'r': lex_state.buildup[lex_state.buildup_length++] = '\r'; break;
                case 't': lex_state.buildup[lex_state.buildup_length++] = '\t'; break;
                case 'b': lex_state.buildup[lex_state.buildup_length++] = '\b'; break;
                case 'e': lex_state.buildup[lex_state.buildup_length++] = '\e'; break;
                case '\'': lex_state.buildup[lex_state.buildup_length++] = '\''; break;
                case '\\': lex_state.buildup[lex_state.buildup_length++] = '\\'; break;
                default:
                    lex_get_location(buffer, i, &line, &column);
                    redprintf("%s:%d:%d: Unknown escape sequence '\\%c'\n", filename_name_const(object->filename), line, column, buffer[i]);
                    compiler_print_source(compiler, line, (source_t){i - 1, object_index, 2});
                    lex_state_free(&lex_state);
                    return FAILURE;
                }
            } else {
                // Check for cheeky null character
                if(buffer[i] == '\0'){
                    lex_get_location(buffer, i, &line, &column);
                    redprintf("%s:%d:%d: Raw null character found in string\n", filename_name_const(object->filename), line, column);
                    lex_state_free(&lex_state);
                    return FAILURE;
                }
                
                lex_state.buildup[lex_state.buildup_length++] = buffer[i];
                lex_state.buildup_inner_stride++;
            }
            break;
        case LEX_STATE_EQUALS:     LEX_OPTIONAL_2MODS_TOKEN_MAPPING('=', TOKEN_EQUALS, '>', TOKEN_STRONG_ARROW, TOKEN_ASSIGN); break;
        case LEX_STATE_NOT:        LEX_OPTIONAL_2MODS_TOKEN_MAPPING('=', TOKEN_NOTEQUALS, '!', TOKEN_TOGGLE, TOKEN_NOT); break;
        case LEX_STATE_COLON:      LEX_OPTIONAL_MOD_TOKEN_MAPPING(':', TOKEN_ASSOCIATE, TOKEN_COLON);                    break;
        case LEX_STATE_ADD:
            LEX_OPTIONAL_2MODS_TOKEN_MAPPING('=', TOKEN_ADD_ASSIGN, '+', TOKEN_INCREMENT, TOKEN_ADD);
            break;
        case LEX_STATE_MULTIPLY:   LEX_OPTIONAL_MOD_TOKEN_MAPPING('=', TOKEN_MULTIPLY_ASSIGN, TOKEN_MULTIPLY);   break;
        case LEX_STATE_MODULUS:    LEX_OPTIONAL_MOD_TOKEN_MAPPING('=', TOKEN_MODULUS_ASSIGN,  TOKEN_MODULUS);    break;
        case LEX_STATE_BIT_XOR:    LEX_OPTIONAL_MOD_TOKEN_MAPPING('=', TOKEN_BIT_XOR_ASSIGN,  TOKEN_BIT_XOR);    break;
        case LEX_STATE_COMPLEMENT: LEX_OPTIONAL_MOD_TOKEN_MAPPING('>', TOKEN_GIVES, TOKEN_BIT_COMPLEMENT);       break;
        case LEX_STATE_LESS:
            if(buffer[i] == '<'){
                // We don't need to check whether i + 1 exceeds the buffer length because we
                // know that the final character in the buffer is a newline
                if(buffer[i + 1] == '<'){
                    if(buffer[i + 2] == '='){
                        // Logical Left Shift Assign
                        LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_LGC_LS_ASSIGN, i - 1, 4);
                        i += 2;
                    } else {
                        // Logical Left Shift
                        LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_LGC_LSHIFT, i - 1, 3);
                        i++;
                    }
                } else if(buffer[i + 1] == '='){
                    // Left Shift Assign
                    LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_LS_ASSIGN, i - 1, 3);
                    i++;
                } else {
                    // Logical Left Shift
                    LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_LSHIFT, i - 1, 2);
                }
            } else if(buffer[i] == '='){
                LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_LESSTHANEQ, i - 1, 2);
            } else {
                LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_LESSTHAN, --i, 1);
            }
            lex_state.state = LEX_STATE_IDLE;
            break;
        case LEX_STATE_GREATER:
            if(buffer[i] == '>'){
                // We don't need to check whether i + 1 exceeds the buffer length because we
                // know that the final character in the buffer is a newline
                if(buffer[i + 1] == '>'){
                    if(buffer[i + 2] == '='){
                        // Logical Right Shift
                        LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_LGC_RS_ASSIGN, i - 1, 3);
                        i += 2;
                    } else {
                        // Logical Right Shift
                        LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_LGC_RSHIFT, i - 1, 3);
                        i++;
                    }
                } else if(buffer[i + 1] == '='){
                    // Right Shift Assign
                    LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_RS_ASSIGN, i - 1, 3);
                    i++;
                } else {
                    // Logical Right Shift
                    LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_BIT_RSHIFT, i - 1, 2);
                }
            } else if(buffer[i] == '='){
                LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_GREATERTHANEQ, i - 1, 2);
            } else {
                LEX_SINGLE_TOKEN_MAPPING_MACRO(TOKEN_GREATERTHAN, --i, 1);
            }
            lex_state.state = LEX_STATE_IDLE;
            break;
        case LEX_STATE_UBERAND: LEX_OPTIONAL_2MODS_TOKEN_MAPPING('&', TOKEN_UBERAND, '=', TOKEN_BIT_AND_ASSIGN, TOKEN_ADDRESS); break;
        case LEX_STATE_UBEROR:  LEX_OPTIONAL_2MODS_TOKEN_MAPPING('|', TOKEN_UBEROR,  '=', TOKEN_BIT_OR_ASSIGN,  TOKEN_BIT_OR); break;
        case LEX_STATE_NUMBER:
            expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);

            tmp = buffer[i];
            if(tmp == '_') break; // Ignore underscores in numbers

            bool exp_exception = false;

            if(!lex_state.is_hexadecimal){
                if(lex_state.can_exp && (tmp == 'e' || tmp == 'E') && lex_state.buildup_length != 0){
                    exp_exception = true;
                    lex_state.can_exp = false;
                } else if(tmp == '-' && !lex_state.can_exp && lex_state.can_exp_neg){
                    exp_exception = true;
                    lex_state.can_exp_neg = false;
                }
            }
            
            if((tmp >= '0' && tmp <= '9') || tmp == '.' || exp_exception || (lex_state.is_hexadecimal
            &&((tmp >= 'A' && tmp <= 'F') || (tmp >= 'a' && tmp <= 'f'))) ){
                // Valid numeric digit
                lex_state.buildup[lex_state.buildup_length++] = buffer[i];
            } else if(lex_state.buildup[0] == '0' && lex_state.buildup_length == 1 && (tmp == 'x' || tmp == 'X')){
                // Hexadecimal prefix
                lex_state.buildup_length--; // Don't prefix hexadecimal buildup with '0x'
                lex_state.is_hexadecimal = true;
            } else {
                expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);
                lex_state.buildup[lex_state.buildup_length] = '\0';

                t = &((*tokens)[tokenlist->length]);
                lex_state.state = LEX_STATE_IDLE;

                length_t num_dots = string_count_character(lex_state.buildup, lex_state.buildup_length, '.');
                int base = lex_state.is_hexadecimal ? 16 : 10;

                if(num_dots != 0 && lex_state.is_hexadecimal){
                    // Don't allow dots in hexadecimal numbers
                    lex_get_location(buffer, i, &line, &column);
                    redprintf("%s:%d:%d: Hexadecimal numbers cannot contain dots\n", filename_name_const(object->filename), line, column);
                    lex_state_free(&lex_state);
                    return FAILURE;
                }
                
                switch(buffer[i]){
                case 'u':
                    switch(i == buffer_size ? /*unused suffix*/ 'k' : buffer[++i]){
                    case 'b':
                        t->id = TOKEN_UBYTE;
                        t->data = malloc(sizeof(adept_ubyte));
                        *((adept_ubyte*) t->data) = string_to_uint8(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    case 's':
                        t->id = TOKEN_USHORT;
                        t->data = malloc(sizeof(adept_ushort));
                        *((adept_ushort*) t->data) = string_to_uint16(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    case 'i':
                        t->id = TOKEN_UINT;
                        t->data = malloc(sizeof(adept_uint));
                        *((adept_uint*) t->data) = string_to_uint32(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    case 'l':
                        t->id = TOKEN_ULONG;
                        t->data = malloc(sizeof(adept_ulong));
                        *((adept_ulong*) t->data) = string_to_uint64(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    case 'z':
                        t->id = TOKEN_USIZE;
                        t->data = malloc(sizeof(adept_usize));
                        *((adept_usize*) t->data) = string_to_uint64(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    default:
                        lex_get_location(buffer, i, &line, &column);
                        redprintf("%s:%d:%d: Expected valid number suffix after 'u' base suffix\n", filename_name_const(object->filename), line, column);
                        lex_state_free(&lex_state);
                        return FAILURE;
                    }
                    break;
                case 's':
                    switch(i == buffer_size ? /*unused suffix*/ 'k' : buffer[++i]){
                    case 'b':
                        t->id = TOKEN_BYTE;
                        t->data = malloc(sizeof(adept_byte));
                        *((adept_byte*) t->data) = string_to_int8(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    case 's':
                        t->id = TOKEN_SHORT;
                        t->data = malloc(sizeof(adept_ushort));
                        *((short*) t->data) = string_to_int16(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    case 'i':
                        t->id = TOKEN_INT;
                        t->data = malloc(sizeof(adept_int));
                        *((adept_int*) t->data) = string_to_int32(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    case 'l':
                        t->id = TOKEN_LONG;
                        t->data = malloc(sizeof(adept_long));
                        *((adept_long*) t->data) = string_to_int64(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                        i++;
                        break;
                    default:
                        t->id = TOKEN_SHORT;
                        t->data = malloc(sizeof(adept_short));
                        *((adept_short*) t->data) = string_to_int16(lex_state.buildup, base);
                        (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 2;
                    }
                    break;
                case 'b':
                    t->id = TOKEN_BYTE;
                    t->data = malloc(sizeof(adept_byte));
                    *((adept_byte*) t->data) = string_to_int8(lex_state.buildup, base);
                    (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 1;
                    i++;
                    break;
                case 'i':
                    t->id = TOKEN_INT;
                    t->data = malloc(sizeof(adept_int));
                    *((adept_int*) t->data) = string_to_int32(lex_state.buildup, base);
                    (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 1;
                    i++;
                    break;
                case 'l':
                    t->id = TOKEN_LONG;
                    t->data = malloc(sizeof(adept_long));
                    *((adept_long*) t->data) = string_to_int64(lex_state.buildup, base);
                    (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 1;
                    i++;
                    break;
                case 'f':
                    t->id = TOKEN_FLOAT;
                    t->data = malloc(sizeof(adept_float));
                    *((adept_float*) t->data) = string_to_float32(lex_state.buildup);
                    (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 1;
                    i++;
                    break;
                case 'd':
                    t->id = TOKEN_DOUBLE;
                    t->data = malloc(sizeof(adept_double));
                    *((adept_double*) t->data) = string_to_float64(lex_state.buildup);
                    (*sources)[tokenlist->length++].stride = lex_state.buildup_length + 1;
                    i++;
                    break;
                default:
                    (*sources)[tokenlist->length++].stride = lex_state.buildup_length;
                    
                    switch(num_dots){
                    case 0: // Number is generic integer
                        if(string_to_int_must_be_uint64(lex_state.buildup, lex_state.buildup_length, base)){
                            // Numbers that cannot be expressed using int64 will be promoted to uint64
                            t->id = TOKEN_ULONG;
                            t->data = malloc(sizeof(adept_ulong));
                            *((adept_ulong*) t->data) = string_to_uint64(lex_state.buildup, base);
                        } else {
                            // Otherwise, default to normal generic integer
                            t->id = TOKEN_GENERIC_INT;
                            t->data = malloc(sizeof(adept_generic_int));
                            *((adept_generic_int*) t->data) = string_to_int64(lex_state.buildup, base);
                        }
                        break;
                    case 1: // Number is generic floating point
                        t->id = TOKEN_GENERIC_FLOAT;
                        t->data = malloc(sizeof(adept_generic_float));
                        *((adept_generic_float*) t->data) = string_to_float64(lex_state.buildup);
                        break;
                    default:
                        // Error: dots_count > 1
                        lex_get_location(buffer, i, &line, &column);
                        redprintf("%s:%d:%d: Numbers cannot contain multiple dots\n", filename_name_const(object->filename), line, column);
                        lex_state_free(&lex_state);
                        return FAILURE;
                    }
                    break;
                }
                i--;
            }
            break;
        case LEX_STATE_SUBTRACT:
            // Test for number
            if(buffer[i] >= '0' && buffer[i] <= '9'){
                if(lex_state.buildup == NULL){
                    lex_state.buildup = malloc(256);
                    lex_state.buildup_capacity = 256;
                }
                lex_state.buildup_length = 2;
                lex_state.buildup_inner_stride = 2;
                lex_state.buildup[0] = '-';
                lex_state.buildup[1] = buffer[i];
                lex_state.state = LEX_STATE_NUMBER;
                lex_state.is_hexadecimal = false;
                lex_state.can_exp = true;
                lex_state.can_exp_neg = true;
                break;
            }

            t = &((*tokens)[tokenlist->length]);
            lex_state.state = LEX_STATE_IDLE;
            t->data = NULL;
            if(buffer[i] == '='){
                t->id = TOKEN_SUBTRACT_ASSIGN;
                (*sources)[tokenlist->length++].stride = 2;
            } else if(buffer[i] == '-'){
                t->id = TOKEN_DECREMENT;
                (*sources)[tokenlist->length++].stride = 2;
            } else {
                t->id = TOKEN_SUBTRACT;
                (*sources)[tokenlist->length++].stride = 1;
                i--;
            }
            break;
        case LEX_STATE_DIVIDE:
            switch(buffer[i]){
            case '/':
                lex_state.state = LEX_STATE_LINECOMMENT;
                break;
            case '*':
                lex_state.state = LEX_STATE_LONGCOMMENT;
                break;
            case '=':
                lex_state.state = LEX_STATE_IDLE;
                t = &((*tokens)[tokenlist->length]);
                t->id = TOKEN_DIVIDE_ASSIGN;
                t->data = NULL;
                (*sources)[tokenlist->length++].stride = 2;
                break;
            default:
                lex_state.state = LEX_STATE_IDLE;
                t = &((*tokens)[tokenlist->length]);
                t->id = TOKEN_DIVIDE;
                t->data = NULL;
                (*sources)[tokenlist->length++].stride = 1;
                i--;
            }
            break;
        case LEX_STATE_LINECOMMENT:
            if(buffer[i] == '\n') { lex_state.state = LEX_STATE_IDLE; i--; }
            break;
        case LEX_STATE_LONGCOMMENT:
            if(buffer[i] == '*') lex_state.state = LEX_STATE_ENDCOMMENT;
            break;
        case LEX_STATE_ENDCOMMENT: // End Multiline Comment
            if(buffer[i] == '/') { lex_state.state = LEX_STATE_IDLE; }
            else lex_state.state = LEX_STATE_LONGCOMMENT;
            break;
        case LEX_STATE_META: case LEX_STATE_POLYMORPH: case LEX_STATE_POLYCOUNT:
            tmp = buffer[i];
            if(tmp == '_' || (tmp >= 65 && tmp <= 90) || (tmp >= 97 && tmp <= 122) || (tmp >= '0' && tmp <= '9')){
                expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);
                lex_state.buildup[lex_state.buildup_length++] = buffer[i];
            } else if(lex_state.buildup_length == 0 && lex_state.state == LEX_STATE_POLYMORPH && tmp == '~'){
                // Allow tilde for first character of polymorph
                expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);
                lex_state.buildup[lex_state.buildup_length++] = tmp;
            } else if(lex_state.buildup_length == 0 && lex_state.state == LEX_STATE_POLYMORPH && tmp == '#'){
                // Allow tilde for first character of polymorph
                lex_state.state = LEX_STATE_POLYCOUNT;
            } else {
                // Terminate string buildup buffer
                expand((void**) &lex_state.buildup, sizeof(char), lex_state.buildup_length, &lex_state.buildup_capacity, 1, 256);
                lex_state.buildup[lex_state.buildup_length] = '\0';

                (*sources)[tokenlist->length].stride = lex_state.buildup_length + (lex_state.state == LEX_STATE_POLYCOUNT ? 2 : 1);
                t = &((*tokens)[tokenlist->length++]);

                switch(lex_state.state){
                case LEX_STATE_META:
                    t->id = TOKEN_META;
                    break;
                case LEX_STATE_POLYMORPH:
                    t->id = TOKEN_POLYMORPH;
                    break;
                case LEX_STATE_POLYCOUNT:
                    t->id = TOKEN_POLYCOUNT;
                    
                    break;
                }

                t->data = malloc(lex_state.buildup_length + 1);
                memcpy(t->data, lex_state.buildup, lex_state.buildup_length);
                ((char*) t->data)[lex_state.buildup_length] = '\0';

                i--; lex_state.state = LEX_STATE_IDLE;
            }
            break;
        }

        #undef LEX_SINGLE_TOKEN_MAPPING_MACRO
        #undef LEX_SINGLE_STATE_MAPPING_MACRO
        #undef LEX_OPTIONAL_MOD_TOKEN_MAPPING
        #undef LEX_OPTIONAL_2MODS_TOKEN_MAPPING
    }

    if(lex_state.state != LEX_STATE_IDLE){
        lex_get_location(buffer, (*sources)[tokenlist->length].index, &line, &column);
        switch(lex_state.state){
        case LEX_STATE_STRING:
        case LEX_STATE_CSTRING:
            redprintf("%s:%d:%d: Unterminated string literal\n", filename_name_const(object->filename), line, column);
            (*sources)[tokenlist->length].stride = 1;
            break;
        case LEX_STATE_LONGCOMMENT:
        case LEX_STATE_ENDCOMMENT:
            (*sources)[tokenlist->length].stride = 2;
            redprintf("%s:%d:%d: Unterminated multiline comment\n", filename_name_const(object->filename), line, column);
            break;
        }

        compiler_print_source(compiler, line, (*sources)[tokenlist->length]);
        lex_state_free(&lex_state);
        return FAILURE;
    }

    lex_state_free(&lex_state);

    if(compiler->traits & COMPILER_MAKE_PACKAGE){
        if(compiler_create_package(compiler, object) == 0){
            compiler->result_flags |= COMPILER_RESULT_SUCCESS;
        }
        return FAILURE;
    }
    
    return SUCCESS;
}

void lex_state_init(lex_state_t *lex_state){
    lex_state->state = LEX_STATE_IDLE;
    lex_state->buildup = NULL;
    lex_state->buildup_length = 0;
    lex_state->buildup_capacity = 0;
    lex_state->buildup_inner_stride = 0;
}

void lex_state_free(lex_state_t *lex_state){
    free(lex_state->buildup);
}

void lex_get_location(const char *buffer, length_t i, int *line, int *column){
    // NOTE: Expects i to be pointed at the character that caused the error or is the area of interest

    if(i == 0){
        *line = 1; *column = 1; return;
    }

    length_t x, y;
    *line = 1;

    for(x = 0; x != i; x++){
        if(buffer[x] == '\n') (*line)++;
    }

    // TODO: Remove this hacky code
    if(buffer[i] == '\n' && i != 0){
        *column = 1;
        for(y = i-1; y != 0 && buffer[y] != '\n'; y--){
            if(buffer[y] == '\t'){
                (*column) += 4;
            } else {
                (*column)++;
            }
        }
    } else {
        *column = 0;
        for(y = i; y != 0 && buffer[y] != '\n'; y--){
            if(buffer[y] == '\t'){
                (*column) += 4;
            } else {
                (*column)++;
            }
        }
    }

    // NOTE: Under this is really dirty code for correcting the line & column numbers
    // NOTE: Should probably not need this if I fix up the main procedure
    // TODO: Remove this hacky code

    // Do some error correction if on line 1
    if(y == 0){
        if(buffer[0] == '\t'){
            (*column) += 4;
        } else if(buffer[0] != '\n'){
            (*column)++;
        }
    }

    if(*column == 0) *column = 1;
}
