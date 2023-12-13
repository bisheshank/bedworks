#pragma once

#include"utils/debug.h"
#include"utils/shaderloader.h"
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>

class Shader
{
public:
    // Reference ID of the Shader Program
    GLuint ID;
    // Default constructor
    Shader();
    // Constructor that build the Shader Program from 2 different shaders
    Shader(const char* vertexFile, const char* fragmentFile);

    // Loads shader data into a shader object
    void loadData(const char* vertexFile, const char* fragmentFile);

    // Activates the Shader Program
    void Activate();
    // Deactivates the Shader Program
    void Deactivate();
    // Deletes the Shader Program
    void Delete();
};
