
#include <fstream>
#include <cassert>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string.h>

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
