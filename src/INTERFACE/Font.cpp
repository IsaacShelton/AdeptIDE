
#include <fstream>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "UTIL/strings.h"
#include "INTERFACE/Font.h"

#define chariswhitespace(a) (a == ' ' or a == '\n' or a == '\t')
#define charisoperator(a) (a == '(' or a == ')' or a == '[' or a == ']' or a == '{' or a == '}' or a == ',' or a == '.' or a == ';' or a == '&' or a == '+' or a == '-' or a == '*' or a == '/' or a == '%' or a == '=' or a == ':')
#define chariswhitespaceoroperator(a) (chariswhitespace(a) || charisoperator(a))
#define charisnumeric(a) (a >= '0' and a <= '9')
#define charisalphabetical(a) ( (a >= 'a' and a <= 'z') or (a >= 'A' and a <= 'Z') )
#define charisidentifier(a) ( (a >= 'a' and a <= 'z') or (a >= 'A' and a <= 'Z') or a == '_' )

#define prevchariswhitespace() (i != 0 ? chariswhitespace(text[i-1]) : true)
#define prevcharisnumeric() (i != 0 ? charisnumeric(text[i-1]) : true)
#define prevchariswhitespaceornumeric() (i != 0 ? (charisnumeric(text[i-1]) or chariswhitespace(text[i-1])) : true)
#define prevcharisoperator() (i != 0 ? (charisoperator(text[i-1])) : true)
#define prevchariswhitespaceoroperator() (i != 0 ? (chariswhitespaceoroperator(text[i-1])) : true)
#define prevcharisidentifier() (i != 0 ? (charisalphabetical(text[i-1]) or text[i-1] == '_') : true)

FontCharacter::FontCharacter(int id, double u, double v, double u_max, double v_max, double x_offset, double y_offset, double size_x, double size_y, double advance){
    this->id = id;
    this->u = u;
    this->v = v;
    this->u_max = u_max;
    this->v_max = v_max;
    this->x_offset = x_offset;
    this->y_offset = y_offset;
    this->size_x = size_x;
    this->size_y = size_y;
    this->advance = advance;
}

AttributeEntry::SubAttribute* AttributeEntry::getSubAttribute(const std::string& name){
    for(AttributeEntry::SubAttribute& sub : sub_attributes){
        if(sub.name == name) return &sub;
    }

    return NULL;
}

void Font::load(const std::string& filename){
    loadData(filename);
    generateCharacters();
}

void Font::loadData(const std::string& filename){
    std::ifstream font_file(filename);
    std::string line;

    if(!font_file.is_open()){
        std::cerr << "Failed to load '" << filename << "'" << std::endl;
        exit(1);
    }

    while(getline(font_file, line)){
        if(line.length() == 0) continue;
        line += " "; // for easier parsing

        std::string name;
        std::vector<AttributeEntry::SubAttribute> sub_attributes;
        size_t i = 0;

        while(line[i] != ' '){
            name += line[i];
            i++;
        }
        i++;

        while(line[i] == ' ') i++;

        while(i != line.length()){
            std::string attr;
            std::string value;
            while(line[i] != '=') attr += line[i++];

            if(line[i++] != '='){
                std::cerr << "Expected '=' after attribute in font file" << std::endl;
                exit(1);
            }

            while(line[i] != ' ') value += line[i++];
            while(line[i] == ' ') i++;
            sub_attributes.push_back( AttributeEntry::SubAttribute{attr, value} );
        }

        attributes.push_back( AttributeEntry{name, sub_attributes} );
    }

    font_file.close();
}

void Font::generateCharacters(){
    AttributeEntry* info_data = getAttribute("info");
    AttributeEntry* common_data = getAttribute("common");

    assert(info_data != NULL);
    assert(common_data != NULL);

    AttributeEntry::SubAttribute* scaleW = common_data->getSubAttribute("scaleW");
    AttributeEntry::SubAttribute* scaleH = common_data->getSubAttribute("scaleH");
    AttributeEntry::SubAttribute* lineHeight = common_data->getSubAttribute("lineHeight");

    assert(scaleW != NULL);
    assert(scaleH != NULL);

    width = to_float(scaleW->value);
    height = to_float(scaleH->value);
    line_height = to_int(lineHeight->value);

    this->mono_character_width = 0;

    for(AttributeEntry& entry : attributes){
        if(entry.name == "char"){
            // This is a character!

            AttributeEntry::SubAttribute* id = entry.getSubAttribute("id");
            AttributeEntry::SubAttribute* x = entry.getSubAttribute("x");
            AttributeEntry::SubAttribute* y = entry.getSubAttribute("y");
            AttributeEntry::SubAttribute* w = entry.getSubAttribute("width");
            AttributeEntry::SubAttribute* h = entry.getSubAttribute("height");
            AttributeEntry::SubAttribute* xoffset = entry.getSubAttribute("xoffset");
            AttributeEntry::SubAttribute* yoffset = entry.getSubAttribute("yoffset");
            AttributeEntry::SubAttribute* xadvance = entry.getSubAttribute("xadvance");
            
            assert(id != NULL);
            assert(x != NULL);
            assert(y != NULL);
            assert(w != NULL);
            assert(h != NULL);
            assert(xoffset != NULL);
            assert(yoffset != NULL);
            assert(xadvance != NULL);

            int char_id = to_int(id->value);
            float char_u = to_float(x->value) / width;
            float char_v = to_float(y->value) / height;
            float char_x_offset = to_float(xoffset->value);
            float char_y_offset = to_float(yoffset->value);
            float char_size_x = to_float(w->value);
            float char_size_y = to_float(h->value);
            float char_u_max = char_u + char_size_x / width;
            float char_v_max = char_v + char_size_y / height;
            float char_advance = to_float(xadvance->value) * 0.7f;

            if(char_id == '.'){
                char_x_offset += 10.0f;

                // Take advance for space from advance for period
                if(this->mono_character_width == 0)
                    mono_character_width = char_advance;
            }

            characters.push_back( FontCharacter(char_id, char_u, char_v, char_u_max, char_v_max, char_x_offset, char_y_offset, char_size_x, char_size_y, char_advance) );
        }
    }
}

AttributeEntry* Font::getAttribute(const std::string& name){
    for(AttributeEntry& attr : attributes){
        if(attr.name == name) return &attr;
    }

    return NULL;
}

TextModel Font::generatePlainTextModel(const std::string& text, float scale, Vector3f color){
    float cursor_x = 0.0f;
    float cursor_y = 0.0f; //(float) line_height;
    std::vector<unsigned int> indices;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<float> text_colors;
    FontCharacter* character;
    size_t index = 0;

    for(char ascii : text){
        // Newline if newline characater
        if(ascii == '\n'){
            cursor_x = 0.0f;
            cursor_y += line_height * scale;
            continue;
        } else if (ascii == ' '){
            cursor_x += this->mono_character_width * scale;
            continue;
        } else if(ascii == '\t'){
            cursor_x += this->mono_character_width * scale * 4;
            continue;
        }

        // Find the character data for the character
        character = NULL;
        for(FontCharacter& font_char : characters){
            if(font_char.id == ascii){
                character = &font_char;
                break;
            }
        }

        // Don't render anything if the character wasn't found
        if(character == NULL) continue;

        vertices.push_back(cursor_x + character->x_offset * scale);
        vertices.push_back(cursor_y + character->y_offset * scale);

        vertices.push_back(cursor_x + character->x_offset * scale);
        vertices.push_back(cursor_y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(cursor_x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(cursor_y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(cursor_x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(cursor_y + character->y_offset * scale);


        indices.push_back(index);
        indices.push_back(index+1);
        indices.push_back(index+2);
        indices.push_back(index+2);
        indices.push_back(index+3);
        indices.push_back(index);
        index += 4;

        uvs.push_back(character->u);
        uvs.push_back(character->v);

        uvs.push_back(character->u);
        uvs.push_back(character->v_max);

        uvs.push_back(character->u_max);
        uvs.push_back(character->v_max);

        uvs.push_back(character->u_max);
        uvs.push_back(character->v);

        text_colors.push_back(color.x);
        text_colors.push_back(color.y);
        text_colors.push_back(color.z);
        text_colors.push_back(color.x);
        text_colors.push_back(color.y);
        text_colors.push_back(color.z);
        text_colors.push_back(color.x);
        text_colors.push_back(color.y);
        text_colors.push_back(color.z);
        text_colors.push_back(color.x);
        text_colors.push_back(color.y);
        text_colors.push_back(color.z);

        cursor_x += character->advance * scale;
    }

    std::vector<unsigned int> reversed_indices(indices.size());

    for(size_t i = 0; i != indices.size(); i += 6){
        reversed_indices[indices.size() - i - 6] = indices[i];
        reversed_indices[indices.size() - i - 5] = indices[i + 1];
        reversed_indices[indices.size() - i - 4] = indices[i + 2];
        reversed_indices[indices.size() - i - 3] = indices[i + 3];
        reversed_indices[indices.size() - i - 2] = indices[i + 4];
        reversed_indices[indices.size() - i - 1] = indices[i + 5];
    }

    TextModel generated_model;
    generated_model.load(&vertices[0], vertices.size(), &reversed_indices[0], reversed_indices.size(), &uvs[0], uvs.size(), &text_colors[0], text_colors.size());
    return generated_model;
}

TextModel Font::generateAdeptTextModel(const std::string& text, float scale, const SyntaxColorPalette& palette){
    float x = 0.0f, y = 0.0f;
    std::vector<unsigned int> indices;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<float> text_colors;
    FontCharacter* character;
    size_t index = 0;

    for(size_t i = 0; i != text.size(); i++){
        char ascii = text[i];

        // Newline if newline characater
        if(ascii == '\n'){
            x = 0.0f;
            y += line_height * scale;
            continue;
        }
        // Tab
        if(ascii == '\t'){
            // Find space character
            character = NULL;
            for(FontCharacter& font_char : characters){
                if(font_char.id == ' '){
                    character = &font_char;
                    break;
                }
            }

            if(character == NULL) continue;

            x += character->advance * 4;
            continue;
        }
        // Length-Strings
        if(ascii == '"'){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(text[scan_i] != '"' and scan_i + 1 != text.size()){
                    if(text[scan_i] == '\\' and scan_i + 2 != text.size()) scan_i++;
                    scan_i++;
                }

                this->highlightCharacters(text, i, scan_i - i + 1, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.string, indices, vertices, uvs, text_colors, index, x,  y, scale);
                i--; continue;
            }
        }
        // C-Strings
        if(ascii == '\''){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(text[scan_i] != '\'' and scan_i + 1 != text.size()){
                    if(text[scan_i] == '\\' and scan_i + 2 != text.size()) scan_i++;
                    scan_i++;
                }

                if(scan_i + 2 < text.size() && strncmp(&text[scan_i + 1], "ub", 2) == 0){
                    scan_i += 2;
                }

                this->highlightCharacters(text, i, scan_i - i + 1, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }
        // Annotations
        if(ascii == '#'){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(charisidentifier(text[scan_i]) or charisnumeric(text[scan_i])) if(++scan_i == text.size()) break;
                this->highlightCharacters(text, i, scan_i - i, palette.compile_time, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.compile_time, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }
        // Comments
        if(i + 1 < text.size()){
            if(ascii == '/' and text[i+1] == '/'){
                size_t scan_i = i + 2;

                if(scan_i != text.size()){
                    while(text[scan_i] != '\n' and scan_i + 1 != text.size()) scan_i++;
                    this->highlightCharacters(text, i, scan_i - i + 1, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                } else {
                    this->highlightCharacters(text, i, 2, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                }
            }
            if(ascii == '/' and text[i+1] == '*'){
                size_t scan_i = i + 3;
                
                if(scan_i + 1 < text.size()){
                    while(scan_i + 1 != text.size() && (text[scan_i - 1] != '*' || text[scan_i] != '/')) scan_i++;
                    this->highlightCharacters(text, i, scan_i - i + 1, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                } else {
                    this->highlightCharacters(text, i, text.size() - i, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                }
            }
        }
        // Numbers
        if(charisnumeric(ascii) and !(prevcharisnumeric() or prevcharisidentifier())){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                if(text[scan_i] == 'x'){ // Hex numbers
                    if(++scan_i != text.size()) while( (charisnumeric(text[scan_i]) or (text[scan_i]>='a' and text[scan_i]<='f') or (text[scan_i]>='A' and text[scan_i]<='F') )) if(++scan_i == text.size()) break;
                }
                else {// Regular numbers
                    while( (charisnumeric(text[scan_i]) or text[scan_i] == '.')) if(++scan_i == text.size()) break;
                }

                if(scan_i != text.size()){
                    if(text[scan_i] == 's'){
                        scan_i++;
                        if(scan_i != text.size()){
                            if(text[scan_i] == 'b') scan_i++;
                            else if(text[scan_i] == 's') scan_i++;
                            else if(text[scan_i] == 'i') scan_i++;
                            else if(text[scan_i] == 'l') scan_i++;
                        }
                    }
                    else if(text[scan_i] == 'u'){
                        scan_i++;
                        if(scan_i != text.size()){
                            if(text[scan_i] == 'b') scan_i++;
                            else if(text[scan_i] == 's') scan_i++;
                            else if(text[scan_i] == 'i') scan_i++;
                            else if(text[scan_i] == 'l') scan_i++;
                            else if(text[scan_i] == 'z') scan_i++;
                        }
                    }
                    else if(text[scan_i] == 'b'){scan_i++;}
                    else if(text[scan_i] == 'i'){scan_i++;}
                    else if(text[scan_i] == 'l'){scan_i++;}
                    else if(text[scan_i] == 'f'){scan_i++;}
                    else if(text[scan_i] == 'd'){scan_i++;}
                    else if(text[scan_i] == 'h'){scan_i++;}
                }

                this->highlightCharacters(text, i, scan_i - i, palette.number, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.number, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }

        #define HIGHLIGHT(a, b) this->highlightCharacters(text, i, b, a, indices, vertices, uvs, text_colors, index, x, y, scale);

        enum Relation {
            WHITESPACE, WHITESPACE_OR_OPERATOR
        };

        struct Special {
            std::string name;
            Relation prereleation, postrelation;
            const Vector3f *color;
        };

        // NOTE: Must be sorted by name length
        Special specials[] = {
            {"as", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"at", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"if", WHITESPACE, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"or", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"in", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"it", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"for", WHITESPACE, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"and", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"new", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"int", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"ptr", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"out", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"idx", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"def", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"Any", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"POD", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"func", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"void", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"null", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"cast", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"uint", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"byte", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"long", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"bool", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"else", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"case", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"true", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"this", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"each", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"enum", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"while", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"until", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"defer", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"ubyte", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"ulong", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"short", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"float", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"usize", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"false", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"alias", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"break", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"undef", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"inout", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"struct", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"public", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"import", WHITESPACE, WHITESPACE, &palette.keyword},
            {"return", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"switch", WHITESPACE, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"delete", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"sizeof", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"unless", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"ushort", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"String", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"double", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"pragma", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"repeat", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"static", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"AnyType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"private", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"foreign", WHITESPACE, WHITESPACE, &palette.keyword},
            {"dynamic", WHITESPACE, WHITESPACE, &palette.keyword},
            {"default", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"stdcall", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
            {"constant", WHITESPACE, WHITESPACE, &palette.keyword},
            {"continue", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"__types__", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.number},
            {"dangerous", WHITESPACE, WHITESPACE, &palette.keyword},
            {"AnyPtrType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"successful", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"AnyTypeKind", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},    
            {"AnyStructType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"AnyFuncPtrType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"__types_length__", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.number},
            {"AnyFixedArrayType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        };

        size_t specials_length = sizeof(specials) / sizeof(Special);
        size_t name_length_level = 0;
        bool previous_character_is_whitespace = prevchariswhitespace();
        bool previous_character_is_operator = previous_character_is_whitespace ? false : prevcharisoperator();
        bool next_character_is_whitespace = false, next_character_is_operator = false;

        for(size_t s = 0; s != specials_length; s++){
            Special *special = &specials[s];
            size_t name_length = special->name.length();

            if(name_length_level != name_length){
                // Update calculations
                if(i + name_length < text.size()){
                    next_character_is_whitespace = chariswhitespace(text[i + name_length]);
                    next_character_is_operator = next_character_is_whitespace ? false : charisoperator(text[i + name_length]);
                } else {
                    // Pretend the next character is there
                    next_character_is_operator = true;
                    next_character_is_whitespace = true;
                }

                name_length_level = name_length;
            }

            // Faster substring compare than using std::string::substr
            if(strncmp(&text[i], special->name.data(), name_length) == 0){
                // Determine whether or not we should highlight it
                bool should_highlight = (special->prereleation == WHITESPACE) ? (
                    (previous_character_is_whitespace)
                    && ((special->postrelation == WHITESPACE) ? (
                        next_character_is_whitespace
                    ) : (special->postrelation == WHITESPACE_OR_OPERATOR ? (
                        next_character_is_whitespace || next_character_is_operator
                    ) : false))
                ) : (special->prereleation == WHITESPACE_OR_OPERATOR ? (
                    (previous_character_is_whitespace || previous_character_is_operator)
                    && ((special->postrelation == WHITESPACE) ? (
                        next_character_is_whitespace
                    ) : (special->postrelation == WHITESPACE_OR_OPERATOR ? (
                        next_character_is_whitespace || next_character_is_operator
                    ) : false))
                ) : false);

                if(should_highlight){
                    if(special->name == "pragma"){
                        // Do special stuff for pragma
                        while(i + name_length < text.size() && chariswhitespace(text[i + name_length])) name_length++;
                        while(i + name_length < text.size() && charisidentifier(text[i + name_length])) name_length++;
                    }

                    HIGHLIGHT(*special->color, name_length);
                    i--;
                    goto hard_continue;
                }
            }
        }

        #undef HIGHLIGHT

        // Find the character data for the character
        // TODO: Make this lookup faster (maybe use table?)
        character = NULL;
        for(FontCharacter& font_char : characters){
            if(font_char.id == ascii){
                character = &font_char;
                break;
            }
        }

        // Don't render anything if the character wasn't found
        if(character == NULL) continue;

        vertices.push_back(x + character->x_offset * scale);
        vertices.push_back(y + character->y_offset * scale);

        vertices.push_back(x + character->x_offset * scale);
        vertices.push_back(y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(y + character->y_offset * scale);

        indices.push_back(index);   indices.push_back(index+1);
        indices.push_back(index+2); indices.push_back(index+2);
        indices.push_back(index+3); indices.push_back(index);
        index += 4;

        uvs.push_back(character->u);     uvs.push_back(character->v);
        uvs.push_back(character->u);     uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v);

        if(charisoperator(ascii)){
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
        }
        else {
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
        }


        x += character->advance * scale;

hard_continue:
        continue;    
    }

    std::vector<unsigned int> reversed_indices(indices.size());

    for(size_t i = 0; i != indices.size(); i += 6){
        reversed_indices[indices.size() - i - 6] = indices[i];
        reversed_indices[indices.size() - i - 5] = indices[i + 1];
        reversed_indices[indices.size() - i - 4] = indices[i + 2];
        reversed_indices[indices.size() - i - 3] = indices[i + 3];
        reversed_indices[indices.size() - i - 2] = indices[i + 4];
        reversed_indices[indices.size() - i - 1] = indices[i + 5];
    }

    TextModel generated_model;
    generated_model.load(&vertices[0], vertices.size(), &reversed_indices[0], reversed_indices.size(), &uvs[0], uvs.size(), &text_colors[0], text_colors.size());
    return generated_model;
}

TextModel Font::generateJavaTextModel(const std::string& text, float scale, const SyntaxColorPalette& palette){
    float x = 0.0f, y = 0.0f;
    std::vector<unsigned int> indices;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<float> text_colors;
    FontCharacter* character;
    size_t index = 0;

    for(size_t i = 0; i != text.size(); i++){
        char ascii = text[i];

        // Newline if newline characater
        if(ascii == '\n'){
            x = 0.0f;
            y += line_height * scale;
            continue;
        }
        // Tab
        if(ascii == '\t'){
            // Find space character
            character = NULL;
            for(FontCharacter& font_char : characters){
                if(font_char.id == ' '){
                    character = &font_char;
                    break;
                }
            }

            if(character == NULL) continue;

            x += character->advance * 4;
            continue;
        }
        // Length-Strings
        if(ascii == '"'){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(text[scan_i] != '"' and scan_i + 1 != text.size()){
                    if(text[scan_i] == '\\' and scan_i + 2 != text.size()) scan_i++;
                    scan_i++;
                }

                this->highlightCharacters(text, i, scan_i - i + 1, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.string, indices, vertices, uvs, text_colors, index, x,  y, scale);
                i--; continue;
            }
        }
        // C-Strings
        if(ascii == '\''){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(text[scan_i] != '\'' and scan_i + 1 != text.size()){
                    if(text[scan_i] == '\\' and scan_i + 2 != text.size()) scan_i++;
                    scan_i++;
                }

                this->highlightCharacters(text, i, scan_i - i + 1, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }
        // Annotations
        if(ascii == '@'){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(charisidentifier(text[scan_i]) or charisnumeric(text[scan_i])) if(++scan_i == text.size()) break;
                this->highlightCharacters(text, i, scan_i - i, palette.compile_time, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.compile_time, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }
        // Comments
        if(i + 1 < text.size()){
            if(ascii == '/' and text[i+1] == '/'){
                size_t scan_i = i + 2;

                if(scan_i != text.size()){
                    while(text[scan_i] != '\n' and scan_i + 1 != text.size()) scan_i++;
                    this->highlightCharacters(text, i, scan_i - i + 1, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                } else {
                    this->highlightCharacters(text, i, 2, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                }
            }
            if(ascii == '/' and text[i+1] == '*'){
                size_t scan_i = i + 3;

                if(scan_i + 1 < text.size()){
                    while(scan_i + 1 != text.size() && (text[scan_i - 1] != '*' || text[scan_i] != '/')) scan_i++;
                    this->highlightCharacters(text, i, scan_i - i + 1, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                } else {
                    this->highlightCharacters(text, i, text.size() - i, palette.comment, indices, vertices, uvs, text_colors, index, x, y, scale);
                    i--; continue;
                }
            }
        }
        // Numbers
        if(charisnumeric(ascii) and !(prevcharisnumeric() or prevcharisidentifier())){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                if(text[scan_i] == 'x'){ // Hex numbers
                    if(++scan_i != text.size()) while( (charisnumeric(text[scan_i]) or (text[scan_i]>='a' and text[scan_i]<='f') or (text[scan_i]>='A' and text[scan_i]<='F') )) if(++scan_i == text.size()) break;
                }
                else {// Regular numbers
                    while( (charisnumeric(text[scan_i]) or text[scan_i] == '.')) if(++scan_i == text.size()) break;
                }

                if(scan_i != text.size()){
                    if(text[scan_i] == 'd')      scan_i++;
                    else if(text[scan_i] == 'f') scan_i++;
                    else if(text[scan_i] == 'l') scan_i++;
                    else if(text[scan_i] == 'L') scan_i++;
                }

                this->highlightCharacters(text, i, scan_i - i, palette.number, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.number, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }

        #define HIGHLIGHT(a, b) this->highlightCharacters(text, i, b, a, indices, vertices, uvs, text_colors, index, x, y, scale);

        enum Relation {
            WHITESPACE, WHITESPACE_OR_OPERATOR
        };

        struct Special {
            std::string name;
            Relation prereleation, postrelation;
            const Vector3f *color;
        };

        // NOTE: Must be sorted by name length
        Special specials[] = {
            {"do", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"if", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"for", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"int", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"new", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"try", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"File", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"Math", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"byte", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"case", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"char", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"else", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"enum", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"goto", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"long", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"null", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"this", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"true", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"void", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"break", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"catch", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"class", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"const", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"false", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"final", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"float", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"short", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"super", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"while", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"Object", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"Random", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"System", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"String", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"assert", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"double", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"import", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"native", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"public", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"return", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"static", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"switch", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"throws", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"Console", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"Scanner", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"boolean", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"default", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"extends", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"finally", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"package", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"private", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"abstract", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"continue", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"strictfp", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"volatile", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"Exception", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"interface", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"protected", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"transient", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"BigDecimal", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"BigInteger", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"implements", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"instanceof", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"IOException", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"InputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"MathContext", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"EOFException", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"OutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"Serializable", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"StringBuffer", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"synchronized", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
            {"StringBuilder", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"BufferedReader", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"BufferedWriter", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"FileDescriptor", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"CharArrayReader", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"CharArrayWriter", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"DataInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"FileInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"DataOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"FileOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"BufferedInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"BufferedOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"ByteArrayInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
            {"ByteArrayOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        };

        size_t specials_length = sizeof(specials) / sizeof(Special);
        size_t name_length_level = 0;
        bool previous_character_is_whitespace = prevchariswhitespace();
        bool previous_character_is_operator = previous_character_is_whitespace ? false : prevcharisoperator();
        bool next_character_is_whitespace = false, next_character_is_operator = false;

        for(size_t s = 0; s != specials_length; s++){
            Special *special = &specials[s];
            size_t name_length = special->name.length();

            if(name_length_level != name_length){
                // Update calculations
                if(i + name_length < text.size()){
                    next_character_is_whitespace = chariswhitespace(text[i + name_length]);
                    next_character_is_operator = next_character_is_whitespace ? false : charisoperator(text[i + name_length]);
                } else {
                    // Pretend the next character is there
                    next_character_is_operator = true;
                    next_character_is_whitespace = true;
                }

                name_length_level = name_length;
            }

            // Faster substring compare than using std::string::substr
            if(strncmp(&text[i], special->name.data(), name_length) == 0){
                // Determine whether or not we should highlight it
                bool should_highlight = (special->prereleation == WHITESPACE) ? (
                    (previous_character_is_whitespace)
                    && ((special->postrelation == WHITESPACE) ? (
                        next_character_is_whitespace
                    ) : (special->postrelation == WHITESPACE_OR_OPERATOR ? (
                        next_character_is_whitespace || next_character_is_operator
                    ) : false))
                ) : (special->prereleation == WHITESPACE_OR_OPERATOR ? (
                    (previous_character_is_whitespace || previous_character_is_operator)
                    && ((special->postrelation == WHITESPACE) ? (
                        next_character_is_whitespace
                    ) : (special->postrelation == WHITESPACE_OR_OPERATOR ? (
                        next_character_is_whitespace || next_character_is_operator
                    ) : false))
                ) : false);

                if(should_highlight){
                    HIGHLIGHT(*special->color, name_length);
                    i--;
                    goto hard_continue;
                }
            }
        }

        #undef HIGHLIGHT

        // Find the character data for the character
        // TODO: Make this lookup faster (maybe use table?)
        character = NULL;
        for(FontCharacter& font_char : characters){
            if(font_char.id == ascii){
                character = &font_char;
                break;
            }
        }

        // Don't render anything if the character wasn't found
        if(character == NULL) continue;

        vertices.push_back(x + character->x_offset * scale);
        vertices.push_back(y + character->y_offset * scale);

        vertices.push_back(x + character->x_offset * scale);
        vertices.push_back(y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(y + character->y_offset * scale);

        indices.push_back(index);   indices.push_back(index+1);
        indices.push_back(index+2); indices.push_back(index+2);
        indices.push_back(index+3); indices.push_back(index);
        index += 4;

        uvs.push_back(character->u);     uvs.push_back(character->v);
        uvs.push_back(character->u);     uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v);

        if(charisoperator(ascii)){
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
            text_colors.push_back(palette.operation.x); text_colors.push_back(palette.operation.y); text_colors.push_back(palette.operation.z);
        }
        else {
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
        }


        x += character->advance * scale;

hard_continue:
        continue;    
    }

    std::vector<unsigned int> reversed_indices(indices.size());

    for(size_t i = 0; i != indices.size(); i += 6){
        reversed_indices[indices.size() - i - 6] = indices[i];
        reversed_indices[indices.size() - i - 5] = indices[i + 1];
        reversed_indices[indices.size() - i - 4] = indices[i + 2];
        reversed_indices[indices.size() - i - 3] = indices[i + 3];
        reversed_indices[indices.size() - i - 2] = indices[i + 4];
        reversed_indices[indices.size() - i - 1] = indices[i + 5];
    }

    TextModel generated_model;
    generated_model.load(&vertices[0], vertices.size(), &reversed_indices[0], reversed_indices.size(), &uvs[0], uvs.size(), &text_colors[0], text_colors.size());
    return generated_model;
}

TextModel Font::generateHtmlTextModel(const std::string& text, float scale, const SyntaxColorPalette& palette){
    float x = 0.0f, y = 0.0f;
    std::vector<unsigned int> indices;
    std::vector<float> vertices;
    std::vector<float> uvs;
    std::vector<float> text_colors;
    FontCharacter* character;
    size_t index = 0;

    for(size_t i = 0; i != text.size(); i++){
        char ascii = text[i];

        // Newline if newline characater
        if(ascii == '\n'){
            x = 0.0f;
            y += line_height * scale;
            continue;
        }
        // Tab
        if(ascii == '\t'){
            // Find space character
            character = NULL;
            for(FontCharacter& font_char : characters){
                if(font_char.id == ' '){
                    character = &font_char;
                    break;
                }
            }

            if(character == NULL) continue;

            x += character->advance * 4;
            continue;
        }
        // Length-Strings
        if(ascii == '"'){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(text[scan_i] != '"' and scan_i + 1 != text.size()){
                    if(text[scan_i] == '\\' and scan_i + 2 != text.size()) scan_i++;
                    scan_i++;
                }

                this->highlightCharacters(text, i, scan_i - i + 1, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.string, indices, vertices, uvs, text_colors, index, x,  y, scale);
                i--; continue;
            }
        }
        // C-Strings
        if(ascii == '\''){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(text[scan_i] != '\'' and scan_i + 1 != text.size()){
                    if(text[scan_i] == '\\' and scan_i + 2 != text.size()) scan_i++;
                    scan_i++;
                }

                this->highlightCharacters(text, i, scan_i - i + 1, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.string, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }
        // Tags
        if(ascii == '<' || ascii == '/'){
            size_t scan_i = i + 1;

            if(scan_i != text.size()){
                while(charisidentifier(text[scan_i]) or charisnumeric(text[scan_i])) if(++scan_i == text.size()) break;
                this->highlightCharacters(text, i, scan_i - i, palette.keyword, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            } else {
                this->highlightCharacters(text, i, text.size() - i, palette.keyword, indices, vertices, uvs, text_colors, index, x, y, scale);
                i--; continue;
            }
        }

        // Find the character data for the character
        // TODO: Make this lookup faster (maybe use table?)
        character = NULL;
        for(FontCharacter& font_char : characters){
            if(font_char.id == ascii){
                character = &font_char;
                break;
            }
        }

        // Don't render anything if the character wasn't found
        if(character == NULL) continue;

        vertices.push_back(x + character->x_offset * scale);
        vertices.push_back(y + character->y_offset * scale);

        vertices.push_back(x + character->x_offset * scale);
        vertices.push_back(y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(y + character->size_y * scale + character->y_offset * scale);

        vertices.push_back(x + character->size_x * scale + character->x_offset * scale);
        vertices.push_back(y + character->y_offset * scale);

        indices.push_back(index);   indices.push_back(index+1);
        indices.push_back(index+2); indices.push_back(index+2);
        indices.push_back(index+3); indices.push_back(index);
        index += 4;

        uvs.push_back(character->u);     uvs.push_back(character->v);
        uvs.push_back(character->u);     uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v);

        if(ascii == '>'){
            text_colors.push_back(palette.keyword.x); text_colors.push_back(palette.keyword.y); text_colors.push_back(palette.keyword.z);
            text_colors.push_back(palette.keyword.x); text_colors.push_back(palette.keyword.y); text_colors.push_back(palette.keyword.z);
            text_colors.push_back(palette.keyword.x); text_colors.push_back(palette.keyword.y); text_colors.push_back(palette.keyword.z);
            text_colors.push_back(palette.keyword.x); text_colors.push_back(palette.keyword.y); text_colors.push_back(palette.keyword.z);
        } else {
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
            text_colors.push_back(palette.plain.x); text_colors.push_back(palette.plain.y); text_colors.push_back(palette.plain.z);
        }

        x += character->advance * scale;
    }

    std::vector<unsigned int> reversed_indices(indices.size());

    for(size_t i = 0; i != indices.size(); i += 6){
        reversed_indices[indices.size() - i - 6] = indices[i];
        reversed_indices[indices.size() - i - 5] = indices[i + 1];
        reversed_indices[indices.size() - i - 4] = indices[i + 2];
        reversed_indices[indices.size() - i - 3] = indices[i + 3];
        reversed_indices[indices.size() - i - 2] = indices[i + 4];
        reversed_indices[indices.size() - i - 1] = indices[i + 5];
    }

    TextModel generated_model;
    generated_model.load(&vertices[0], vertices.size(), &reversed_indices[0], reversed_indices.size(), &uvs[0], uvs.size(), &text_colors[0], text_colors.size());
    return generated_model;
}

void Font::print(){
    for(const AttributeEntry& attr : attributes){
        std::cout << attr.name << " ";
        for(const AttributeEntry::SubAttribute& sub : attr.sub_attributes){
            std::cout << sub.name << "=" << sub.value << " ";
        }
        std::cout << std::endl;
    }
}

void Font::highlightCharacters(const std::string& text, size_t& i, size_t length, const Vector3f& color,
                               std::vector<unsigned int>& indices, std::vector<float>& vertices, std::vector<float>& uvs, std::vector<float>& text_colors,
                               size_t& index, float& cursor_x, float& cursor_y, float scale){
    FontCharacter* character;
    char ascii = text[i];

    float r = color.x;
    float g = color.y;
    float b = color.z;

    for(size_t inner_character_i = 0; inner_character_i != length; inner_character_i++){
        // Newline if newline characater
        if(ascii == '\n'){
            cursor_x = 0.0f;
            cursor_y += line_height * scale;
            i++; ascii = text[i];
            continue;
        }
        if(ascii == '\t'){
            // Find space character
            character = NULL;
            for(FontCharacter& font_char : characters){
                if(font_char.id == ' '){
                    character = &font_char;
                    break;
                }
            }

            if(character == NULL) { i++; ascii = text[i]; continue; }

            cursor_x += character->advance * 4;
            i++; ascii = text[i];
            continue;
        }

        // Find the character data for the character
        character = NULL;
        for(FontCharacter& font_char : characters){
            if(font_char.id == ascii){
                character = &font_char;
                break;
            }
        }

        // Don't render anything if the character wasn't found
        if(character == NULL) { i++; ascii = text[i]; continue; }

        vertices.push_back(cursor_x + character->x_offset * scale); vertices.push_back(cursor_y + character->y_offset * scale);
        vertices.push_back(cursor_x + character->x_offset * scale); vertices.push_back(cursor_y + character->size_y * scale + character->y_offset * scale);
        vertices.push_back(cursor_x + character->size_x * scale + character->x_offset * scale); vertices.push_back(cursor_y + character->size_y * scale + character->y_offset * scale);
        vertices.push_back(cursor_x + character->size_x * scale + character->x_offset * scale); vertices.push_back(cursor_y + character->y_offset * scale);

        indices.push_back(index); indices.push_back(index+1); indices.push_back(index+2);
        indices.push_back(index+2); indices.push_back(index+3); indices.push_back(index);
        index += 4;

        uvs.push_back(character->u); uvs.push_back(character->v);
        uvs.push_back(character->u); uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v_max);
        uvs.push_back(character->u_max); uvs.push_back(character->v);

        text_colors.push_back(r); text_colors.push_back(g); text_colors.push_back(b);
        text_colors.push_back(r); text_colors.push_back(g); text_colors.push_back(b);
        text_colors.push_back(r); text_colors.push_back(g); text_colors.push_back(b);
        text_colors.push_back(r); text_colors.push_back(g); text_colors.push_back(b);

        i++; ascii = text[i];
        cursor_x += character->advance * scale;
    }
}
