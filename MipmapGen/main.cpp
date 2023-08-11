#include <iostream>
#include "lodepng/lodepng.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "usage: [file] [mip level].";
    }
    
    std::string fileName = argv[1];
    std::vector<unsigned char> buffer;
    lodepng::load_file(buffer, fileName);
}