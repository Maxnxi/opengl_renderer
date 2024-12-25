#include <iostream>
#include <filesystem>
#include <unistd.h> 

#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
//#include <gperftools/profiler.h>

/*
this ia based on materials from 
https://habr.com/ru/articles/248153/
https://github.com/ssloy/tinyrenderer/wiki
*/

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width = 800;
const int height = 800;
/*
https://github.com/ssloy/tinyrenderer/blob/d0703acf18c48f2f7d00e552697d4797e0669ade/main.cpp
*/
// version #1
// void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
// 	for (float t=0.;t<1.;t+=.01) {
// 		int x = x0*(1.-t) + x1*t;
// 		int y = y0*(1.-t) + y1*t;
// 		image.set(x,y,color);
// 	}
// }

// version #2
// void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
// 	bool steep = false;
// 	if (std::abs(x0-x1)<std::abs(y0-y1)) {
// 		std::swap(x0, y0);
// 		std::swap(x1, y1);
// 		steep = true;
// 	}
// 	if (x0>x1) {
// 		std::swap(x0,x1);
// 		std::swap(y0,y1);
// 	}
// 	int dx = x1-x0;
// 	int dy = y1-y0;
// 	float derror = std::abs(dy/float(dx));
// 	float error = 0;
// 	int y = y0;
// 	for (int x=x0; x<=x1; x++) {
// 		if (steep) {
// 			image.set(y, x, color);
// 		} else {
// 			image.set(x, y, color);
// 		}
// 		error += derror;

// 		if (error>.5) {
// 			y += (y1>y0?1:-1);
// 			error -= 1.;
// 		}
// 	}
// }
// version #3
// void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
//     bool steep = false;
//     if (std::abs(x0-x1)<std::abs(y0-y1)) {
//         std::swap(x0, y0);
//         std::swap(x1, y1);
//         steep = true;
//     }
//     if (x0>x1) {
//         std::swap(x0, x1);
//         std::swap(y0, y1);
//     }
//     int dx = x1-x0;
//     int dy = y1-y0;
//     int derror2 = std::abs(dy)*2;
//     int error2 = 0;
//     int y = y0;
//     for (int x=x0; x<=x1; x++) {
//         if (steep) {
//             image.set(y, x, color);
//         } else {
//             image.set(x, y, color);
//         }
//         error2 += derror2;

//         if (error2 > dx) {
//             y += (y1>y0?1:-1);
//             error2 -= dx*2;
//         }
//     }
// }

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

// initial version
// int main(int argc, char** argv) {
// 	TGAImage image(100, 100, TGAImage::RGB);
// 	image.set(52, 41, red);
// 	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
// 	image.write_tga_file("output.tga");
// 	return 0;
// }

// same as initial version
// add path to save output.tga
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
    
    // TGAImage image(100, 100, TGAImage::RGB);
    // image.set(52, 41, red);

	// line(10,10,80,10,image,red);
	// line(10,10,80,80, image, red);
	// line(10,80,80,80,image,red);

	 if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        for (int j=0; j<3; j++) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            int x0 = (v0.x+1.)*width/2.;
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
            int y1 = (v1.y+1.)*height/2.;
            line(x0, y0, x1, y1, image, white);
        }
    }

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