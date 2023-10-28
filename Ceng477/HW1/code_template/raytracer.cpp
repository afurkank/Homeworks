#include <iostream>
#include <cmath>
#include "parser.h"
#include "ppm.h"

typedef unsigned char RGB[3];

// float dot_product(std::vector<parser::Vec3f> vec1, std::vector<parser::Vec3f> vec2, float cosT){
//     float result = 0;
//     for(int i = 0; i < vec1.size(); i++){
//         result += vec1[i] * vec2[i]
//     }
// }

parser::Vec3f compute_ray(parser::Vec3f ray, float t){
    return ray * t;
}

float compute_determinant(parser::Vec3f vec1, parser::Vec3f vec2, parser::Vec3f vec3){
    float a = vec1.x, b = vec1.y, c = vec1.z;
    float d = vec2.x, e = vec2.y, f = vec2.z;
    float g = vec3.x, h = vec3.y, i = vec3.z;

    float det = a*(e*i - h*f) + b*(g*f - d*i) + c*(d*h - e*g);
    return det;
}

parser::triangle_ray_intersection_data compute_tri_ray_inter(int a, int b, int c,
parser::Vec3f o, parser::Vec3f d, parser::Scene scene){
    parser::Vec3f triangle_a, triangle_b, triangle_c;
    triangle_a = scene.vertex_data[a];
    triangle_b = scene.vertex_data[b];
    triangle_c = scene.vertex_data[c];
    
    float beta, gamma, t, determinant_A;

    determinant_A = compute_determinant(triangle_a - triangle_b, triangle_a - triangle_c, d);

    beta = compute_determinant(triangle_a-o, triangle_a-triangle_c, d);
    gamma = compute_determinant(triangle_a-triangle_b, triangle_a-o, d);
    t = compute_determinant(triangle_a-triangle_b, triangle_a-triangle_c, triangle_a-o);

    beta/=determinant_A;
    gamma/=determinant_A;
    t/=determinant_A;
}

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

    parser::Vec3f u = camera.up, w = -camera.gaze;
    parser::Vec3f v = parser::Vec3f::cross(u, w);

    parser::Vec3f pixel_ij_world = camera.position + 
    u * pixel_ij_cam_x +
    v * pixel_ij_cam_y +
    w * pixel_ij_cam_z;

    parser::Vec3f d = pixel_ij_world - camera.position;
    parser::Vec3f o = camera.position;

    // iterate through the objects and find if there is an intersection with the
    // ray equation
    int i = 0;
    float t_min = INFINITY;
    bool flag = false;
    parser::Sphere sphere;
    parser::Triangle triangle;
    for(; i < scene.spheres.size(); i++){
        // loop through spheres
        
        parser::Sphere sphere = scene.spheres[i];
        int material_id = sphere.material_id;
        int c_vertex_id = sphere.center_vertex_id;
        parser::Material material = scene.materials[material_id];
        // t = (-d.(o-c)-+sqrt((d.(o-c))^2 - (d.d)((o-c).(o-c)-R^2))) / d*d

        float t;
        parser::Vec3f c = scene.vertex_data[c_vertex_id];
        
        float R = sphere.radius;
        float first_term = (-d).dot(o-c);
        float second_term = sqrt(pow((d.dot(o-c)), 2) - (d.dot(d)*((o-c).dot(o-c)-pow(R, 2))));
        float denominator = d.dot(d);

        float t_1 = (first_term + second_term) / denominator;
        float t_2 = (first_term - second_term) / denominator;

        t = std::min(t_1, t_2);

        if (t < t_min){
            t_min = t;
            flag = true;
            // TODO: record the hit point
        }
    }
    for(i = 0; i < scene.triangles.size() && !flag; i++){
        // loop through triangles
        triangle = scene.triangles[i];
        
        int a, b, c;
        a = triangle.indices.v0_id; b = triangle.indices.v1_id; c = triangle.indices.v2_id;

        parser::triangle_ray_intersection_data data = compute_tri_ray_inter(
            a, b, c, o, d, scene
        );

        float beta = data.beta, gamma = data.gamma, t = data.t;

        if(t < t_min &&
        beta + gamma <= 1 &&
        beta >= 0 &&
        gamma >= 0){
            // TODO: check the cosTheta being more than 90 degrees
            t_min = t;
            flag = true;
            // TODO: record the hit point
        }
    }
    for(i = 0; i < scene.meshes.size() && !flag; i++){
        // loop through meshes
        parser::Mesh mesh = scene.meshes[i];

        int j = 0;
        for(; j < mesh.faces.size(); j++){
            parser::Face face = mesh.faces[j];

            int k = 0;
            bool eq = false;
            for(; k < scene.triangles.size(); k++){
                parser::Triangle tri = scene.triangles[k];
                
                if(tri.indices.v0_id == face.v0_id &&
                tri.indices.v1_id == face.v1_id &&
                tri.indices.v2_id == face.v2_id){
                    eq = true;
                    break;
                }
            }
            if(eq) continue;
            else{
                parser::triangle_ray_intersection_data data;

                int a, b, c;
                a = face.v0_id; b = face.v1_id; c = face.v2_id;

                data = compute_tri_ray_inter(
                    a, b, c, o, d, scene
                );

                float beta = data.beta, gamma = data.gamma, t = data.t;

                if(t < t_min &&
                beta + gamma <= 1 &&
                beta >= 0 &&
                gamma >= 0){
                    // TODO: check the cosTheta being more than 90 degrees
                    t_min = t;
                    flag = true;
                    // TODO: record the hit point
                }
            }
        }
    }
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
