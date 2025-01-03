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
    
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    // for (int i=0; i<model->nfaces(); i++) {
    //     std::vector<int> face = model->face(i);
    //     Vec2i screen_coords[3];
    //     for (int j=0; j<3; j++) {
    //         Vec3f world_coords = model->vert(face[j]);
    //         screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.);
    //     }
    //     TGAColor random_Color = TGAColor(rand()%255, rand()%255, rand()%255, 255);
    //     triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, random_Color);
    // }
for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);
            world_coords[j]  = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically();
    image.write_tga_file("framebuffer.tga");

    std::cout << "Attempting to write output.tga..." << std::endl;
    bool success = image.write_tga_file("output.tga");
    
    if (success) {
        std::cout << "Successfully wrote output.tga" << std::endl;
    } else {
        std::cout << "Failed to write output.tga" << std::endl;
    }

    // delete [] zbuffer;
    return 0;
}