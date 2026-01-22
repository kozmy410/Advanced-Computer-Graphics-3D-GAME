#pragma once
#include "dependencies/glew-2.2.0/include/GL/glew.h"
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
    Diagnostics() {}
};