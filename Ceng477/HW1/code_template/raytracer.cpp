#include "parser.h"
#include "ppm.h"

parser::Vec3f applyShading(parser::Ray *ray, parser::hitRecord *hitRecord, parser::Scene *scene);
parser::Vec3f computeColor(parser::Ray *ray, parser::Scene *scene);

typedef unsigned char RGB[3];

std::vector<parser::Vec3f> triangle_normals;

float compute_determinant(parser::Vec3f vec1, 
parser::Vec3f vec2, parser::Vec3f vec3){
    float a = vec1.x, d = vec2.x, g = vec3.x;
    float b = vec1.y, e = vec2.y, h = vec3.y;
    float c = vec1.z, f = vec2.z, i = vec3.z;

    float det = a*(e*i - h*f) + b*(g*f - d*i) + c*(d*h - e*g);
    return det;
}

parser::triangle_ray_intersection_data compute_tri_ray_inter(int a, int b, int c,
parser::Vec3f o, parser::Vec3f d, parser::Scene *scene){
    parser::Vec3f triangle_a, triangle_b, triangle_c;
    triangle_a = scene->vertex_data[a];
    triangle_b = scene->vertex_data[b];
    triangle_c = scene->vertex_data[c];
    
    parser::triangle_ray_intersection_data data;

    float determinant_A = compute_determinant(
        triangle_a - triangle_b, triangle_a - triangle_c, d);

    if(std::abs(determinant_A) < std::numeric_limits<float>::epsilon()){
        data.hit = false;
        return data;
    }

    //float alpha = compute_determinant(o-triangle_c, triangle_b-triangle_c, -d) / determinant_A;
    //if(alpha < 0.0f){data.hit = false; return data;}
    float beta = compute_determinant(triangle_a - o, triangle_a - triangle_c, d) / determinant_A;
    if(beta < 0.0f){data.hit = false; return data;}
    float gamma = compute_determinant(triangle_a - triangle_b, triangle_a - o, d) / determinant_A;
    if(gamma < 0.0f){data.hit = false; return data;}
    float t = compute_determinant(triangle_a - triangle_b, triangle_a - triangle_c, triangle_a - o) / determinant_A;
    if(t < 0.0f){data.hit = false; return data;}

    if (beta + gamma <= 1.0f &&
        0.0f <= beta &&
        0.0f <= gamma &&
        0.0f < t)
    {
        data.hit = true;
        data.gamma = gamma;
        data.beta = beta;
        data.t = t;
    }
    else data.hit = false;

    return data;
}

parser::sphere_ray_intersection_data compute_sphere_ray_inter(parser::Sphere *sphere,
parser::Vec3f o, parser::Vec3f d, parser::Scene *scene){
    parser::sphere_ray_intersection_data data;
    int c_vertex_id = sphere->center_vertex_id;
    // t = (-d.(o-c)-+sqrt((d.(o-c))^2 - (d.d)((o-c).(o-c)-R^2))) / d*d
    float t;
    parser::Vec3f c = scene->vertex_data[c_vertex_id];
    
    float R = sphere->radius;
    parser::Vec3f o_c = o - c;
    float first_term = (-d).dot(o_c);
    float discriminant = pow(d.dot(o_c), 2) - (d.dot(d) * (o_c.dot(o_c) - pow(R, 2)));

    if (discriminant < 0) {
        // there is no intersection
        data.hit = false;
        return data;
    }

    discriminant = sqrt(discriminant);
    float denominator = d.dot(d);

    float t_1 = (first_term + discriminant) / denominator;
    float t_2 = (first_term - discriminant) / denominator;

    if (t_1 > 0 && t_2 > 0){
        // two intersections
        data.t = std::min(t_1, t_2);
        data.hit = true;
    }
    else if (t_1 > 0){
        data.t = t_1;
        data.hit = true;
    }
    else if (t_2 > 0){
        data.t = t_2;
        data.hit = true;
    }
    else{
        // sphere is behind the camera
        data.hit = false;
    }

    return data;
}

// iterate through the objects and find if there is an intersection with the
// ray equation
bool closestHit(parser::Ray *ray, parser::hitRecord *hitRecord, parser::Scene *scene){
    if (!ray || !hitRecord || !scene) {
        std::cout << "one of the pointers is null" << std::endl;
        return false;
    }
    parser::Vec3f d = ray->direction;
    parser::Vec3f o = ray->origin;
    
    float t_min = INFINITY;
    bool flag = false;

    // loop through spheres
    for(int i = 0; i < scene->spheres.size(); i++){
        parser::Sphere sphere = scene->spheres[i];
        parser::Vec3f c = scene->vertex_data[sphere.center_vertex_id];
        parser::sphere_ray_intersection_data data;
        data = compute_sphere_ray_inter(&sphere, o, d, scene);
        float t = data.t;
        if (t < t_min){
            t_min = t;
            flag = true;
            hitRecord->material_id = sphere.material_id;
            // calculate hit point
            parser::Vec3f p; // intersection point
            p = o + d * t;
            hitRecord->p = p;
            // calculate normal
            parser::Vec3f n; // surface normal at the intersection point
            n = p - c;
            n = n.normalize();
            hitRecord->n = n;
        }
    }
    // loop through triangles
    for(int i = 0; i < scene->triangles.size() && !flag; i++){
        parser::Triangle triangle = scene->triangles[i];
        int a, b, c;
        a = triangle.indices.v0_id; b = triangle.indices.v1_id; c = triangle.indices.v2_id;
        parser::triangle_ray_intersection_data data = compute_tri_ray_inter(
            a, b, c, o, d, scene
        );

        if(data.hit && data.t < t_min){
            float gamma = data.gamma, beta = data.beta, t = data.t;
            // TODO: check the cosTheta being more than 90 degrees
            t_min = t;
            flag = true;
            hitRecord->material_id = triangle.material_id;
            // get triangle normal
            hitRecord->n = triangle_normals[i];
            // calculate hit point
            parser::Vec3f tri_a = scene->vertex_data[a];
            parser::Vec3f tri_b = scene->vertex_data[b];
            parser::Vec3f tri_c = scene->vertex_data[c];
            // p = tri_a + beta * (b - a) + gamma * (c - a)
            parser::Vec3f p = tri_a + (tri_b - tri_a) * beta + (tri_c - tri_a) * gamma;
            hitRecord->p = p;
        }
    }
    
    // loop through meshes
    for(int i = 0; i < scene->meshes.size() && !flag; i++){
        const parser::Mesh& mesh = scene->meshes[i];
        
        // loop through individual faces of the mesh
        for(int j = 0; j < mesh.faces.size(); j++){
            const parser::Face& face = mesh.faces[j];

            parser::triangle_ray_intersection_data data = compute_tri_ray_inter(
                face.v0_id, face.v1_id, face.v2_id, o, d, scene
            );

            if(data.hit && data.t < t_min){
                float gamma = data.gamma, beta = data.beta, t = data.t;
                t_min = data.t;
                flag = true;
                // record the hit point
                hitRecord->material_id = mesh.material_id;
                // calculate normal
                parser::Vec3f tri_a = scene->vertex_data[face.v0_id];
                parser::Vec3f tri_b = scene->vertex_data[face.v1_id];
                parser::Vec3f tri_c = scene->vertex_data[face.v2_id];
                parser::Vec3f normal;
                normal = parser::Vec3f::cross((tri_c - tri_b),(tri_a - tri_b)).normalize();
                hitRecord->n = normal;
                // calculate hit point
                parser::Vec3f p = tri_a + (tri_b - tri_a) * beta + (tri_c - tri_a) * gamma;
                hitRecord->p = p;
            }
        }
    }
    return flag;
}

parser::Vec3f computeColor(parser::Ray *ray, parser::Scene *scene){
    if (ray->depth > scene->max_recursion_depth){
        // if the recursion limit is reached
        return parser::Vec3f(); // returns (0,0,0)
    }
    parser::hitRecord hitRecord;
    if(closestHit(ray, &hitRecord, scene)){
        // if there is a hit
        return applyShading(ray, &hitRecord, scene);
    }
    else if(ray->depth == 0){
        // if there is no hit and ray is the primary ray
        parser::Vec3i bg = scene->background_color;
        return parser::Vec3f(bg.x, bg.y, bg.z);
    }
    else{
        // there is no hit and the ray is not a primary ray
        return parser::Vec3f(); // returns (0,0,0)
    }
}

parser::Vec3f applyShading(parser::Ray *ray, parser::hitRecord *hitRecord, parser::Scene *scene){
    parser::Material material = scene->materials[hitRecord->material_id];
    parser::Vec3f color;
    // add ambient term
    // L_a(x, w_o) = k_a * I_a
    color = material.ambient * scene->ambient_light;

    // Mirror reflection
    if (material.is_mirror) {
        parser::Ray reflectionRay;
        // w_r = -w_o + 2*n*(n . w_o)
        parser::Vec3f w_o = (ray->origin - hitRecord->p).normalize();
        parser::Vec3f reflected_ray = -w_o + (hitRecord->n)*(hitRecord->n.dot(w_o))*2;
        reflectionRay.depth += ray->depth + 1;
        reflectionRay.direction = reflected_ray.normalize();
        reflectionRay.origin = hitRecord->p;
        parser::Vec3f reflectionColor = computeColor(&reflectionRay, scene);
        color += material.mirror * reflectionColor;
    }

    for(int i = 0; i < scene->point_lights.size(); i++){
            
        parser::PointLight point_light = scene->point_lights[i];
        parser::Vec3f light_dir = (point_light.position - hitRecord->p).normalize();

        float cosTheta = std::max(0.0f, light_dir.dot(hitRecord->n));
        if(!(cosTheta == 0.0f)){
            // add diffuse term
            // L_d(x, w_o) = k_d * cosTheta * E_i(x, w_i)
            // cosTheta = max(0, w_i . n)
            // E_i(x, w_i) = I * r^2
            float distance = parser::Vec3f::distance(point_light.position, hitRecord->p);
            parser::Vec3f E = point_light.intensity / pow(distance, 2);

            parser::Vec3f diffuse_shading = E * material.diffuse * cosTheta;

            color += diffuse_shading;

            // add specular shading
            // L_s(x, w_o) = k_s * (cosAlpha^phong_exponent) * E_i(x, w_i)
            // cosAlpha = max(0, n . h)
            // h = (w_i + w_o) / ||w_i + w_o||
            parser::Vec3f k_s, h, w_o;
            w_o = (ray->origin - hitRecord->p).normalize();
            h = (light_dir + w_o) / (light_dir + w_o).length();
            float cosAlpha = std::max(0.0f, hitRecord->n.dot(h));

            parser::Vec3f specular_shading = E * material.specular * (float) pow(cosAlpha, material.phong_exponent);

            color += specular_shading;
        }
    }
    return color;
}

parser::Ray computeRay(int x, int y, parser::Camera *camera, parser::Scene *scene){
    parser::Vec3f u = parser::Vec3f::cross(camera->gaze, camera->up).normalize();
    parser::Vec3f v = camera->up;

    parser::Vec4f near_plane = camera->near_plane;
    float l = near_plane.x;
    float r = near_plane.y;
    float b = near_plane.z;
    float t = near_plane.w;

    // s_u = (i+0.5) * (r-l)/n_x
    // s_v = (j+0.5) * (t-b)/n_y
    float s_u = (x + 0.5) * (r - l)/camera->image_width;
    float s_v = (y + 0.5) * (t - b)/camera->image_height;

    // p_ij_world = e + u * (l + s_u) + v * (t - s_v) + w * -distance
    parser::Vec3f pixel_ij_world = camera->position + 
    u * (l + s_u) + v * (t - s_v) + camera->gaze.normalize() * camera->near_distance;

    parser::Ray ray;
    ray.origin = camera->position;
    ray.direction = pixel_ij_world - camera->position;
    ray.depth = 0;

    return ray;
}

void* render_thread(void* arg) {
    parser::ThreadData* data = static_cast<parser::ThreadData*>(arg);
    parser::Scene* scene = data->scene;
    parser::Camera* camera = data->camera;
    unsigned char* image = data->image;

    float pixelWidth = (camera->near_plane.x - camera->near_plane.y) / camera->image_width;
    float pixelHeight = (camera->near_plane.w - camera->near_plane.z) / camera->image_height;

    for (int y = data->start; y < data->end; ++y) {
        for (int x = 0; x < data->width; ++x) {
            parser::Ray ray = computeRay(x, y, camera, scene);

            parser::Vec3f color = computeColor(&ray, scene);
            
            int index = (y * data->width + x) * 3;
            // clamp color
            parser::Vec3i clamped_color;
            clamped_color.x = std::min(std::max(color.x, 0.0f), 255.0f);
            clamped_color.y = std::min(std::max(color.y, 0.0f), 255.0f);
            clamped_color.z = std::min(std::max(color.z, 0.0f), 255.0f);

            image[index] = (unsigned char)(clamped_color.x); // Red
            image[index + 1] = (unsigned char)(clamped_color.y); // Green
            image[index + 2] = (unsigned char)(clamped_color.z); // Blue
        }
    }

    return nullptr;
}

int main(int argc, char* argv[])
{
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
    const int num_threads = 256;
    pthread_t threads[num_threads];
    parser::ThreadData threadData[num_threads];

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < scene.cameras.size(); i++) {
        parser::Camera& camera = scene.cameras[i];
        int width = camera.image_width, height = camera.image_height;
        unsigned char* image = new unsigned char[width * height * 3];

        for (int j = 0; j < num_threads; j++) {
            threadData[j].start = j * height / num_threads;
            threadData[j].end = (j + 1) * height / num_threads;
            threadData[j].width = width;
            threadData[j].height = height;
            threadData[j].image = image;
            threadData[j].scene = &scene;
            threadData[j].camera = &camera;

            if (pthread_create(&threads[j], nullptr, render_thread, (void*)&threadData[j])) {
                std::cerr << "Error creating thread" << std::endl;
                return 1;
            }
        }
        for (int j = 0; j < num_threads; j++) {
            pthread_join(threads[j], nullptr);
        }

        write_ppm(camera.image_name.c_str(), image, width, height);
        
        // Cleanup
        delete[] image;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    return 0;
}
