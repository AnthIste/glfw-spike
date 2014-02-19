#include "shader_utils.h"

#include <iostream>
#include <fstream>
#include <string>

// Adapted from http://stackoverflow.com/q/2602013/761648
std::string load_shader_from_file(std::string filename)
{
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);

    if (in)
    {
        std::string contents;

        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());

        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());

        in.close();

        return contents;
    }

    std::cerr << "Could not open " << filename << std::endl;

    return "";
}
