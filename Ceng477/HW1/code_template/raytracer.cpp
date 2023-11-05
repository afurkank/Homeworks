#include "parser.h"
#include "ppm.h"

parser::Vec3f applyShading(parser::Ray *ray, parser::hitRecord *hitRecord, parser::Scene *scene);
parser::Vec3f computeColor(parser::Ray *ray, parser::Scene *scene);

typedef unsigned char RGB[3];

std::vector<parser::Vec3f> triangle_normals;

void intersect_sphere(parser::sphere_intersect_data *data, parser::Ray *r,
parser::Sphere *sphere, parser::Scene *scene){

    float A,B,C, delta;
    
    parser::Vec3f c;
    c = scene->vertex_data[sphere->center_vertex_id-1];

    float t, t1, t2;

    C = (r->o.x - c.x)*(r->o.x - c.x) + (r->o.y - c.y) * (r->o.y - c.y) + (r->o.z - c.z) * (r->o.z - c.z) - sphere->radius * sphere->radius;

    B = 2 * r->d.x * (r->o.x - c.x) + 2 * r->d.y * (r->o.y - c.y) + 2 * r->d.z * (r->o.z - c.z);

    A = r->d.x * r->d.x + r->d.y * r->d.y + r->d.z * r->d.z;

    delta = B * B - 4 * A * C;

    if(delta < 0){
        data->hit = false;
        return;
    }
    else if(delta == 0){
        data->hit = true;
        t = -B / (2 * A);
        data->t = t;
        return;
    }
    else{
        data->hit = true;
        delta = sqrt(delta);
        t1 = (-B + delta) / (2*A);
        t2 = (-B - delta) / (2*A);
        if (t1 < t2) t = t1; else t = t2;
        data->t = t;
        return;
    }
}

float compute_determinant(parser::Vec3f vec1, 
parser::Vec3f vec2, parser::Vec3f vec3){
    float a = vec1.x, d = vec2.x, g = vec3.x;
    float b = vec1.y, e = vec2.y, h = vec3.y;
    float c = vec1.z, f = vec2.z, i = vec3.z;

    float det = a*(e*i - h*f) + b*(g*f - d*i) + c*(d*h - e*g);
    return det;
}

parser::tri_intersect_data intersect_triangle(parser::Ray *ray, parser::Face* face, parser::Scene *scene){
    parser::Vec3f a, b, c;
    parser::Vec3f o, d;
    parser::tri_intersect_data data;

    a = scene->vertex_data[face->v0_id-1];
    b = scene->vertex_data[face->v1_id-1];
    c = scene->vertex_data[face->v2_id-1];

    o = ray->o;
    d = ray->d;

    float detA = compute_determinant(a - b, a - c, d);
    
    float beta = compute_determinant(a - o, a - c, d) / detA;
    if(beta < 0.0f){data.hit = false; return data;}

    float gamma = compute_determinant(a - b, a - o, d) / detA;
    if(gamma < 0.0f){data.hit = false; return data;}

    float t = compute_determinant(a - b, a - c, a - o) / detA;
    if(t < 0.0f){data.hit = false; return data;}

    if(beta + gamma <= 1.0f){
        data.hit = true;
        data.gamma = gamma;
        data.beta = beta;
        data.t = t;
        return data;
    }
    else data.hit = false;
    return data;
}

// iterate through the objects and find if there is an intersection with the
// ray equation
bool closestHit(parser::Ray *ray, parser::hitRecord *hitRecord, parser::Scene *scene){
    parser::Vec3f d = ray->d;
    parser::Vec3f e = ray->o;
    
    float t_min = INFINITY;
    bool flag = false;
    
    // loop through spheres
    for(int i = 0; i < scene->spheres.size(); i++){
        parser::Sphere sphere = scene->spheres[i];
        parser::Vec3f c = scene->vertex_data[sphere.center_vertex_id-1];
        parser::sphere_intersect_data data;
        
        intersect_sphere(&data, ray, &sphere, scene);

        if (data.hit && data.t < t_min){
            hitRecord->material_id = sphere.material_id;
            parser::Vec3f p = e + d * data.t;
            hitRecord->p = p;

            hitRecord->n = (hitRecord->p - c).normalize();

            t_min = data.t;
            flag = true;
        }
    }
    // loop through triangles
    for(int i = 0; i < scene->triangles.size(); i++){
        parser::Triangle triangle = scene->triangles[i];

        parser::tri_intersect_data data;

        data = intersect_triangle(ray, &(triangle.indices), scene);

        if(data.hit && data.t < t_min){
            parser::Vec3f a, b, c;

            a = scene->vertex_data[triangle.indices.v0_id-1];
            b = scene->vertex_data[triangle.indices.v1_id-1];
            c = scene->vertex_data[triangle.indices.v2_id-1];

            hitRecord->material_id = triangle.material_id;

            hitRecord->p = e + d * data.t;

            hitRecord->n = triangle_normals[i];

            t_min = data.t;
            flag = true;
        }
    }
    // loop through the meshes
    for(int i = 0; i < scene->meshes.size(); i++){
        parser::Mesh mesh = scene->meshes[i];

        for(int j = 0; j < mesh.faces.size(); j++){
            parser::Face face = mesh.faces[j];

            parser::tri_intersect_data data = intersect_triangle(ray, &face, scene);

            if(data.hit && data.t < t_min){
                parser::Vec3f a, b, c;
                
                a = scene->vertex_data[face.v0_id-1];
                b = scene->vertex_data[face.v1_id-1];
                c = scene->vertex_data[face.v2_id-1];

                hitRecord->material_id = mesh.material_id;

                hitRecord->p = e + d * data.t;

                hitRecord->n = parser::Vec3f::cross(c - b, a - b).normalize();

                t_min = data.t;
                flag = true;
            }
        }
    }
    return flag;
}

parser::Vec3f applyShading(parser::Ray *ray, parser::hitRecord *hitRecord, parser::Scene *scene){
    parser::Material material;
    parser::Vec3f color;
    
    material = scene->materials[hitRecord->material_id-1];
    color = material.ambient * scene->ambient_light;

    // Mirror reflection
    if (material.is_mirror) {
        float cosTheta;
        parser::Ray reflectionRay;
        // w_r = -w_o + 2*n*(n . w_o)
        parser::Vec3f w_o = (ray->o - hitRecord->p).normalize();
        cosTheta = hitRecord->n.dot(w_o);
        parser::Vec3f reflection_dir = -w_o + hitRecord->n*cosTheta*2;
        reflectionRay.depth += ray->depth + 1;
        reflectionRay.d = reflection_dir.normalize();
        reflectionRay.o = hitRecord->p + hitRecord->n * scene->shadow_ray_epsilon;
        color += computeColor(&reflectionRay, scene) * material.mirror;
    }

    for(int i = 0; i < scene->point_lights.size(); i++){
        float cosTheta, distance;
        parser::PointLight point_light;
        parser::Vec3f light_d;
        
        point_light = scene->point_lights[i];

        distance = parser::Vec3f::distance(point_light.position, hitRecord->p);
        light_d = (point_light.position - hitRecord->p).normalize();
        // check if hit point is in shadow
        parser::Ray shadow_ray;
        shadow_ray.d = light_d;
        shadow_ray.o = hitRecord->p + hitRecord->n * scene->shadow_ray_epsilon;
        
        // intersect all objects and see if any of them is closer to 
        // shadow ray origin than the point light
        parser::hitRecord shadowHit;
        if(closestHit(&shadow_ray, &shadowHit, scene)){
            parser::Vec3f shadowHitPoint = shadowHit.p;
            
            float shadow_object_distance = parser::Vec3f::distance(shadowHitPoint, hitRecord->p);

            if(shadow_object_distance < distance){
                continue;
            }
        }
        

        cosTheta = light_d.dot(hitRecord->n);
        if(cosTheta > 0.0f){
            // diffuse shading
            float cosAlpha;
            parser::Vec3f diffuse_shading, E;
            
            E = point_light.intensity / (distance * distance);

            diffuse_shading = E * material.diffuse * cosTheta;

            color += diffuse_shading;

            parser::Vec3f k_s, h, w_o;

            w_o = (ray->o - hitRecord->p).normalize();

            h = (light_d + w_o) / (light_d + w_o).length();

            cosAlpha = std::max(0.0f, hitRecord->n.dot(h));

            float phong_term = (float) pow(cosAlpha, material.phong_exponent);

            parser::Vec3f specular_shading = E * material.specular * phong_term;

            color += specular_shading;
        }
    }

    return color;
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

parser::Ray computeRay(int i, int j, parser::Camera *camera, parser::Scene *scene){
    parser::Vec3f u = parser::Vec3f::cross(camera->up, -camera->gaze).normalize();
    parser::Vec3f v = camera->up.normalize();
    parser::Vec3f w = -camera->gaze.normalize();

    parser::Vec4f near_plane = camera->near_plane;
    float l = near_plane.x;
    float r = near_plane.y;
    float b = near_plane.z;
    float t = near_plane.w;

    // s_u = (i+0.5) * (r-l)/n_x
    // s_v = (j+0.5) * (t-b)/n_y
    float s_u = (i + 0.5f) * (r - l)/camera->image_width;
    float s_v = (j + 0.5f) * (t - b)/camera->image_height;

    // m = e - w*distance
    // q = m + l*u + t*v
    parser::Vec3f m = camera->position + camera->gaze * camera->near_distance;
    parser::Vec3f q = m + u * l + v * t;

    // s = q + s_u * u - s_v * v (pixel ij world)
    parser::Vec3f s = q + u * s_u - v * s_v;

    parser::Ray ray;
    ray.o = camera->position;
    ray.d = (s - camera->position);
    ray.depth = 0;

    return ray;
}

void *singleThread(void* arg) {
    parser::ThreadData* data = static_cast<parser::ThreadData*>(arg);
    parser::Scene* scene = data->scene;
    parser::Camera* camera = data->camera;
    unsigned char* image = data->image;

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
        parser::Triangle& triangle = triangles[i];
        parser::Vec3f a, b, c;
        
        a = vertices[triangle.indices.v0_id-1];
        b = vertices[triangle.indices.v1_id-1];
        c = vertices[triangle.indices.v2_id-1];

        parser::Vec3f normal = parser::Vec3f::cross(c - b, a - b).normalize();
        
        triangle_normals.push_back(normal);
    }

    int num_threads = 256;
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

            if (pthread_create(&threads[j], nullptr, singleThread, (void*)&threadData[j])) {
                printf("error at thread creation\n");
                return 1;
            }
        }
        for (int j = 0; j < num_threads; j++) {
            pthread_join(threads[j], nullptr);
        }
        write_ppm(camera.image_name.c_str(), image, width, height);
        delete[] image;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    return 0;
}
