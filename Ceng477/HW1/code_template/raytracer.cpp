#include "parser.h"
#include "ppm.h"

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
        // Handle null pointer error
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
        float beta = data.beta, gamma = data.gamma, t = data.t;
        // p(beta, gamma) = vec_a + beta*(vec_b-vec_a) + gamma*(vec_c-vec_a)

        if(t < t_min && data.hit){
            // TODO: check the cosTheta being more than 90 degrees
            t_min = t;
            flag = true;
            hitRecord->material_id = triangle.material_id;
            // get triangle normal
            hitRecord->n = triangle_normals[i];
            // calculate hit point
            parser::Vec3f vec_a, vec_b, vec_c;
            vec_a = scene->vertex_data[a]; vec_b = scene->vertex_data[b]; vec_c = scene->vertex_data[c];
            parser::Vec3f p = vec_a + (vec_b - vec_a) * beta + (vec_c - vec_a) * gamma;
            hitRecord->p = p;
        }
    }
    
    // loop through meshes
    for(int i = 0; i < scene->meshes.size() && !flag; i++){
        const parser::Mesh& mesh = scene->meshes[i];
        
        // loop through individual faces of the mesh
        for(int j = 0; j < mesh.faces.size(); j++){
            const parser::Face& face = mesh.faces[j];
            int k = 0;
            bool eq = false;

            parser::triangle_ray_intersection_data data = compute_tri_ray_inter(
                face.v0_id, face.v1_id, face.v2_id, o, d, scene
            );

            if(data.t < t_min && data.hit){
                // TODO: check the cosTheta being more than 90 degrees
                t_min = data.t;
                flag = true;
                // record the hit point
                hitRecord->material_id = mesh.material_id;
                // calculate normal
                parser::Vec3f a, b, c;
                a = scene->vertex_data[face.v0_id];
                b = scene->vertex_data[face.v1_id];
                c = scene->vertex_data[face.v2_id];
                parser::Vec3f normal;
                normal = parser::Vec3f::cross((c - b),(a - b)).normalize();
                hitRecord->n = normal;
                // calculate hit point
                const parser::Vec3f& p = scene->vertex_data[face.v0_id] +
                (scene->vertex_data[face.v1_id] - scene->vertex_data[face.v0_id]) * data.beta +
                (scene->vertex_data[face.v2_id] - scene->vertex_data[face.v0_id]) * data.gamma;
                hitRecord->p = p;
            }
            //processedFaces.insert(faceId);
        }
    }
    return flag;
}

parser::Vec3i applyShading(parser::Ray *ray, parser::hitRecord *hitRecord, parser::Scene *scene){
    parser::Material material = scene->materials[hitRecord->material_id];
    parser::Vec3f color;
    // add ambient term
    // L_a(x, w_o) = k_a * I_a
    parser::Vec3f k_a = material.ambient;
    parser::Vec3f I_a = scene->ambient_light;
    color += k_a * I_a;

    // Mirror reflection
    /* if (material.is_mirror) {
        parser::Ray reflectionRay;
        // w_r = -w_o + 2*n*(n . w_o)
        parser::Vec3f w_o = ray.direction;
        parser::Vec3f reflected_ray = -w_o + (hitRecord->n)*(hitRecord->n.dot(w_o))*2;
        reflectionRay.depth += 1;
        reflectionRay.direction = reflected_ray.normalize();
        reflectionRay.origin = hitRecord->p;
        parser::Vec3f reflectionColor = computeColor(&reflectionRay, scene);
        color += material.mirror * reflectionColor;
    } */

    for(int i = 0; i < scene->point_lights.size(); i++){
        // add diffuse term
        // L_d(x, w_o) = k_d * cosTheta * E_i(x, w_i)
        // cosTheta = max(0, w_i . n)
        // E_i(x, w_i) = I * r^2
        parser::PointLight point_light = scene->point_lights[i];
        parser::Vec3f light_dir = (point_light.position - hitRecord->p).normalize();

        float cosTheta = std::max(0.0f, light_dir.dot(hitRecord->n));
        parser::Vec3f k_d = material.diffuse;
        float r = parser::Vec3f::distance(point_light.position, hitRecord->p);
        parser::Vec3f E_i = point_light.intensity / pow(r, 2);

        parser::Vec3f L_d = E_i * k_d * cosTheta;

        color += L_d;

        // add specular shading
        //
    }
    // clamp color
    parser::Vec3i clamped_color;
    clamped_color.x = std::min(std::max(color.x, 0.0f), 255.0f);
    clamped_color.y = std::min(std::max(color.y, 0.0f), 255.0f);
    clamped_color.z = std::min(std::max(color.z, 0.0f), 255.0f);
    return clamped_color;
}

parser::Vec3i computeColor(parser::Ray *ray, parser::Scene *scene){
    parser::hitRecord hitRecord;
    if(closestHit(ray, &hitRecord, scene)){
        // if there is a hit
        return applyShading(ray, &hitRecord, scene);
    }
    else return scene->background_color;
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

parser::Ray computeRay(int x, int y, parser::Camera *camera, parser::Scene *scene){
    parser::Vec3f u = parser::Vec3f::cross(camera->gaze, camera->up).normalize();
    parser::Vec3f v = -camera->up; // if i dont put minus here, the character is upside down lol

    // Center of the near plane
    parser::Vec3f near_center = camera->position + camera->gaze.normalize() * camera->near_distance;

    parser::Vec4f near_plane = camera->near_plane;
    // Compute the extents of the near plane
    float l = near_plane.x;
    float r = near_plane.y;
    float b = near_plane.z;
    float t = near_plane.w;

    // Compute the Vectors to the corners of the near plane
    parser::Vec3f to_left = u * l;
    parser::Vec3f to_right = u * r;
    parser::Vec3f to_bottom = v * b;
    parser::Vec3f to_top = v * t;

    // For each pixel (x, y)
    float i = l + (r - l) * (x + 0.5) / camera->image_width;
    float j = b + (t - b) * (y + 0.5) / camera->image_height;

    // Point on the near plane for this pixel
    parser::Vec3f point_on_plane = near_center + to_right * i + to_top * j;

    // Ray direction
    parser::Vec3f ray_dir = point_on_plane - camera->position;
    ray_dir = ray_dir.normalize();

    // Create the ray
    parser::Ray ray;
    ray.origin = camera->position;
    ray.direction = ray_dir;
    ray.depth = 0;

    return ray;
}

void* render_thread(void* arg) {
    // Cast argument to ThreadData pointer
    parser::ThreadData* data = static_cast<parser::ThreadData*>(arg);
    parser::Scene* scene = data->scene;
    parser::Camera* camera = data->camera;
    unsigned char* image = data->image;

    // Calculate the size of a pixel in the camera space
    float pixelWidth = (camera->near_plane.x - camera->near_plane.y) / camera->image_width;
    float pixelHeight = (camera->near_plane.w - camera->near_plane.z) / camera->image_height;

    // Loop over each pixel in the assigned rows
    for (int y = data->startRow; y < data->endRow; ++y) {
        for (int x = 0; x < data->width; ++x) {
            // Convert pixel (x, y) to camera space or world space here as required
            // and compute the ray from the camera position through the pixel
            // Here, it's assumed that 'computeRay' is a function that computes the ray for a given pixel
            //std::cout << "y: " << y << ", " << "x: "<< x << std::endl;
            parser::Ray ray = computeRay(x, y, camera, scene); // You would need to implement computeRay

            // Compute color for the ray
            //std::cout << "computeRay successfully called" << std::endl;
            parser::Vec3i color = computeColor(&ray, scene); // computeColor should be thread-safe
            
            // The index in the image array needs to consider the width of the image
            int index = (y * data->width + x) * 3;
            //std::cout << "index: " << index << std::endl;

            // Set pixel color in image buffer, assuming color values are clamped to 0-255

            image[index] = (unsigned char)(color.x); // Red
            image[index + 1] = (unsigned char)(color.y); // Green
            image[index + 2] = (unsigned char)(color.z); // Blue
        }
    }

    return nullptr;
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
    const int num_threads = 256; // or std::thread::hardware_concurrency() to use all hardware threads
    pthread_t threads[num_threads];
    parser::ThreadData threadData[num_threads];

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < scene.cameras.size(); i++) {
        parser::Camera& camera = scene.cameras[i];
        int width = camera.image_width, height = camera.image_height;
        std::cout << "width: " << width << ", height: " << height << std::endl;
        unsigned char* image = new unsigned char[width * height * 3];

        // Create threads
        for (int j = 0; j < num_threads; j++) {
            threadData[j].startRow = j * height / num_threads;
            threadData[j].endRow = (j + 1) * height / num_threads;
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

        // Wait for threads to finish
        for (int j = 0; j < num_threads; j++) {
            pthread_join(threads[j], nullptr);
        }

        // Write the image to file
        write_ppm(camera.image_name.c_str(), image, width, height);
        
        // Cleanup
        delete[] image;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    return 0;
}
