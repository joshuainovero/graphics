#ifndef SHADER_H
#define SHADER_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>
#include <utility>
#include <fstream>
#include <sstream>

std::pair<std::string, std::string> parseShaders(const std::string& vsFile, const std::string& fsFile){
    std::stringstream ssVS, ssFS;
    std::string line;
    std::ifstream vsShaderStream(vsFile);
        while (std::getline(vsShaderStream, line))
            ssVS << line << "\n";
    vsShaderStream.close();
    std::ifstream fsShaderStream(fsFile);
        while(std::getline(fsShaderStream, line))
            ssFS << line << "\n";
    fsShaderStream.close();

    return std::make_pair(ssVS.str(), ssFS.str());
}

GLuint LoadShader(const std::string& vsSource, const std::string& fsSource){
    GLuint shaderProgram = glCreateProgram();
    
    // Create vertex shader
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    const char* charVS = vsSource.c_str();
    glShaderSource(vertexShaderID, 1, &charVS, nullptr);
    glCompileShader(vertexShaderID);

    // Check errors
    GLint vsShaderResult, vsShaderLength;
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &vsShaderResult);
    if (vsShaderResult == GL_FALSE){
        glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &vsShaderLength);
        char* message = (char*)alloca(vsShaderLength * sizeof(char));
        glGetShaderInfoLog(vertexShaderID, vsShaderLength, &vsShaderLength, message);
        printf("Failed to compile vertex shader!\n%c", message);
        glDeleteShader(vertexShaderID);
    }

    // Create fragment shader
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    const char* charFS = fsSource.c_str();
    glShaderSource(fragmentShaderID, 1, &charFS, nullptr);
    glCompileShader(fragmentShaderID);

    // Check errors
    GLint fsShaderResult, fsShaderLength;
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &fsShaderResult);
    if (fsShaderResult == GL_FALSE){
        glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &fsShaderLength);
        char* message2 = (char*)alloca(fsShaderLength * sizeof(char));
        glGetShaderInfoLog(fragmentShaderID, fsShaderLength, &fsShaderLength, message2);
        printf("Failed to compile fragment shader!\n%c", message2);
        glDeleteShader(fragmentShaderID);
    }
    
    // Link program
    glAttachShader(shaderProgram, vertexShaderID);
    glAttachShader(shaderProgram, fragmentShaderID);
    glLinkProgram(shaderProgram);
    glValidateProgram(shaderProgram);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return shaderProgram;
}


#endif // SHADER_H