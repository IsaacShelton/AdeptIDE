
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "OPENGL/Shader.h"

Shader::Shader(const std::string& vertexFile, const std::string& fragmentFile, const std::vector<std::string>& attributes){
    std::string vertexCode, fragmentCode;
    std::stringstream buffer;

    std::ifstream vertexFileStream(vertexFile);
    buffer << vertexFileStream.rdbuf();
    vertexCode = buffer.str();
    vertexFileStream.close();

    buffer.str("");

    std::ifstream fragmentFileStream(fragmentFile);
    buffer << fragmentFileStream.rdbuf();
    fragmentCode = buffer.str();
    fragmentFileStream.close();

    char *logMessage;
    GLsizei logLength;
    GLint status;

    this->vertex = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexCodePointer = vertexCode.c_str();
    glShaderSource(this->vertex, 1, &vertexCodePointer, NULL);
    glCompileShader(this->vertex);
    glGetShaderiv(this->vertex, GL_COMPILE_STATUS, &status);

    if(status == 0){
        std::cerr << "Failed to compile vertex shader '" << vertexFile << "'" << std::endl;
        logMessage = static_cast<char*>(malloc(1024));
        glGetShaderInfoLog(this->vertex, 1023, &logLength, logMessage);
        if(logLength != 0) std::cerr << logMessage << std::endl;
        free(logMessage);
    }

    this->fragment = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentCodePointer = fragmentCode.c_str();
    glShaderSource(this->fragment, 1, &fragmentCodePointer, NULL);
    glCompileShader(this->fragment);
    glGetShaderiv(this->fragment, GL_COMPILE_STATUS, &status);

    if(status == 0){
        std::cerr << "Failed to compile fragment shader '" << fragmentFile << "'" << std::endl;
        logMessage = static_cast<char*>(malloc(1024));
        glGetShaderInfoLog(this->fragment, 1023, &logLength, logMessage);
        if(logLength != 0) std::cerr << logMessage << std::endl;
        free(logMessage);
    }

    this->program = glCreateProgram();
    glAttachShader(this->program, this->vertex);
    glAttachShader(this->program, this->fragment);

    for(size_t i = 0; i != attributes.size(); i++){
        glBindAttribLocation(this->program, i, attributes[i].c_str());
    }

    glLinkProgram(this->program);
    glValidateProgram(this->program);
}

Shader::~Shader(){
    glDetachShader(this->program, this->vertex);
    glDetachShader(this->program, this->fragment);
    glDeleteShader(this->vertex);
    glDeleteShader(this->fragment);
    glDeleteProgram(this->program);
}

void Shader::bind(){
    glUseProgram(this->program);
}

void Shader::giveFloat(const std::string& uniform, float value){
    glUniform1f(this->getUniformLocation(uniform), value);
}

void Shader::giveVector3f(const std::string& uniform, const Vector3f& value){
    glUniform3f(this->getUniformLocation(uniform), value.x, value.y, value.z);
}

void Shader::giveVector4f(const std::string& uniform, const Vector4f& value){
    glUniform4f(this->getUniformLocation(uniform), value.x, value.y, value.z, value.w);
}

void Shader::giveMatrix4f(const std::string& uniform, const Matrix4f& value){
    glUniformMatrix4fv(this->getUniformLocation(uniform), 1, false, value.array);
}

GLint Shader::getUniformLocation(const std::string& name){
    return glGetUniformLocation(this->program, name.c_str());
}
