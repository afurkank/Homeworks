#include <iostream>
#include "parser.h"
#include "ppm.h"
#include <chrono>
#include <pthread.h>
#include <limits>
#include <algorithm>
#include <set>

typedef unsigned char RGB[3];

std::vector<parser::Vec3f> triangle_normals;
std::set<std::string> processedFaces;

std::string createFaceId(const parser::Face& face) {
    return std::to_string(face.v0_id) + "-" +
            std::to_string(face.v1_id) + "-" +
            std::to_string(face.v2_id);
}

float compute_determinant(parser::Vec3f vec1, 
parser::Vec3f vec2, parser::Vec3f vec3){
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
    
    parser::triangle_ray_intersection_data data;

    float determinant_A = compute_determinant(triangle_a - triangle_b, triangle_a - triangle_c, d);

    if(std::abs(determinant_A) < std::numeric_limits<float>::epsilon()){
        data.hit = false;
        return data;
    }

    float beta = compute_determinant(triangle_a-o, triangle_a-triangle_c, d) / determinant_A;
    float gamma = compute_determinant(triangle_a-triangle_b, triangle_a-o, d) / determinant_A;
    float t = compute_determinant(triangle_a-triangle_b, triangle_a-triangle_c, triangle_a-o) / determinant_A;

    if (beta >= 0.0f && gamma >= 0.0f && (beta + gamma) <= 1.0f) {
        data.hit = true;
        data.beta = beta;
        data.gamma = gamma;
        data.t = t;
    } else {
        data.hit = false;
    }

    return data;
}

parser::sphere_ray_intersection_data compute_sphere_ray_inter(parser::Sphere sphere,
parser::Vec3f o, parser::Vec3f d, parser::Scene scene){
    parser::sphere_ray_intersection_data data;
    int c_vertex_id = sphere.center_vertex_id;
    // t = (-d.(o-c)-+sqrt((d.(o-c))^2 - (d.d)((o-c).(o-c)-R^2))) / d*d
    float t;
    parser::Vec3f c = scene.vertex_data[c_vertex_id];
    
    float R = sphere.radius;
    parser::Vec3f o_c = o - c;
    float first_term = (-d).dot(o_c);
    float discriminant = pow(d.dot(o_c), 2) - (d.dot(d) * (o_c.dot(o_c) - pow(R, 2)));

    if (discriminant < 0) {
        data.hit = false;
        return data;
    }

    discriminant = sqrt(discriminant);
    float denominator = d.dot(d);

    float t_1 = (first_term + discriminant) / denominator;
    float t_2 = (first_term - discriminant) / denominator;

    if (t_1 > 0 && t_2 > 0) {
        data.t = std::min(t_1, t_2);
        data.hit = true;
    } else if (t_1 > 0) {
        data.t = t_1;
        data.hit = true;
    } else if (t_2 > 0) {
        data.t = t_2;
        data.hit = true;
    } else {
        data.hit = false;
    }

    return data;
}

parser::Vec3f get_pixel_ij_world(parser::Scene scene, int x, int y, parser::Camera camera){
    float pixel_width, pixel_height;
    parser::Vec4f near_plane = camera.near_plane;

    float left, right, bottom, top;
    left = near_plane.x, right = near_plane.y, bottom = near_plane.z, top = near_plane.w;
    pixel_width = (right - left) / camera.image_width;
    pixel_height = (top - bottom) / camera.image_height;

    float pixel_ij_cam_x = left + (x + 0.5)*pixel_width;
    float pixel_ij_cam_y = top - (y + 0.5)*pixel_height;
    float pixel_ij_cam_z = -camera.near_distance;

    parser::Vec3f w = -camera.gaze.normalize();
    parser::Vec3f v = camera.up.normalize();
    parser::Vec3f u = parser::Vec3f::cross(v, w);

    parser::Vec3f pixel_ij_world = camera.position + 
    u * pixel_ij_cam_x +
    v * pixel_ij_cam_y -
    w * pixel_ij_cam_z;

    return pixel_ij_world;
}

// iterate through the objects and find if there is an intersection with the
// ray equation
bool closestHit(parser::Vec3f ray, int x, int y, parser::hitRecord &hitRecord, 
parser::Scene scene, parser::Camera camera){
    parser::Vec3f pixel_ij_world = get_pixel_ij_world(scene, x, y, camera);
    parser::Vec3f d = (pixel_ij_world - camera.position).normalize();
    parser::Vec3f o = camera.position;
    
    float t_min = INFINITY;
    bool flag = false;

    // loop through spheres
    for(int i = 0; i < scene.spheres.size(); i++){
        parser::Sphere sphere = scene.spheres[i];
        parser::Vec3f c = scene.vertex_data[sphere.center_vertex_id];
        parser::sphere_ray_intersection_data data;
        data = compute_sphere_ray_inter(sphere, o, d, scene);
        float t = data.t;
        if (t < t_min){
            t_min = t;
            flag = true;
            hitRecord.material_id = sphere.material_id;
            // calculate hit point
            parser::Vec3f p; // intersection point
            p = o + d * t;
            hitRecord.p = p;
            // calculate normal
            parser::Vec3f n; // surface normal at the intersection point
            n = p - c;
            n = n.normalize();
            hitRecord.n = n;
        }
    }
    // loop through triangles
    for(int i = 0; i < scene.triangles.size() && !flag; i++){
        parser::Triangle triangle = scene.triangles[i];
        int a, b, c;
        a = triangle.indices.v0_id; b = triangle.indices.v1_id; c = triangle.indices.v2_id;
        parser::triangle_ray_intersection_data data = compute_tri_ray_inter(
            a, b, c, o, d, scene
        );
        float beta = data.beta, gamma = data.gamma, t = data.t;
        // p(beta, gamma) = vec_a + beta*(vec_b-vec_a) + gamma*(vec_c-vec_a)

        if(t < t_min && data.hit){
            // TODO: check the cosTheta being more than 90 degrees
            t_min = t;
            flag = true;
            hitRecord.material_id = triangle.material_id;
            // get triangle normal
            hitRecord.n = triangle_normals[i];
            // calculate hit point
            parser::Vec3f vec_a, vec_b, vec_c;
            vec_a = scene.vertex_data[a]; vec_b = scene.vertex_data[b]; vec_c = scene.vertex_data[c];
            parser::Vec3f p = vec_a + (vec_b - vec_a) * beta + (vec_c - vec_a) * gamma;
            hitRecord.p = p;
        }
    }
    // loop through meshes
    for(int i = 0; i < scene.meshes.size() && !flag; i++){
        const parser::Mesh& mesh = scene.meshes[i];
        
        // loop through individual faces of the mesh
        for(int j = 0; j < mesh.faces.size(); j++){
            const parser::Face& face = mesh.faces[j];
            int k = 0;
            bool eq = false;

            // check if we are computing ray interaction twice for a triangle
            std::string faceId = createFaceId(face);
            if (processedFaces.find(faceId) != processedFaces.end()) {
                // We've already processed this face, skip it.
                continue;
            }
            parser::triangle_ray_intersection_data data = compute_tri_ray_inter(
                face.v0_id, face.v1_id, face.v2_id, o, d, scene
            );

            if(data.t < t_min && data.hit){
                // TODO: check the cosTheta being more than 90 degrees
                t_min = data.t;
                flag = true;
                // record the hit point
                hitRecord.material_id = mesh.material_id;
                // calculate normal
                //std::vector<parser::Vec3f> vertices = scene.vertex_data;
                parser::Vec3f normal;
                //normal = parser::Vec3f::cross((vertices[c] - vertices[b]),(vertices[a] - vertices[b]));
                normal = normal.normalize();
                hitRecord.n = normal;
                // calculate hit point
                const parser::Vec3f& p = scene.vertex_data[face.v0_id] +
                (scene.vertex_data[face.v1_id] - scene.vertex_data[face.v0_id]) * data.beta +
                (scene.vertex_data[face.v2_id] - scene.vertex_data[face.v0_id]) * data.gamma;
                hitRecord.p = p;
            }
            processedFaces.insert(faceId);
        }
    }
    return flag;
}

parser::Vec3i applyShading(parser::Vec3f ray, parser::hitRecord hitRecord, parser::Scene scene){
    //parser::Material material = hitRecord.material;
    //parser::Vec3f ambient_light = scene.ambient_light;
    parser::Vec3i color = parser::Vec3i(255, 0, 0); //ambient_light*(material.ambient);

    /* if (material.is_mirror){
        parser::Vec3f reflectionRay = reflect(ray.dir, hitRecord.n);
        reflectionRay.depth = ray.depth + 1;
        color += material.mirror * computeColor(reflectionRay, scene);
    }

    for(int i = 0; i < scene.point_lights.size(); i++){
        if(!inShadow(hitRecord.x, I)){
            color += diffuseTerm(hitRecord, I) + specularTerm(hitRecord, I);
        }
    } */
    return color;
}

parser::Vec3i computeColor(parser::Vec3f ray, int x, int y, 
parser::Scene scene, parser::Camera camera){
    parser::hitRecord hitRecord;
    if(closestHit(ray, x, y, hitRecord, scene, camera)){
        // if there is a hit
        return applyShading(ray, hitRecord, scene);
    }
    else return parser::Vec3i();
    /* parser::hitRecord hitRecord;
    if (ray.depth > scene.max_recursion_depth){
        // if the recursion limit is reached
        return parser::Vec3i(); // returns (0,0,0)
    }
    else if(closestHit(ray, x, y, hitRecord, scene, camera)){
        // if there is a hit
        return applyShading(ray, hitRecord, scene);
    }
    else if(ray.depth == 0){
        // if there is no hit and ray is the primary ray
        return scene.background_color;
    }
    else{
        // there is no hit and the ray is reflected from a surface
        return parser::Vec3i(); // returns (0,0,0)
    } */
}

int main(int argc, char* argv[])
{
    // Sample usage for reading an XML scene file
    parser::Scene scene;
    scene.loadFromXml(argv[1]); // reading the scene(lights, objects, etc.)

    std::vector<parser::Camera> cameras = scene.cameras;
    std::vector<parser::Triangle> triangles = scene.triangles;
    std::vector<parser::Vec3f> vertices = scene.vertex_data;

    triangle_normals.reserve(triangles.size());

    for (int i = 0; i < triangles.size(); i++) {
        const parser::Triangle& triangle = triangles[i];
        
        const int a = triangle.indices.v0_id;
        const int b = triangle.indices.v1_id;
        const int c = triangle.indices.v2_id;

        parser::Vec3f normal = parser::Vec3f::cross(vertices[c] - vertices[b], vertices[a] - vertices[b]).normalize();
        
        triangle_normals.push_back(normal);
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < cameras.size(); i++){
        parser::Camera camera = cameras[i];
        int width = camera.image_width, height = camera.image_height;
        unsigned char* image = new unsigned char [width * height * 3];
        const char* image_name = camera.image_name.c_str();
        int j = 0;
        for (int y = 0; y < height; y++){
            for (int x = 0; x < width; x++){
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

                parser::Vec3i color = computeColor(ray, x, y, scene, camera); // TODO: seg fault here
                std::cout << "pixel i,j: "<< x << ", " << y << std::endl;
                // convert color from float to integer
                // and clamp it to be in-between (0,255)
                parser::Vec3i clamped_color = color; // = clamp(color)
                image[j++] = (unsigned char)clamped_color.x; // Red
                image[j++] = (unsigned char)clamped_color.y; // Green
                image[j++] = (unsigned char)clamped_color.z; // Blue
            }
        }
        write_ppm(image_name, image, width, height);
        delete image;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    return 0;
}
