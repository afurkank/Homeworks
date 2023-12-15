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

Matrix4 get_rotation_matrix(Rotation *rotation){
	// ROTATION
	
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
		{  0,   0,   0, 1}
	};

	// CALCULATE THE COMPOSITE CAMERA TRANSFORMATION MATRIX
	Matrix4 M_cam = multiplyMatrixWithMatrix(rotate_camera, translate_camera);
	return M_cam;
}

Matrix4 get_perspective_proj_matrix(Camera *camera){
	Matrix4 M_per;
	// CALCULATE THE ORTHOGRAPHIC PROJECTION MATRIX
	double r, l, t, b, n, f;
	r = camera->right;
	l = camera->left;
	t = camera->top;
	b = camera->bottom;
	n = camera->near;
	f = camera->far;

	double M_orth[4][4] = {
		{2 / (r - l), 			0, 			  0,  -((r + l) / (r - l))},
		{		   0, 2 / (t - b), 			  0,  -((t + b) / (t - b))},
		{		   0, 			0, -2 / (f - n),  -((f + n) / (f - n))},
		{		   0, 			0, 			  0, 					 1}
	};

	// IF THE PROJECTION TYPE IS PERSPECTIVE, CALCULATE THE M_P20 MATRIX AS WELL
	if(camera->projectionType == PERSPECTIVE_PROJECTION){
		double M_p2o[4][4] = {
			{n, 0, 	   0,     0},
			{0, n, 	   0, 	  0},
			{0, 0, f + n, f * n},
			{0, 0, 	  -1, 	  0}
		};

		M_per = multiplyMatrixWithMatrix(M_orth, M_p2o);
	}
	else{
		M_per = M_orth;
	}
}

// Liang-Barsky line clipping algorithm
bool clipLine(Line &line, double &tE, double &tL, Camera *camera) {
	Vec4 v0, v1;
	v0 = line.v0; v1 = line.v1;
	Color c0, c1;
	c0 = line.c0; c1 = line.c1;
    double dx = v1.x - v0.x;
    double dy = v1.y - v0.y;
    double dz = v1.z - v0.z;
	Color dc = subtractColor(c1, c0);

    double x_min = -1, x_max = 1, y_min = -1, y_max = 1, z_min = -1, z_max = 1;
	/*double x_min = 0.0, x_max = camera->horRes, y_min = 0.0, y_max = camera->verRes, z_min = camera->near, z_max = camera->far;*/
    double denominators[6] = {dx, -dx, dy, -dy, dz, -dz};
    double numerators[6] = {x_min - v0.x, v0.x - x_max, y_min - v0.y, v0.y - y_max, z_min - v0.z, v0.z - z_max};

    for (int i = 0; i < 6; i++) {
		double den, num, t;
		den = denominators[i]; num = numerators[i];
		t = num / den;

		if(den > 0){
			if(t > tL) return false;
			else if(t > tE) tE = t;
		}
		else if(den < 0){
			if(t < tE) return false;
			else if(t < tL) tL = t;
		}
		else if(num > 0) return false;

	}

	if(tL < 1){
		line.v1.x = v0.x + (dx * tL);
		line.v1.y = v0.y + (dy * tL);
		line.v1.z = v0.z + (dz * tL);
		c1 = addColor(c0, multiplyColor(dc, tL));
	}
	if(tE > 0){
		line.v0.x = v0.x + (dx * tE);
		line.v0.y = v0.y + (dy * tE);
		line.v0.z = v0.z + (dz * tE);
		c0 = addColor(c0, multiplyColor(dc, tE));
	}
	
    return true;
}

/*return true;

if (denominators[i] == 0 && numerators[i] < 0) return false;
double t = numerators[i] / denominators[i];
if (denominators[i] < 0) {
	if (t > tL) return false;
	else if (t > tE) tE = t;
}
else {
	if (t < tE) return false;
	else if (t < tL) tL = t;
}*/
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
	
	int n_x, n_y;
	n_x = camera->horRes;
	n_y = camera->verRes;
	
	for(int i = 0; i < meshes.size(); i++){
		Mesh *mesh = meshes[i];
		vector<Matrix4> transformations;
		vector<Vec3> transformed_vertices;

		int j = 0;
		
		// CALCULATE COMPOSITE MODELING TRANSFORMATION MATRIX
		Matrix4 M_model_matrix = getIdentityMatrix();
		for(; j < mesh->numberOfTransformations; j++){
			int transformationId = mesh->transformationIds[j] - 1;

			if(mesh->transformationTypes[j] == 't'){
				// TRANSLATION
				Translation *translation = translations[transformationId];
				Matrix4 translation_matrix = get_translation_matrix(translation);
				M_model_matrix = multiplyMatrixWithMatrix(translation_matrix, M_model_matrix);
			}
			else if(mesh->transformationTypes[j] == 's'){
				// SCALING
				Scaling *scaling = scalings[transformationId];
				Matrix4 scaling_matrix = get_scaling_matrix(scaling);
				M_model_matrix = multiplyMatrixWithMatrix(scaling_matrix, M_model_matrix);
			}
			else{
				Rotation *rotation = rotations[transformationId];
				Matrix4 rotation_matrix = get_rotation_matrix(rotation);
				M_model_matrix = multiplyMatrixWithMatrix(rotation_matrix, M_model_matrix);
			}
		}
		cout << "b" << endl;
		// APPLY VIWEING TRANSFORMATIONS
		Matrix4 M_cam_translation = get_cam_translation_matrix(camera);
		// PERFORM PERSPECTIVE OR ORTHOGRAPHIC PROJECTION
		Matrix4 M_proj_matrix = get_perspective_proj_matrix(camera);
		// CALCULATE THE COMPOSITE MATRIX
		Matrix4 M_composite = multiplyMatrixWithMatrix(M_proj_matrix, multiplyMatrixWithMatrix(M_cam_translation, M_model_matrix));
		// PERFORM CLIPPING AND CULLING OPERATIONS
		for (int j = 0; j < mesh->numberOfTriangles; j++) {
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

			if(mesh->type == WIREFRAME_MESH){
				// Clip the lines of the triangle
				double tE = 0, tL = 1;
				Color *c0, *c1, *c2;
				c0 = this->colorsOfVertices[vertex0.colorId - 1];
				c1 = this->colorsOfVertices[vertex1.colorId - 1];
				c2 = this->colorsOfVertices[vertex2.colorId - 1];
				Line line1 = Line(vertex0, vertex1, *c0, *c1);
				Line line2 = Line(vertex1, vertex2, *c1, *c2);
				Line line3 = Line(vertex2, vertex0, *c2, *c0);
				bool visible = clipLine(line1, tE, tL, camera);
				if (visible) {
					if (tL < 1) {
						line1.v1.x = vertex0.x + (vertex1.x - vertex0.x) * tL;
						line1.v1.y = vertex0.y + (vertex1.y - vertex0.y) * tL;
						line1.v1.z = vertex0.z + (vertex1.z - vertex0.z) * tL;
					}
					if (tE > 0) {
						line1.v0.x = vertex0.x + (vertex1.x - vertex0.x) * tE;
						line1.v0.y = vertex0.y + (vertex1.y - vertex0.y) * tE;
						line1.v0.z = vertex0.z + (vertex1.z - vertex0.z) * tE;
					}
				}

				visible = clipLine(line2, tE, tL, camera);
				if (visible) {
					if (tL < 1) {
						line1.v1.x = vertex1.x + (vertex2.x - vertex1.x) * tL;
						line1.v1.y = vertex1.y + (vertex2.y - vertex1.y) * tL;
						line1.v1.z = vertex1.z + (vertex2.z - vertex1.z) * tL;
					}
					if (tE > 0) {
						line1.v0.x = vertex1.x + (vertex2.x - vertex1.x) * tE;
						line1.v0.y = vertex1.y + (vertex2.y - vertex1.y) * tE;
						line1.v0.z = vertex1.z + (vertex2.z - vertex1.z) * tE;
					}
				}

				visible = clipLine(line3, tE, tL, camera);
				if (visible) {
					if (tL < 1) {
						line1.v1.x = vertex2.x + (vertex0.x - vertex2.x) * tL;
						line1.v1.y = vertex2.y + (vertex0.y - vertex2.y) * tL;
						line1.v1.z = vertex2.z + (vertex0.z - vertex2.z) * tL;
					}
					if (tE > 0) {
						line1.v0.x = vertex2.x + (vertex0.x - vertex2.x) * tE;
						line1.v0.y = vertex2.y + (vertex0.y - vertex2.y) * tE;
						line1.v0.z = vertex2.z + (vertex0.z - vertex2.z) * tE;
					}
				}
			}
		}

		// CALCULATE VIEWPORT TRANSFORMATION MATRIX
		double M_vp_values[4][4] = {
			{n_x / 2.0, 	    0,   0, (n_x - 1) / 2.0},
			{	     0, n_y / 2.0,   0, (n_y - 1) / 2.0},
			{	     0, 		0, 1/2, 		    1/2},
			{	     0, 		0,   0, 		      1}
		};

		Matrix4 M_vp = M_vp_values;

		// APPLY THE VIEWPORT TRANSFORMATION TO ALL THE VERTICES

		for(j = 0; j < vertices.size(); j++){
			double x = vertices[j]->x;
			double y = vertices[j]->y;
			double z = vertices[j]->z;

			Vec4 point = Vec4(x, y, z, 1);

			Vec4 new_vertex = multiplyMatrixWithVec4(M_vp, point);

			vertices[j]->x = new_vertex.x;
			vertices[j]->y = new_vertex.y;
			vertices[j]->z = new_vertex.z;
		}

		// Implement backface culling
		if(this->cullingEnabled){
			for (int j = 0; j < faces.size(); j++) {
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
		}
		// PERFORM RASTERIZATION

		// If mesh is of type wireframe, perform line rasterization and color interpolation
		// use the midpoint algorithm for line rasterization

		if(mesh->type == WIREFRAME_MESH){
			for (int j = 0; j < faces.size(); j++){
				Vec3* vertex0 = vertices[faces[j].vertexIds[0]] - 1;
				Vec3* vertex1 = vertices[faces[j].vertexIds[1]] - 1;
				Vec3* vertex2 = vertices[faces[j].vertexIds[2]] - 1;

				Color c, c0, c1, dc;
				double x, y;
				double x0, x1, y0, y1;
				double d;

				// Rasterize the line from vertex0 to vertex1
				x0 = (vertex0->x < vertex1->x) ? vertex0->x : vertex1->x;
				x1 = (vertex0->x > vertex1->x) ? vertex1->x : vertex0->x;
				y0 = (vertex0->y < vertex1->y) ? vertex0->y : vertex1->y;
				y1 = (vertex0->y > vertex1->y) ? vertex1->y : vertex0->y;

				x = x0; y = y0;

				d = 2 * (y0 - y1) + (x1 - x0);

				c0 = *(this->colorsOfVertices[vertex0->colorId]);
				c1 = *(this->colorsOfVertices[vertex0->colorId]);

				c = c0;

				dc = divideColor(subtractColor(c1, c0), (x1 - x0));
				while(x < x1){
				
					// round(c)
					int r, g, b;
					Color roundedColor = roundColor(c);
					r = roundedColor.r; g = roundedColor.g; b = roundedColor.b;

					// make color values between 0 and 255
					r = makeBetweenZeroAnd255(r);
					g = makeBetweenZeroAnd255(g);
					b = makeBetweenZeroAnd255(b);

					Color color = Color(r, g, b);

					// image[x][y] = color
					assignColorToPixel(int(x), int(y), color);

					if(d < 0){
						y = y+1;
						d += 2 * ((y0 - y1) + (x1 - x0));
					}
					else{
						d += 2 * (y0 - y1);
					}

					c = addColor(c, dc);
				}
			}
		}

		// Else, perform triangle rasterization
		// use barycentric coordinates for color interpolation
		else{

		}

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

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
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
	command = "./magick convert " + ppmFileName + " " + ppmFileName + ".png";
	system(command.c_str());
}