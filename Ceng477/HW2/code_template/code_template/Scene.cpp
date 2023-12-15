#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#include "tinyxml2.h"
#include "Triangle.h"
#include "Helpers.h"
#include "Scene.h"
#include "Line.h"

#include <iostream>

using namespace tinyxml2;
using namespace std;

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
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

Matrix4 get_rotation_matrix(Rotation *r){
	// ROTATION
	/*
	double ux = rotation->ux;
	double uy = rotation->uy;
	double uz = rotation->uz;
	double angle = rotation->angle;

	Vec3 v_projection = Vec3(ux, uy, uz);
	Vec3 u_projection = normalizeVec3(v_projection);
	double a = u_projection.x;
	double b = u_projection.y;
	double c = u_projection.z;
	double d = sqrt(b*b + c*c);

	// CALCULATE THE X-AXIS ALIGNMENT MATRIX
	double cos_alpha = c / d;
	double sin_alpha = b / d;

	double R_x_alpha[4][4] = {
		{1,         0, 		    0, 0},
		{0, cos_alpha, -sin_alpha, 0},
		{0, sin_alpha,  cos_alpha, 0},
		{0, 		0, 			0, 1}
	};
	double R_x_minus_alpha[4][4] = {
		{1,          0, 	     0, 0},
		{0,  cos_alpha,  sin_alpha, 0},
		{0, -sin_alpha,  cos_alpha, 0},
		{0, 		 0, 		 0, 1}
	};

	// CALCULATE THE Y-AXIS ALIGNMENT MATRIX
	double cos_beta = d;
	double sin_beta = a;

	double R_y_beta[4][4] = {
		{cos_beta, 0, -sin_beta, 0},
		{		0, 1, 		  0, 0},
		{sin_beta, 0,  cos_beta, 0},
		{		0, 0, 		  0, 1}
	};
	double R_y_minus_beta[4][4] = {
		{ cos_beta, 0, sin_beta, 0},
		{		 0, 1, 		  0, 0},
		{-sin_beta, 0, cos_beta, 0},
		{		 0, 0, 		  0, 1}
	};

	// CALCULATE THE ACTUAL ROTATION MATRIX
	double cos_angle = cos(angle);
	double sin_angle = sin(angle);

	double R_z_angle[4][4] = {
		{cos_angle, -sin_angle, 0, 0},
		{sin_angle,  cos_angle, 0, 0},
		{		 0, 		 0, 0, 0},
		{		 0, 		 0, 0, 1}
	};

	// CALCULATE THE COMPOSITE ROTATION MATRIX
	Matrix4 R;
	R = multiplyMatrixWithMatrix(R_x_minus_alpha, R_y_beta);
	R = multiplyMatrixWithMatrix(R, R_z_angle);
	R = multiplyMatrixWithMatrix(R, R_y_minus_beta);
	R = multiplyMatrixWithMatrix(R, R_x_alpha);

	return R;*/

	/* First find ONB uvw and construct M(mMatrix) */
	Vec3 u = Vec3(r->ux, r->uy, r->uz, -1), v, w;
	double minComp = std::min(std::min(abs(r->ux), abs(r->uy)), abs(r->uz));
	if (minComp == abs(r->ux))
		v = Vec3(0, -1 * r->uz, r->uy, -1);
	else if (minComp == abs(r->uy))
		v = Vec3(-1 * r->uz, 0, r->ux, -1);
	else if (minComp == abs(r->uz))
		v = Vec3(-1 * r->uy, r->ux, 0, -1);
	w = crossProductVec3(u, v);
	// Do not forget to normalize v and w
	v = normalizeVec3(v);
	w = normalizeVec3(w);
	double mMatrix[4][4] = {{u.x,u.y,u.z,0},
							{v.x,v.y,v.z,0},
							{w.x,w.y,w.z,0},
							{0,0,0,1}};
	double mMatrix_inverse[4][4] = {{u.x,v.x,w.x,0},
									{u.y,v.y,w.y,0},
									{u.z,v.z,w.z,0},
									{0,0,0,1}};
	/* rMatrix is rotation along X axis since now u is aligned with X*/
	double rMatrix[4][4] = {{1,0,0,0},
							{0,cos(r->angle * M_PI/180),(-1) * sin(r->angle * M_PI/180),0},
							{0,sin(r->angle * M_PI/180),cos(r->angle * M_PI/180),0},
							{0,0,0,1}};
	Matrix4 rot1 = multiplyMatrixWithMatrix(rMatrix, mMatrix);
	Matrix4 rotRes = multiplyMatrixWithMatrix(mMatrix_inverse, rot1);
	return rotRes;
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

bool visible(double den, double num, double &tE, double &tL){
	double t = num / den;
	if(den > 0){
		if(t > tL) return false;
		if(t > tE) tE = t;
	}
	else if(den < 0){
		if(t < tE) return false;
		if(t < tL) tL = t;
	}
	else if(num > 0) return false;
	return true;
}

// Liang-Barsky line clipping algorithm
bool clipLine(Line &line, Camera *camera) {
	Vec4 v0 = line.v0, v1 = line.v1;
	Color c0 = line.c0, c1 = line.c1;
    double dx = v1.x - v0.x;
    double dy = v1.y - v0.y;
    double dz = v1.z - v0.z;
	Color dc = subtractColor(c1, c0);

    double x_min = -1, x_max = 1, y_min = -1, y_max = 1, z_min = -1, z_max = 1;
	/*double x_min = 0.0, x_max = camera->horRes, y_min = 0.0, y_max = camera->verRes, z_min = camera->near, z_max = camera->far;*/
	
	double den, num, t;
	double tE = 0, tL = 1;
	bool visib = false;
	if(visible(dx, x_min - v0.x, tE, tL) &&
	visible(-dx, v0.x - x_max, tE, tL) &&
	visible(dy, y_min - v0.y, tE, tL) &&
	visible(-dy, v0.y - y_max, tE, tL) &&
	visible(dz, z_min - v0.z, tE, tL) &&
	visible(-dz, v0.z - z_max, tE, tL)){
		visib = true;
		if(tL < 1){
			v1.x = v0.x + dx * tL;
			v1.y = v0.y + dy * tL;
			v1.z = v0.z + dz * tL;
		}
		if(tE > 0){
			v0.x = v0.x + dx * tE;
			v0.y = v0.y + dy * tE;
			v0.z = v0.z + dz * tE;
		}
	}
	line.v0 = v0; line.v1 = v1;
    return visib;
}

void rasterizeLineHelper(Line &line, vector<vector<Color>> &image, int x, int y, Color c) {
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
	image[x][y] = color;
}

void rasterizeLine(Line &line, vector<vector<Color>> &image) {
	Vec4 v0 = line.v0, v1 = line.v1;
	Color c0 = line.c0, c1 = line.c1;
	Color c, dc;
	double x0, x1, y0, y1;
	x0 = v0.x; x1 = v1.x; y0 = v0.y; y1 = v1.y;
	int d;

	double dx = v1.x - v0.x;
	double dy = v1.y - v0.y;
	double temp;
	Color temp_color;
	// calculate the slope of the line
	double slope = abs(v1.y - v0.y) / abs(v1.x - v0.x);
	// check if the slope is between 0 and 1
	if (0 < slope && slope <= 1) {
		int y_direction = 1;
		// rasterize the line from v0 to v1
		if(x0 > x1){
			// change the vertex values
			temp = x0;
			x0 = x1;
			x1 = temp;
			temp = y0;
			y0 = y1;
			y1 = temp;
			temp_color = c0;
			c0 = c1;
			c1 = temp_color;
		}
		if(y0 > y1){
			// y will decrease
			y_direction = -1;
		}
		d = (y0 - y1) + y_direction*0.5*(x1 - x0);
		c = c0;
		dc = divideColor(subtractColor(c1, c0), abs(x1 - x0));
		int y = y0;
		for(int x = x0; x <= x1; x++) {
			rasterizeLineHelper(line, image, x, y, c);
			if (y_direction*d < 0) {
				y = y + y_direction;
				d += (y0 - y1) + y_direction*(x1 - x0);
			}
			else {
				d += y0 - y1;
			}

			c = addColor(c, dc);
		}
	}
	else {
		int x_direction = 1;
		// rasterize the line from v0 to v1
		if(x0 > x1){
			// x will decrease
			x_direction = -1;
		}
		if(y0 > y1){
			// change the vertex values
			temp = x0;
			x0 = x1;
			x1 = temp;
			temp = y0;
			y0 = y1;
			y1 = temp;
			temp_color = c0;
			c0 = c1;
			c1 = temp_color;
		}
		d = (x1 - x0) + (x_direction * 0.5 * (y0 - y1));
		c = c0;
		dc = divideColor(subtractColor(c1, c0), abs(y1 - y0));
		int x = x0;
		for(int y = y0; y <= y1; y++) {
			rasterizeLineHelper(line, image, x, y, c);
			if (x_direction * d > 0) {
				x += x_direction;
				d +=  (x1 - x0) + x_direction * (y0 - y1);
			}
			else {
				d += x1 - x0;
			}
			c = addColor(c, dc);
		}
	}
}

void perform_pers_divide(Line &line){	
	line.v0.x /= line.v0.t;
	line.v0.y /= line.v0.t;
	line.v0.z /= line.v0.t;
	line.v0.t /= line.v0.t;
	
	line.v1.x /= line.v1.t;
	line.v1.y /= line.v1.t;
	line.v1.z /= line.v1.t;
	line.v1.t /= line.v1.t;
}

void perform_viewport_transformation(Matrix4 M_vp, Line &line){
	line.v0 = multiplyMatrixWithVec4(M_vp, line.v0);
	line.v1 = multiplyMatrixWithVec4(M_vp, line.v1);
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
		//cout << M_model_matrix << endl;
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
				Matrix4 rotation_matrix = get_rotation_matrix(rotations[transformationId]);
				M_model_matrix = multiplyMatrixWithMatrix(rotation_matrix, M_model_matrix);
			}
		}
		//cout << "M_model_matrix: " << endl << M_model_matrix << endl;
		//cout << "**********************************" << endl;
		// APPLY VIWEING TRANSFORMATIONS
		Matrix4 M_cam_translation = get_cam_translation_matrix(camera);
		//cout << "M_cam_translation: " << endl << M_cam_translation << endl;
		//cout << "**********************************" << endl;
		// PERFORM PERSPECTIVE OR ORTHOGRAPHIC PROJECTION
		Matrix4 M_proj_matrix = get_perspective_proj_matrix(camera);
		//cout << "M_proj_matrix: " << endl << M_proj_matrix << endl;
		//cout << "**********************************" << endl;
		// CALCULATE THE COMPOSITE MATRIX
		Matrix4 M_composite = multiplyMatrixWithMatrix(M_proj_matrix, multiplyMatrixWithMatrix(M_cam_translation, M_model_matrix));
		//cout << "M_composite: " << endl << M_composite << endl;
		//cout << "**********************************" << endl;
		//cout << M_composite << endl;
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
			cout << "projected vertex0" << endl << vertex0 << endl;
			cout << "projected vertex1" << endl << vertex1 << endl;
			cout << "projected vertex2" << endl << vertex2 << endl;
			Color c0, c1, c2;
			c0 = *(this->colorsOfVertices[vertex0.colorId - 1]);
			c1 = *(this->colorsOfVertices[vertex1.colorId - 1]);
			c2 = *(this->colorsOfVertices[vertex2.colorId - 1]);
			//cout << "c0: " << endl << c0 << endl;

			if(mesh->type == WIREFRAME_MESH){
				// Clip the lines of the triangle in wireframe mode
				Line line1 = Line(vertex0, vertex1, c0, c1);
				Line line2 = Line(vertex1, vertex2, c1, c2);
				Line line3 = Line(vertex2, vertex0, c2, c0);
				//bool visible1 = clipLine(line1, camera);
				//bool visible2 = clipLine(line2, camera);
				//bool visible3 = clipLine(line3, camera);
				// if the projection type is perspective, apply perspective divide
				if(camera->projectionType){
					//cout << "line1.v0 before perspective divide"<< endl << line1.v0 << endl;
					//cout << "**********************************" << endl;
					perform_pers_divide(line1);
					perform_pers_divide(line2);
					perform_pers_divide(line3);
					//cout << "line1.v0 after perspective divide"<< endl << line1.v0 << endl;
					//cout << "**********************************" << endl;
				}
				/* if(visible1 || visible2 || visible3){
					cout << "one of the lines was clipped" << endl;
				} */
				// after perspective divide, apply viewport transformation
				Matrix4 M_vp = get_viewport_matrix(camera);
				perform_viewport_transformation(M_vp, line1);
				perform_viewport_transformation(M_vp, line2);
				perform_viewport_transformation(M_vp, line3);
				//cout << line1.v0 << endl;
				//cout << line1.v1 << endl;
				//cout << "**********************************" << endl;
				// perform line rasterization and color interpolation
				// use the midpoint algorithm for line rasterization
				//if(visible1) rasterizeLine(line1, this->image);
				//if(visible2) rasterizeLine(line2, this->image);
				//if(visible3) rasterizeLine(line3, this->image);
				rasterizeLine(line1, this->image);
				rasterizeLine(line2, this->image);
				rasterizeLine(line3, this->image);
			}
		}

		/* // Implement backface culling
		if(this->cullingEnabled){
			for (j = 0; j < faces.size(); j++) {
				int v0 = faces[j].vertexIds[0] - 1;
				int v1 = faces[j].vertexIds[1] - 1;
				int v2 = faces[j].vertexIds[2] - 1;

				Vec3* vertex0 = vertices[v0];
				Vec3* vertex1 = vertices[v1];
				Vec3* vertex2 = vertices[v2];

				Vec3 normal = subtractVec3(*vertex1, *vertex0);
				normal = crossProductVec3(normal, subtractVec3(*vertex2, *vertex0));
				normal = normalizeVec3(normal);

				Vec3 cameraToFace = subtractVec3(*vertex0, camera->position);

				double dotProduct = dotProductVec3(normal, cameraToFace);
				if (dotProduct < 0) {
					// Remove the face from the list of faces
					faces.erase(faces.begin() + j);

					j--;
				}
			}
		} */
		// Else, perform triangle rasterization
		// use barycentric coordinates for color interpolation

		/* // Initialize the depth buffer
		vector<vector<double>> depthBuffer;
		for (int j = 0; j < camera->horRes; j++) {
			vector<double> row;
			for (int k = 0; k < camera->verRes; k++) {
				row.push_back(1.01);
			}
			depthBuffer.push_back(row);
		}

		// Initialize the frame buffer
		vector<vector<Color>> frameBuffer;
		for (int j = 0; j < camera->horRes; j++) {
			vector<Color> row;
			for (int k = 0; k < camera->verRes; k++) {
				row.push_back(this->backgroundColor);
			}
			frameBuffer.push_back(row);
		} */
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