#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include "tinyxml2.h"
#include "Triangle.h"
#include "Helpers.h"
#include "Scene.h"
#include "Line.h"
#include <iostream>

using namespace tinyxml2;
using namespace std;

int convertToZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

Matrix4 get_translation_matrix(Translation *translation){
	double tx = translation->tx;
	double ty = translation->ty;
	double tz = translation->tz;
	double T[4][4] = {
		{1, 0, 0, tx},
		{0, 1, 0, ty},
		{0, 0, 1, tz},
		{0, 0, 0,  1}
	};
	return T;
}

Matrix4 get_scaling_matrix(Scaling *scaling){
	double sx = scaling->sx;
	double sy = scaling->sy;
	double sz = scaling->sz;
	double S[4][4] = {
		{sx,  0,  0, 0},
		{ 0, sy,  0, 0},
		{ 0,  0, sz, 0},
		{ 0,  0,  0, 1}
	};
	return S;
}

Matrix4 get_rotation_matrix(Rotation *r) {
    Vec3 u(r->ux, r->uy, r->uz, -1);
    Vec3 v = (min({abs(r->ux), abs(r->uy), abs(r->uz)}) == abs(r->ux)) ?
             Vec3(0, -r->uz, r->uy, -1) :
             (min({abs(r->ux), abs(r->uy), abs(r->uz)}) == abs(r->uy)) ?
             Vec3(-r->uz, 0, r->ux, -1) : Vec3(-r->uy, r->ux, 0, -1);

    Vec3 w = normalizeVec3(crossProductVec3(u, v));
    v = normalizeVec3(v);

    double m[4][4] = {
		{u.x, u.y, u.z, 0},
		{v.x, v.y, v.z, 0},
		{w.x, w.y, w.z, 0},
		{  0,   0,   0, 1}
		};
    double inverse[4][4] = {
		{u.x, v.x, w.x, 0},
		{u.y, v.y, w.y, 0},
		{u.z, v.z, w.z, 0},
		{  0,   0,   0, 1}
		};
    double angle = r->angle * M_PI / 180;
    double rot[4][4] = {
		{1, 		 0, 		  0, 0},
		{0, cos(angle), -sin(angle), 0},
		{0, sin(angle),  cos(angle), 0},
		{0, 		 0, 		  0, 1}
		};
    
    Matrix4 rot1 = multiplyMatrixWithMatrix(rot, m);
    Matrix4 R = multiplyMatrixWithMatrix(inverse, rot1);

    return R;
}

Matrix4 get_cam_translation_matrix(Camera *camera){
	double e_x, e_y, e_z;
	e_x = camera->position.x;
	e_y = camera->position.y;
	e_z = camera->position.z;

	double translate_camera[4][4] = {
		{1, 0, 0, -e_x},
		{0, 1, 0, -e_y},
		{0, 0, 1, -e_z},
		{0, 0, 0,    1}
	};

	// ROTATE UVW TO ALIGN IT WITH XYZ
	double rotate_camera[4][4] = {
		{camera->u.x, camera->u.y, camera->u.z, 0},
		{camera->v.x, camera->v.y, camera->v.z, 0},
		{camera->w.x, camera->w.y, camera->w.z, 0},
		{  		   0,   		0,   		 0, 1}
	};

	// CALCULATE THE COMPOSITE CAMERA TRANSFORMATION MATRIX
	Matrix4 M_cam = multiplyMatrixWithMatrix(rotate_camera, translate_camera);
	return M_cam;
}

Matrix4 get_perspective_proj_matrix(Camera *camera){
	double r, l, t, b, n, f;
	r = camera->right;
	l = camera->left;
	t = camera->top;
	b = camera->bottom;
	n = camera->near;
	f = camera->far;
	if(camera->projectionType == PERSPECTIVE_PROJECTION){
		double M_pers[4][4] = {
			{(2 * n) / (r - l), 				0,    (r + l) / (r - l), 						0},
			{				 0, (2 * n) / (t - b),    (t + b) / (t - b), 						0},
			{				 0, 				0, -((f + n) / (f - n)), -((2 * f * n) / (f - n))},
			{				 0, 				0, 					 -1, 						0}
		};
		return M_pers;
	}
	else{
		double M_orth[4][4] = {
			{2 / (r - l), 			0, 			  0,  -((r + l) / (r - l))},
			{		   0, 2 / (t - b), 			  0,  -((t + b) / (t - b))},
			{		   0, 			0, -2 / (f - n),  -((f + n) / (f - n))},
			{		   0, 			0, 			  0, 					 1}
		};
		return M_orth;
	}
}

Matrix4 get_viewport_matrix(Camera *camera){
	// CALCULATE VIEWPORT TRANSFORMATION MATRIX
	int n_x = camera->horRes, n_y = camera->verRes;
	double M_vp[4][4] = {
		{n_x / 2.0, 	    0,   0, (n_x - 1) / 2.0},
		{	     0, n_y / 2.0,   0, (n_y - 1) / 2.0},
		{	     0, 		0, 1/2, 		    1/2},
		{	     0, 		0,   0, 		      1}
	};
	return M_vp;
}

Matrix4 get_viewport_matrix_with_depth(Camera *camera) {
    int n_x = camera->horRes, n_y = camera->verRes;
    double depthMin = 0; // min depth
    double depthMax = 1; // max depth

    double M_vp[4][4] = {
        {n_x / 2.0,         0,          				 0, 			(n_x - 1) / 2.0},
        {        0, n_y / 2.0,          				 0, 			(n_y - 1) / 2.0},
        {        0,         0, (depthMax - depthMin) / 2.0, (depthMax + depthMin) / 2.0},
        {        0,         0,          				 0,               			  1}
    };
    return M_vp;
}

bool isLineVisible(double denominator, double numerator, double &tEntry, double &tLeave){
    double t = numerator / denominator;
    if(denominator > 0) {
        if(t > tLeave) return false;
        tEntry = std::max(t, tEntry);
    }
	else if(denominator < 0){
        if(t < tEntry) return false;
        tLeave = std::min(t, tLeave);
    }
	else if(numerator > 0) return false;
    return true;
}

// Liang-Barsky line clipping algorithm
bool clipLine(Line &line) {
    Vec4 &start = line.v0, &end = line.v1;
    Color &colStart = line.c0, &colEnd = line.c1;
    double dx = end.x - start.x, dy = end.y - start.y, dz = end.z - start.z;
    Color dCol = subtractColor(colEnd, colStart);

    double x_min = -1, x_max = 1, y_min = -1, y_max = 1, z_min = -1, z_max = 1;
    double tEntry = 0, tLeave = 1;
    if(isLineVisible(dx, x_min - start.x, tEntry, tLeave) && isLineVisible(-dx, start.x - x_max, tEntry, tLeave) &&
       isLineVisible(dy, y_min - start.y, tEntry, tLeave) && isLineVisible(-dy, start.y - y_max, tEntry, tLeave) &&
       isLineVisible(dz, z_min - start.z, tEntry, tLeave) && isLineVisible(-dz, start.z - z_max, tEntry, tLeave)) {
		
        if(tLeave < 1) {
            end.x = start.x + dx * tLeave;
            end.y = start.y + dy * tLeave;
            end.z = start.z + dz * tLeave;
            colEnd = addColor(colStart, multiplyColor(dCol, tLeave));
        }
        if(tEntry > 0) {
            start.x = start.x + dx * tEntry;
            start.y = start.y + dy * tEntry;
            start.z = start.z + dz * tEntry;
            colStart = addColor(colStart, multiplyColor(dCol, tEntry));
        }
        return true;
    }
    return false;
}

void rasterizeHelper(vector<vector<Color>> &image, int x, int y, Color c) {
	// round(c)
	int r, g, b;
	Color roundedColor = roundColor(c);
	r = roundedColor.r; g = roundedColor.g; b = roundedColor.b;

	// make color values between 0 and 255
	r = convertToZeroAnd255(r);
	g = convertToZeroAnd255(g);
	b = convertToZeroAnd255(b);

	Color color = Color(r, g, b);

	// image[x][y] = color
	x = (x < 0) ? 0 : x;
	y = (y < 0) ? 0 : y;
	image[x][y] = color;
}

void rasterizeLine(Line &line, vector<vector<Color>> &image) {
    Vec4 p0 = line.v0, p1 = line.v1;
    Color col0 = line.c0, col1 = line.c1;
    double x0 = p0.x, y0 = p0.y, x1 = p1.x, y1 = p1.y;
    double dx = x1 - x0, dy = y1 - y0;
    double absDx = abs(dx), absDy = abs(dy);

    auto swap = [](auto &a, auto &b) {
        auto temp = a;
        a = b;
        b = temp;
    };

    bool steep = absDy > absDx;
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
        swap(absDx, absDy);
    }
    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
        swap(col0, col1);
    }

    double deltaErr = (absDy / absDx);
    double err = 0.0;
    int yStep = (y0 < y1) ? 1 : -1;
    int y = y0;
    Color col = col0, dCol = divideColor(subtractColor(col1, col0), absDx);

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            rasterizeHelper(image, y, x, col);
        } else {
            rasterizeHelper(image, x, y, col);
        }
        err += deltaErr;
        if (err >= 0.5) {
            y += yStep;
            err -= 1.0;
        }
        col = addColor(col, dCol);
    }
}

double edgeFunction(const Vec4 &a, const Vec4 &b, const Vec4 &c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}

Color interpolateColor(double alpha, double beta, double gamma, 
Color c0, Color c1, Color c2) {
    double r = alpha * c0.r + beta * c1.r + gamma * c2.r;
    double g = alpha * c0.g + beta * c1.g + gamma * c2.g;
    double b = alpha * c0.b + beta * c1.b + gamma * c2.b;
    return Color(round(r), round(g), round(b));
}

void rasterizeTriangle(Vec4 v0, Vec4 v1, Vec4 v2, Color c0, Color c1, Color c2, 
vector<vector<Color>> &image, vector<vector<double>> &depthBuffer, Camera *camera) {
    // find bounding box
    int minX = min({v0.x, v1.x, v2.x});
	minX = (minX < 0) ? 0 : minX;
	minX = (minX > camera->horRes - 1) ? camera->horRes-1 : minX;
    int maxX = max({v0.x, v1.x, v2.x});
	maxX = (maxX < 0) ? 0 : maxX;
	maxX = (maxX > camera->horRes - 1) ? camera->horRes-1 : maxX;
    int minY = min({v0.y, v1.y, v2.y});
	minY = (minY < 0) ? 0 : minY;
	minY = (minY > camera->verRes - 1) ? camera->verRes-1 : minY;
    int maxY = max({v0.y, v1.y, v2.y});
	maxY = (maxY < 0) ? 0 : maxY;
	maxY = (maxY > camera->verRes - 1) ? camera->verRes-1 : maxY;

    // edge function denominators
    double f12_v0 = edgeFunction(v1, v2, v0);
    double f20_v1 = edgeFunction(v2, v0, v1);
    double f01_v2 = edgeFunction(v0, v1, v2);

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            Vec4 p = Vec4(x, y, 0, 0);

            // calculate barycentric coordinates
            double alpha = edgeFunction(v1, v2, p) / f12_v0;
            double beta  = edgeFunction(v2, v0, p) / f20_v1;
            double gamma = edgeFunction(v0, v1, p) / f01_v2;

            // check if inside
            if (alpha >= 0 && beta >= 0 && gamma >= 0) {
				// interpolate depth
				double depth = alpha * v0.z + beta * v1.z + gamma * v2.z;
				// check if there is anything closer than the current object
				if(depth < depthBuffer[x][y]){
					// interpolate color
					Color c = interpolateColor(alpha, beta, gamma, c0, c1, c2);

					// draw the color
					rasterizeHelper(image, x, y, c);

					// update depth buffer
					depthBuffer[x][y] = depth;
				}
            }
        }
    }
}

void perform_pers_divide_on_line(Line &line){
	line.v0.x /= line.v0.t;
	line.v0.y /= line.v0.t;
	line.v0.z /= line.v0.t;
	line.v0.t /= line.v0.t;
	
	line.v1.x /= line.v1.t;
	line.v1.y /= line.v1.t;
	line.v1.z /= line.v1.t;
	line.v1.t /= line.v1.t;
}

void perform_pers_divide_on_v(Vec4 &v){
	v.x /= v.t;
	v.y /= v.t;
	v.z /= v.t;
	v.t /= v.t;
}

void perform_viewport_transformation_on_line(Matrix4 M_vp, Line &line){
	line.v0 = multiplyMatrixWithVec4(M_vp, line.v0);
	line.v1 = multiplyMatrixWithVec4(M_vp, line.v1);
}

void perform_viewport_transformation_on_v(Matrix4 M_vp, Vec4 &v){
	v = multiplyMatrixWithVec4(M_vp, v);
}

/*
	Transformations, clipping, culling, rasterization are done here.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{
	vector<vector<Color> > image = this->image;
	vector<vector<double> > depth = this->depth;
	vector<Vec3 *> vertices = this->vertices;
	vector<Color *> colorsOfVertices = this->colorsOfVertices;
	vector<Scaling *> scalings = this->scalings;
	vector<Rotation *> rotations = this->rotations;
	vector<Translation *> translations = this->translations;
	vector<Mesh *> meshes = this->meshes;
	
	for(int i = 0; i < meshes.size(); i++){
		int j = 0;
		Mesh *mesh = meshes[i];
		vector<Matrix4> transformations;
		// CALCULATE COMPOSITE MODELING TRANSFORMATION MATRIX
		Matrix4 M_model_matrix = getIdentityMatrix();
		for(; j < mesh->numberOfTransformations; j++){
			int transformationId = mesh->transformationIds[j] - 1;
			if(mesh->transformationTypes[j] == 't'){
				// TRANSLATION
				Matrix4 translation_matrix = get_translation_matrix(translations[transformationId]);
				M_model_matrix = multiplyMatrixWithMatrix(translation_matrix, M_model_matrix);
			}
			else if(mesh->transformationTypes[j] == 's'){
				// SCALING
				Matrix4 scaling_matrix = get_scaling_matrix(scalings[transformationId]);
				M_model_matrix = multiplyMatrixWithMatrix(scaling_matrix, M_model_matrix);
			}
			else{
				// ROTATION
				Matrix4 rotation_matrix = get_rotation_matrix(rotations[transformationId]);
				M_model_matrix = multiplyMatrixWithMatrix(rotation_matrix, M_model_matrix);
			}
		}
		// APPLY VIWEING TRANSFORMATIONS
		Matrix4 M_cam_translation = get_cam_translation_matrix(camera);
		// PERFORM PERSPECTIVE OR ORTHOGRAPHIC PROJECTION
		Matrix4 M_proj_matrix = get_perspective_proj_matrix(camera);
		// CALCULATE THE COMPOSITE MATRIX
		Matrix4 M_composite = multiplyMatrixWithMatrix(M_proj_matrix, multiplyMatrixWithMatrix(M_cam_translation, M_model_matrix));
		for (j = 0; j < mesh->numberOfTriangles; j++) {
			Triangle triangle = mesh->triangles[j];
			Vec3 *v0, *v1, *v2;
			v0 = this->vertices[triangle.vertexIds[0]-1];
			v1 = this->vertices[triangle.vertexIds[1]-1];
			v2 = this->vertices[triangle.vertexIds[2]-1];

			Vec4 vertex0, vertex1, vertex2;
			vertex0 = Vec4(v0->x, v0->y, v0->z, 1);
			vertex1 = Vec4(v1->x, v1->y, v1->z, 1);
			vertex2 = Vec4(v2->x, v2->y, v2->z, 1);
			vertex0.colorId = v0->colorId;
			vertex1.colorId = v1->colorId;
			vertex2.colorId = v2->colorId;
			vertex0 = multiplyMatrixWithVec4(M_composite, vertex0);
			vertex1 = multiplyMatrixWithVec4(M_composite, vertex1);
			vertex2 = multiplyMatrixWithVec4(M_composite, vertex2);
			// check if the triangle facing away from the camera
			if(this->cullingEnabled){
				// convert to vec3 for vector operations
				Vec3 proj_v0 = Vec3(vertex0.x, vertex0.y, vertex0.z);
				Vec3 proj_v1 = Vec3(vertex1.x, vertex1.y, vertex1.z);
				Vec3 proj_v2 = Vec3(vertex2.x, vertex2.y, vertex2.z);
				// calculate normal of triangle
				Vec3 normal = subtractVec3(proj_v1, proj_v0);
				normal = crossProductVec3(normal, subtractVec3(proj_v2, proj_v0));
				normal = normalizeVec3(normal);
				// calculate dot product
				double dotProduct = dotProductVec3(normal, proj_v0);
				if (dotProduct < 0) { // facing away from camera
					continue; // no need to rasterize the lines of this triangle
				}
			}

			Color c0, c1, c2;
			c0 = *(this->colorsOfVertices[vertex0.colorId - 1]);
			c1 = *(this->colorsOfVertices[vertex1.colorId - 1]);
			c2 = *(this->colorsOfVertices[vertex2.colorId - 1]);
			if(!(mesh->type == WIREFRAME_MESH)){
				// solid mode, rasterize the triangle
				// no clipping here

				// if the projection type is perspective, apply perspective divide
				if(camera->projectionType){
					perform_pers_divide_on_v(vertex0);
					perform_pers_divide_on_v(vertex1);
					perform_pers_divide_on_v(vertex2);
				}
				// apply viewport matrix
				Matrix4 M_vp = get_viewport_matrix_with_depth(camera);
				perform_viewport_transformation_on_v(M_vp, vertex0);
				perform_viewport_transformation_on_v(M_vp, vertex1);
				perform_viewport_transformation_on_v(M_vp, vertex2);
				// rasterize the triangle
				rasterizeTriangle(vertex0, vertex1, vertex2, c0, c1, c2, this->image, this->depth, camera);
			}
			else{
				// clip the lines of the triangle in wireframe mode
				Line line1 = Line(vertex0, vertex1, c0, c1);
				Line line2 = Line(vertex1, vertex2, c1, c2);
				Line line3 = Line(vertex2, vertex0, c2, c0);
				// if the projection type is perspective, apply perspective divide
				if(camera->projectionType){
					perform_pers_divide_on_line(line1);
					perform_pers_divide_on_line(line2);
					perform_pers_divide_on_line(line3);
				}
				bool visible1 = clipLine(line1);
				bool visible2 = clipLine(line2);
				bool visible3 = clipLine(line3);
				// after perspective divide, apply viewport transformation
				Matrix4 M_vp = get_viewport_matrix_with_depth(camera);
				perform_viewport_transformation_on_line(M_vp, line1);
				perform_viewport_transformation_on_line(M_vp, line2);
				perform_viewport_transformation_on_line(M_vp, line3);
				// perform line rasterization and color interpolation
				// use the midpoint algorithm for line rasterization
				if(visible1) rasterizeLine(line1, this->image);
				if(visible2) rasterizeLine(line2, this->image);
				if(visible3) rasterizeLine(line3, this->image);
			}
		}
	}
}

/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *xmlElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *rootNode = xmlDoc.FirstChild();

	// read background color
	xmlElement = rootNode->FirstChildElement("BackgroundColor");
	str = xmlElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	xmlElement = rootNode->FirstChildElement("Culling");
	if (xmlElement != NULL)
	{
		str = xmlElement->GetText();

		if (strcmp(str, "enabled") == 0)
		{
			this->cullingEnabled = true;
		}
		else
		{
			this->cullingEnabled = false;
		}
	}

	// read cameras
	xmlElement = rootNode->FirstChildElement("Cameras");
	XMLElement *camElement = xmlElement->FirstChildElement("Camera");
	XMLElement *camFieldElement;
	while (camElement != NULL)
	{
		Camera *camera = new Camera();

		camElement->QueryIntAttribute("id", &camera->cameraId);

		// read projection type
		str = camElement->Attribute("type");

		if (strcmp(str, "orthographic") == 0)
		{
			camera->projectionType = ORTOGRAPHIC_PROJECTION;
		}
		else
		{
			camera->projectionType = PERSPECTIVE_PROJECTION;
		}

		camFieldElement = camElement->FirstChildElement("Position");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->position.x, &camera->position.y, &camera->position.z);

		camFieldElement = camElement->FirstChildElement("Gaze");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->gaze.x, &camera->gaze.y, &camera->gaze.z);

		camFieldElement = camElement->FirstChildElement("Up");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf", &camera->v.x, &camera->v.y, &camera->v.z);

		camera->gaze = normalizeVec3(camera->gaze);
		camera->u = crossProductVec3(camera->gaze, camera->v);
		camera->u = normalizeVec3(camera->u);

		camera->w = inverseVec3(camera->gaze);
		camera->v = crossProductVec3(camera->u, camera->gaze);
		camera->v = normalizeVec3(camera->v);

		camFieldElement = camElement->FirstChildElement("ImagePlane");
		str = camFieldElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &camera->left, &camera->right, &camera->bottom, &camera->top,
			   &camera->near, &camera->far, &camera->horRes, &camera->verRes);

		camFieldElement = camElement->FirstChildElement("OutputName");
		str = camFieldElement->GetText();
		camera->outputFilename = string(str);

		this->cameras.push_back(camera);

		camElement = camElement->NextSiblingElement("Camera");
	}

	// read vertices
	xmlElement = rootNode->FirstChildElement("Vertices");
	XMLElement *vertexElement = xmlElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (vertexElement != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = vertexElement->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = vertexElement->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		this->vertices.push_back(vertex);
		this->colorsOfVertices.push_back(color);

		vertexElement = vertexElement->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	xmlElement = rootNode->FirstChildElement("Translations");
	XMLElement *translationElement = xmlElement->FirstChildElement("Translation");
	while (translationElement != NULL)
	{
		Translation *translation = new Translation();

		translationElement->QueryIntAttribute("id", &translation->translationId);

		str = translationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		this->translations.push_back(translation);

		translationElement = translationElement->NextSiblingElement("Translation");
	}

	// read scalings
	xmlElement = rootNode->FirstChildElement("Scalings");
	XMLElement *scalingElement = xmlElement->FirstChildElement("Scaling");
	while (scalingElement != NULL)
	{
		Scaling *scaling = new Scaling();

		scalingElement->QueryIntAttribute("id", &scaling->scalingId);
		str = scalingElement->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		this->scalings.push_back(scaling);

		scalingElement = scalingElement->NextSiblingElement("Scaling");
	}

	// read rotations
	xmlElement = rootNode->FirstChildElement("Rotations");
	XMLElement *rotationElement = xmlElement->FirstChildElement("Rotation");
	while (rotationElement != NULL)
	{
		Rotation *rotation = new Rotation();

		rotationElement->QueryIntAttribute("id", &rotation->rotationId);
		str = rotationElement->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		this->rotations.push_back(rotation);

		rotationElement = rotationElement->NextSiblingElement("Rotation");
	}

	// read meshes
	xmlElement = rootNode->FirstChildElement("Meshes");

	XMLElement *meshElement = xmlElement->FirstChildElement("Mesh");
	while (meshElement != NULL)
	{
		Mesh *mesh = new Mesh();

		meshElement->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = meshElement->Attribute("type");

		if (strcmp(str, "wireframe") == 0)
		{
			mesh->type = WIREFRAME_MESH;
		}
		else
		{
			mesh->type = SOLID_MESH;
		}

		// read mesh transformations
		XMLElement *meshTransformationsElement = meshElement->FirstChildElement("Transformations");
		XMLElement *meshTransformationElement = meshTransformationsElement->FirstChildElement("Transformation");

		while (meshTransformationElement != NULL)
		{
			char transformationType;
			int transformationId;

			str = meshTransformationElement->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			meshTransformationElement = meshTransformationElement->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *cloneStr;
		int v1, v2, v3;
		XMLElement *meshFacesElement = meshElement->FirstChildElement("Faces");
		str = meshFacesElement->GetText();
		cloneStr = strdup(str);

		row = strtok(cloneStr, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);

			if (result != EOF)
			{
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		this->meshes.push_back(mesh);

		meshElement = meshElement->NextSiblingElement("Mesh");
	}
}

void Scene::assignColorToPixel(int i, int j, Color c)
{
	this->image[i][j].r = c.r;
	this->image[i][j].g = c.g;
	this->image[i][j].b = c.b;
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;
			vector<double> rowOfDepths;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
				rowOfDepths.push_back(1.01);
			}

			this->image.push_back(rowOfColors);
			this->depth.push_back(rowOfDepths);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				assignColorToPixel(i, j, this->backgroundColor);
				this->depth[i][j] = 1.01;
				this->depth[i][j] = 1.01;
				this->depth[i][j] = 1.01;
			}
		}
	}
}

int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFilename.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFilename << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
*/
void Scene::convertPPMToPNG(string ppmFileName)
{
	string command;

	// TODO: Change implementation if necessary.
	command = "convert " + ppmFileName + " " + ppmFileName + ".png";
	system(command.c_str());
}