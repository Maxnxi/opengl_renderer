#include "tgaimage.h"
#include <iostream>
#include <filesystem>
#include <unistd.h> 

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

// int main(int argc, char** argv) {
// 	TGAImage image(100, 100, TGAImage::RGB);
// 	image.set(52, 41, red);
// 	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
// 	image.write_tga_file("output.tga");
// 	return 0;
// }

int main(int argc, char** argv) {
	    // Get the current executable path
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    if (len != -1) {
        buffer[len] = '\0';
        // Extract directory path
        for(int i = len; i >= 0; i--) {
            if(buffer[i] == '/') {
                buffer[i] = '\0';
                break;
            }
        }
        // Change working directory to executable location
        chdir(buffer);
    }
    
    std::cout << "Current working directory: " << getcwd(nullptr, 0) << std::endl;
    
    std::cout << "Creating image..." << std::endl;
    
    TGAImage image(100, 100, TGAImage::RGB);
    image.set(52, 41, red);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    
    std::cout << "Attempting to write output.tga..." << std::endl;
    bool success = image.write_tga_file("output.tga");
    
    if (success) {
        std::cout << "Successfully wrote output.tga" << std::endl;
    } else {
        std::cout << "Failed to write output.tga" << std::endl;
    }
    
    return 0;
}