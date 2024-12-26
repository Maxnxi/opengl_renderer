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

const TGAColor white = TGAColor(255, 255,  255, 255);
const TGAColor red   = TGAColor(255, 0,    0,   255);
const TGAColor green = TGAColor(0,   255,  0,   255);
Model *model = NULL;
const int width = 800;
const int height = 800;
/*
https://github.com/ssloy/tinyrenderer/blob/d0703acf18c48f2f7d00e552697d4797e0669ade/main.cpp
*/

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0.x, p1.x);
        std::swap(p0.y, p1.y);
    }

    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    line(t0, t1, image, color);
    line(t1, t2, image, color);
    line(t2, t0, image, color);
}



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
    
    

	//  if (2==argc) {
    //     model = new Model(argv[1]);
    // } else {
    //     model = new Model("obj/african_head.obj");
    // }

    // TGAImage image(width, height, TGAImage::RGB);
    // for (int i=0; i<model->nfaces(); i++) {
    //     std::vector<int> face = model->face(i);
    //     for (int j=0; j<3; j++) {
    //         Vec3f v0 = model->vert(face[j]);
    //         Vec3f v1 = model->vert(face[(j+1)%3]);
    //         int x0 = (v0.x+1.)*width/2.;
    //         int y0 = (v0.y+1.)*height/2.;
    //         int x1 = (v1.x+1.)*width/2.;
    //         int y1 = (v1.y+1.)*height/2.;
    //         line(x0, y0, x1, y1, image, white);
    //     }
    // }

    TGAImage image(width, height, TGAImage::RGB);

    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);

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