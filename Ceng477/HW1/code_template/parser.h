#ifndef __HW1__PARSER__
#define __HW1__PARSER__
#include <cmath>
#include <string>
#include <vector>

namespace parser
{
    //Notice that all the structures are as simple as possible
    //so that you are not enforced to adopt any style or design.
    struct Vec3f
    {
        float x, y, z;
        int depth;

        Vec3f(): x(0), y(0), z(0), depth(0) {}
        Vec3f(float x, float y, float z): x(x), y(y), z(z), depth(depth) {}
        Vec3f operator*(float scalar) const{
            return Vec3f(x*scalar, y*scalar, z*scalar);
        }
        Vec3f operator+(Vec3f vec2) const{
            return Vec3f(x+vec2.x, y+vec2.y, z+vec2.z);
        }
        Vec3f operator-(Vec3f vec2) const{
            return Vec3f(x-vec2.x, y-vec2.y, z-vec2.z);
        }
        Vec3f operator-() const {
            return Vec3f(-x, -y, -z);
        }
        Vec3f& operator+=(const Vec3f& vec2) {
            x += vec2.x;
            y += vec2.y;
            z += vec2.z;
            return *this;
        }
        static Vec3f cross(const Vec3f& a, const Vec3f& b) {
            return Vec3f(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }
        float length() const{
            return sqrt(x*x + y*y + z*z);
        }
        float dot(const Vec3f& vec2) const {
            return x * vec2.x + y * vec2.y + z * vec2.z;
        }
        Vec3f convert_to_unit() const{
            float len = length();
            if(len == 0) return Vec3f();
            else{
                return Vec3f(x/len, y/len, z/len);
            }
        }
    };

    struct hitRecord{
        // information about the hit point
        int material_id;
        Vec3f n, p; // surface normal, intersection point

        hitRecord(): material_id(0), 
        n(Vec3f()), p(Vec3f()) {}
    };

    struct Vec3i
    {
        int x, y, z;

        Vec3i(): x(0), y(0), z(0) {}

        Vec3i(int x, int y, int z): x(x), y(y), z(z) {}

    };

    struct Vec4f
    {
        float x, y, z, w;
    };

    struct Camera
    {
        Vec3f position;
        Vec3f gaze;
        Vec3f up;
        Vec4f near_plane;
        float near_distance;
        int image_width, image_height;
        std::string image_name;
    };

    struct PointLight
    {
        Vec3f position;
        Vec3f intensity;
    };

    struct Material
    {
        bool is_mirror;
        Vec3f ambient;
        Vec3f diffuse;
        Vec3f specular;
        Vec3f mirror;
        float phong_exponent;
    };

    struct Face
    {
        int v0_id;
        int v1_id;
        int v2_id;
    };

    struct Mesh
    {
        int material_id;
        std::vector<Face> faces;
    };

    struct triangle_ray_intersection_data{
        float beta, gamma, t;

        triangle_ray_intersection_data(): beta(0), gamma(0), t(0) {}

        triangle_ray_intersection_data(float beta,
        float gamma, float t): beta(beta), gamma(gamma), t(t) {}
    };

    struct Triangle
    {
        int material_id;
        Face indices;
    };

    struct sphere_ray_intersection_data{
        float t;

        sphere_ray_intersection_data(): t(0) {}

        sphere_ray_intersection_data(float t): t(t) {}
    };

    struct Sphere
    {
        int material_id;
        int center_vertex_id;
        float radius;
    };

    struct Scene
    {
        //Data
        Vec3i background_color;
        float shadow_ray_epsilon;
        int max_recursion_depth;
        std::vector<Camera> cameras;
        Vec3f ambient_light;
        std::vector<PointLight> point_lights;
        std::vector<Material> materials;
        std::vector<Vec3f> vertex_data;
        std::vector<Mesh> meshes;
        std::vector<Triangle> triangles;
        std::vector<Sphere> spheres;

        //Functions
        void loadFromXml(const std::string &filepath);
    };
}

#endif
