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
const TGAColor blue  = TGAColor(0,   0,    255, 255);

Model *model = NULL;
const int width = 800;
const int height = 800;
/*
https://github.com/ssloy/tinyrenderer/blob/d0703acf18c48f2f7d00e552697d4797e0669ade/main.cpp
*/

Vec3f light_dir(1,1,1);
// Vec3f       eye(1,1,3);
// Vec3f    center(0,0,0);
// Vec3f        up(0,1,1);

// struct Shader : public IShader {
//     mat<2,3,float> varying_uv;
//     mat<4,3,float> varying_tri;

//     virtual Vec4f vertex(int iface, int nthvert) {
//         varying_uv.set_col(nthvert, model->uv(iface, nthvert));
//         Vec4f gl_Vertex = Projection*ModelView*embed<4>(model->vert(iface,nthvert));
//         varying_tri.set_col(nthvert, gl_Vertex);
//         return gl_Vertex;
//     }

//     virtual bool fragment(Vec3f bar, TGAColor &color) {
//         Vec2f uv = varying_uv*bar;
//         color=model->diffuse(uv);
//         return false;
//     }
// };

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }

    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t + .5;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    if (t0.y==t1.y && t0.y==t2.y) return;
    // sort the vertices
    if (t0.y>t1.y) std::swap(t0, t1);
    if (t0.y>t2.y) std::swap(t0, t2);
    if (t1.y>t2.y) std::swap(t1, t2);

    int total_height = t2.y-t0.y;

    for (int i=0; i<total_height; i++) {
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        float alpha = (float)i/total_height;
        float beta = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height;
        Vec2i A =               t0 + (t2-t0)*alpha;
        Vec2i B = second_half ? t1 + (t2-t1)*beta : t0 + (t1-t0)*beta;
        if (A.x>B.x) std::swap(A,B);
        for (int j=A.x; j<=B.x; j++) {
            image.set(j, t0.y+i, color);
        }
    }
}

void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[]) {
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }
    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        if (ybuffer[x]<y) {
            ybuffer[x] = y;
            image.set(x, 0, color);
        }
    }
}



int main(int argc, char** argv) {
     { // just dumping the 2d scene (yay we have enough dimensions!)
        TGAImage scene(width, height, TGAImage::RGB);

        // scene "2d mesh"
        line(Vec2i(20, 34),   Vec2i(744, 400), scene, red);
        line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
        line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

        // screen line
        line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

        scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        scene.write_tga_file("scene.tga");
    }

    {
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
    
    // if (2==argc) {
    //     model = new Model(argv[1]);
    // } else {
    //     model = new Model("obj/african_head.obj");
    // }

    TGAImage render(width, height, TGAImage::RGB);
    int ybuffer[width];
    for (int i=0;i<width; i++) {
        ybuffer[i] = std::numeric_limits<int>::min();
    }
    rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
    rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
    rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);


    render.flip_vertically();
    render.write_tga_file("framebuffer.tga");

    std::cout << "Attempting to write output.tga..." << std::endl;
    bool success = render.write_tga_file("output.tga");
    
    if (success) {
        std::cout << "Successfully wrote output.tga" << std::endl;
    } else {
        std::cout << "Failed to write output.tga" << std::endl;
    }

    // delete [] zbuffer;
    }
    return 0;
}