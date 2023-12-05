#pragma once
#include <GL/glew.h>
#include <iostream>

namespace Debug
{
// Task 2: Add file name and line number parameters
inline void glErrorCheck(const char* file, int line) {
// Only executes in debug mode
#ifdef QT_DEBUG
    GLenum errorNumber = glGetError();
    while (errorNumber != GL_NO_ERROR) {
        // Task 2: Edit this print statement to be more descriptive
        // Done
        // TODO Add a conditional that checks for types of error number enums
        std::string error;
        switch(errorNumber) {
        case(0x0500):
            error.assign("GL_INVALID_ENUM");
            break;
        case(0x0501):
            error.assign("GL_INVALID_VALUE");
            break;
        case(0x0502):
            error.assign("GL_INVALID_OPERATION");
            break;
        case(0x0503):
            error.assign("GL_STACK_OVERFLOW");
            break;
        case(0x0504):
            error.assign("GL_STACK_UNDERFLOW");
            break;
        case(0x0505):
            error.assign("GL_OUT_OF_MEMORY");
            break;
        case(0x0506):
            error.assign("GL_INVALID_FRAMEBUFFER_OPERATION");
            break;
        case(0x0507):
            error.assign("GL_CONTEXT_LOST");
            break;
        case(0x8031):
            error.assign("GL_TABLE_TOO_LARGE");
        }

        std::cout << error.c_str() << " error occurred on line " << line << ", file " << file << std::endl;

        errorNumber = glGetError();
    }
#endif
}
}

// TASK 3: Add a preprocessor directive to automate the writing of this function
// Note that the preprocessor directive (is this different from a macro) must come after declaration
#define glErrorCheck() glErrorCheck(__FILE__, __LINE__)
