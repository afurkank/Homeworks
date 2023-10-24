#include <iostream>
#include "parser.h"
#include "ppm.h"

typedef unsigned char RGB[3];

int main(int argc, char* argv[])
{
    // Sample usage for reading an XML scene file
    parser::Scene scene;

    scene.loadFromXml(argv[1]); // reading the scene(lights, objects, etc.)

    std::vector<parser::Camera> cameras = scene.cameras;
    std::vector<parser::Material> materials = scene.materials;
    std::vector<parser::Mesh> meshes = scene.meshes;
    std::vector<parser::PointLight> pointLights = scene.point_lights;
    std::vector<parser::Sphere> spheres = scene.spheres;
    std::vector<parser::Triangle> triangles = scene.triangles;
    parser::Vec3i background_color = scene.background_color;
    parser::Vec3f ambient_light = scene.ambient_light;
    parser::Vec4f vec4f;
    parser::Face face;

    // The code below creates a test pattern and writes
    // it to a PPM file to demonstrate the usage of the
    // ppm_write function.
    //
    // Normally, you would be running your ray tracing
    // code here to produce the desired image.

    const RGB BAR_COLOR[8] =
    {
        { 255, 255, 255 },  // 100% White
        { 255, 255,   0 },  // Yellow
        {   0, 255, 255 },  // Cyan
        {   0, 255,   0 },  // Green
        { 255,   0, 255 },  // Magenta
        { 255,   0,   0 },  // Red
        {   0,   0, 255 },  // Blue
        {   0,   0,   0 },  // Black
    };

    int width = 640, height = 480;
    int columnWidth = width / 8; // this is the mod value

    unsigned char* image = new unsigned char [width * height * 3]; // this is our image plane

    std::cout << "There are this many cameras in the scene: " << cameras.size() << std::endl;
    int i, j, k;
    for (i = 0; i < cameras.size(); i++){
        
    }


    // for (int y=0; y < height; y++){
    //     for (int x=0; x < width; x++){
            
    //     }
    // }

    int i = 0;
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int colIdx = x / columnWidth; // this is like a mod operation
            image[i++] = BAR_COLOR[colIdx][0]; // Red
            image[i++] = BAR_COLOR[colIdx][1]; // Green
            image[i++] = BAR_COLOR[colIdx][2]; // Blue
        }
    }

    write_ppm("test.ppm", image, width, height); // constructing the image from the image plane

}
