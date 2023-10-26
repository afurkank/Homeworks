#include <iostream>
#include "parser.h"
#include "ppm.h"

typedef unsigned char RGB[3];

// float dot_product(std::vector<parser::Vec3f> vec1, std::vector<parser::Vec3f> vec2, float cosT){
//     float result = 0;
//     for(int i = 0; i < vec1.size(); i++){
//         result += vec1[i] * vec2[i]
//     }
// }

parser::Vec3f applyShading(parser::Vec3f ray, parser::hitRecord hitRecord, parser::Scene scene){
    parser::Material material = hitRecord.material;
    parser::Vec3f ambient_light = scene.ambient_light;
    parser::Vec3f color = ambient_light*(material.ambient);

    if (material.is_mirror){
        parser::Vec3f reflectionRay = reflect(ray.dir, hitRecord.n);
        reflectionRay.depth = ray.depth + 1;
        color += material.mirror * computeColor(reflectionRay, scene);
    }

    for(int i = 0; i < scene.point_lights.size(); i++){
        if(!inShadow(hitRecord.x, I)){
            color += diffuseTerm(hitRecord, I) + specularTerm(hitRecord, I);
        }
    }
    return color;
}

bool closestHit(parser::Vec3f ray, int i, int j, parser::hitRecord hitRecord, parser::Scene scene, parser::Camera camera){
    float pixel_width, pixel_height;
    parser::Vec4f near_plane = camera.near_plane;

    float left, right, bottom, top;
    left = near_plane.x, right = near_plane.y, bottom = near_plane.z, top = near_plane.w;
    pixel_width = (right - left) / camera.image_width;
    pixel_height = (top - bottom) / camera.image_height;

    float pixel_ij_cam_x = left + (i + 0.5)*pixel_width;
    float pixel_ij_cam_y = top - (j + 0.5)*pixel_height;
    float pixel_ij_cam_z = -camera.near_distance;

    parser::Vec3f pixel_ij_cam = parser::Vec3f(pixel_ij_cam_x, pixel_ij_cam_y, pixel_ij_cam_z);

    parser::Vec3f u = camera.up, w = -camera.gaze;
    parser::Vec3f v = parser::Vec3f::cross(u, w);

    parser::Vec3f pixel_ij_world = camera.position + pixel_ij_cam
}

parser::Vec3f computeColor(parser::Vec3f ray, int i, int j, parser::Scene scene){
    if (ray.depth > scene.max_recursion_depth){
        return parser::Vec3f();
    }
    else if(closestHit(ray, i, j, hitRecord, scene)){
        return applyShading(ray, hitRecord);
    }
    else if(ray.depth == 0){
        return scene.background_color;
    }
    else{
        return parser::Vec3f();
    }
}

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

    std::cout << "There are this many cameras in the scene: " << cameras.size() << std::endl;
    int i, k;
    for (i = 0; i < cameras.size(); i++){
        parser::Camera camera = cameras[i];
        int width = camera.image_width, height = camera.image_width;
        unsigned char* image = new unsigned char [width * height * 3];
        const char* image_name = camera.image_name.c_str();
        int j = 0;
        for (int y = 0; y < height; ++y){
            for (int x = 0; x < width; ++x){
                // looping through each pixel

                // compute the viewing ray
                // ray eqn: r(t) = e + t*d
                parser::Vec3f camera_pos = camera.position;
                parser::Vec3f gaze = camera.gaze;
                parser::Vec3f up = camera.up;

                parser::Vec4f near_plane = camera.near_plane;
                float near_distance = camera.near_distance;
                
                parser::Vec3f e = camera_pos;
                parser::Vec3f t = camera.gaze;
                float d = camera.near_distance;

                parser::Vec3f ray = e + t*d;
                ray.depth = 0;
                parser::Vec3f color = computeColor(ray, scene);
                // convert color from float to integer
                // and clamp it to be in-between (0,255)
                parser::Vec3i clamped_color; // = clamp(color)
                image[j++] = clamped_color.x; // Red
                image[j++] = clamped_color.y; // Green
                image[j++] = clamped_color.z; // Blue
            }
        }
        write_ppm(image_name, image, width, height);
    }
}
