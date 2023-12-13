#include <iostream>
#include <cmath>
#include "Helpers.h"

/*
 * Calculate cross product of vec3 a, vec3 b and return resulting vec3.
 */
Vec3 crossProductVec3(Vec3 a, Vec3 b)
{
    return Vec3(a.y * b.z - b.y * a.z, b.x * a.z - a.x * b.z, a.x * b.y - b.x * a.y);
}

/*
 * Calculate dot product of vec3 a, vec3 b and return resulting value.
 */
double dotProductVec3(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/*
 * Find length (|v|) of vec3 v.
 */
double magnitudeOfVec3(Vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

/*
 * Normalize the vec3 to make it unit vec3.
 */
Vec3 normalizeVec3(Vec3 v)
{
    double d = magnitudeOfVec3(v);
    return Vec3(v.x / d, v.y / d, v.z / d);
}

/*
 * Return -v (inverse of vec3 v)
 */
Vec3 inverseVec3(Vec3 v)
{
    return Vec3(-v.x, -v.y, -v.z);
}

/*
 * Add vec3 a to vec3 b and return resulting vec3 (a+b).
 */
Vec3 addVec3(Vec3 a, Vec3 b)
{
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

/*
 * Subtract vec3 b from vec3 a and return resulting vec3 (a-b).
 */
Vec3 subtractVec3(Vec3 a, Vec3 b)
{
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/*
 * Multiply each element of vec3 with scalar.
 */
Vec3 multiplyVec3WithScalar(Vec3 v, double c)
{
    return Vec3(v.x * c, v.y * c, v.z * c);
}

/*
 * Prints elements in a vec3. Can be used for debugging purposes.
 */
void printVec3(Vec3 v)
{
    std::cout << "(" << v.x << "," << v.y << "," << v.z << ")" << std::endl;
}

/*
 * Check whether vec3 a and vec3 b are equal.
 * In case of equality, returns 1.
 * Otherwise, returns 0.
 */
int areEqualVec3(Vec3 a, Vec3 b)
{

    /* if x difference, y difference and z difference is smaller than threshold, then they are equal */
    if ((ABS((a.x - b.x)) < EPSILON) && (ABS((a.y - b.y)) < EPSILON) && (ABS((a.z - b.z)) < EPSILON))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
 * Returns an identity matrix (values on the diagonal are 1, others are 0).
 */
Matrix4 getIdentityMatrix()
{
    Matrix4 result;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (i == j)
            {
                result.values[i][j] = 1.0;
            }
            else
            {
                result.values[i][j] = 0.0;
            }
        }
    }

    return result;
}

/*
 * Multiply matrices m1 (Matrix4) and m2 (Matrix4) and return the result matrix r (Matrix4).
 */
Matrix4 multiplyMatrixWithMatrix(Matrix4 m1, Matrix4 m2)
{
    Matrix4 result;
    double total;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            total = 0;
            for (int k = 0; k < 4; k++)
            {
                total += m1.values[i][k] * m2.values[k][j];
            }

            result.values[i][j] = total;
        }
    }

    return result;
}

/*
 * Multiply matrix m (Matrix4) with vector v (vec4) and store the result in vector r (vec4).
 */
Vec4 multiplyMatrixWithVec4(Matrix4 m, Vec4 v)
{
    double values[4];
    double total;

    for (int i = 0; i < 4; i++)
    {
        total = 0;
        for (int j = 0; j < 4; j++)
        {
            total += m.values[i][j] * v.getNthComponent(j);
        }
        values[i] = total;
    }

    return Vec4(values[0], values[1], values[2], values[3], v.colorId);
}

// COLOR HELPERS

/*
* Subtract Color c2 from Color c1 and return the result as another color.
*/
Color subtractColor(Color c1, Color c2){
    double r = c1.r - c2.r;
    double g = c1.g - c2.g;
    double b = c1.b - c2.b;

    return Color(r, g, b);
}

/*
* Add Color c1 to Color c2 and return the result as another color.
*/
Color addColor(Color c1, Color c2){
    double r = c1.r + c2.r;
    double g = c1.g + c2.g;
    double b = c1.b + c2.b;

    return Color(r, g, b);
}

/*
* Divide Color c by double s and return the divided number.
*/
Color divideColor(Color c, double s){
    double r, g, b;
    r = c.r / s;
    g = c.g / s;
    b = c.b / s;
    return Color(r, g, b);
}

/*
* Round the given Color c and return the rounded color.
*/
Color roundColor(Color c){
    double r, g, b;
    r = round(c.r);
    g = round(c.g);
    b = round(c.b);
    return Color(r, g, b);
}

// CLIPPING HELPER

// Liang-Barsky line clipping algorithm
bool clipLine(double x0, double y0, double z0, double x1, double y1, double z1, double& tE, double& tL,
int horRes, int verRes, double n, double f) {
    double dx = x1 - x0;
    double dy = y1 - y0;
    double dz = z1 - z0;

    double x_min = 0.0, x_max = horRes, y_min = 0.0, y_max = verRes, z_min = n, z_max = f;

    double denominators[6] = {-dx, dx, -dy, dy, -dz, dz};
    double numerators[6] = {x0 - x_min, x_max - x0, y0 - y_min, y_max - y0, z0 - z_min, z_max - z0};

    for (int i = 0; i < 6; i++) {
        if (denominators[i] == 0 && numerators[i] < 0) return false;
        double t = numerators[i] / denominators[i];
        if (denominators[i] < 0) {
            if (t > tL) return false;
            else if (t > tE) tE = t;
        }
        else {
            if (t < tE) return false;
            else if (t < tL) tL = t;
        }
    }

    return true;
}