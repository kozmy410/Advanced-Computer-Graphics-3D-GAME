#pragma once

// We need the GLEW definitions for OpenGL functions.
#include "dependencies/glew-2.2.0/include/GL/glew.h"

// We need iostream to print to the console.
#include <iostream>

class Diagnostics {
public:
    /**
     * @brief Queries and prints a detailed report of the OpenGL context,
     *        including driver version, hardware vendor, and implementation limits.
     *        This is very useful for diagnostics.
     */
    static void printOpenGLInfo();

private:
    // A private constructor prevents anyone from creating an instance of this class.
    // This enforces its use as a static utility class.
    Diagnostics() {}
};