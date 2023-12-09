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

using namespace tinyxml2;
using namespace std;

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

/*
	Transformations, clipping, culling, rasterization are done here.
*/
void Scene::forwardRenderingPipeline(Camera *camera)
{

	Vec3 u = crossProductVec3(camera->gaze, camera->v);
	Vec3 v = crossProductVec3(camera->v, camera->gaze);
	Vec3 w = inverseVec3(camera->gaze);

	// Perform transformations, clipping, culling, and rasterization here
	// TODO: Implement the forward rendering pipeline

	// 1. Apply transformations to the scene objects

	vector<vector<Color> > image = this->image;
	vector<vector<double> > depth = this->depth;
	vector<Vec3 *> vertices = this->vertices;
	vector<Color *> colorsOfVertices = this->colorsOfVertices;
	vector<Scaling *> scalings = this->scalings;
	vector<Rotation *> rotations = this->rotations;
	vector<Translation *> translations = this->translations;
	vector<Mesh *> meshes = this->meshes;

	for(int i = 0; i < meshes.size(); i++){
		Mesh *mesh = meshes[i];
		vector<Matrix4> transformations;
		vector<Vec3> transformed_vertices;

		int j = 0;
		// 1.1 APPLY MODELING TRANSFORMATIONS
		// EXTRACT ALL THE TRANSFORMATION MATRICES
		for(; j < mesh->numberOfTransformations; j++){
			int transformationId = mesh->transformationIds[j];
			char transformationType = mesh->transformationTypes[j];

			if(transformationType == 't'){
				// TRANSLATION
				Translation *translation = translations[transformationId];
				double tx = translation->tx;
				double ty = translation->ty;
				double tz = translation->tz;
				double values[4][4] = {
					{1, 0, 0, tx},
					{0, 1, 0, ty},
					{0, 0, 1, tz},
					{0, 0, 0,  1}
				};
				Matrix4 T = values;
				transformations.push_back(T);
			}
			else if(transformationType == 's'){
				// SCALING
				Scaling *scaling = scalings[transformationId];
				double sx = scaling->sx;
				double sy = scaling->sy;
				double sz = scaling->sz;
				double values[4][4] = {
					{sx,  0,  0, 0},
					{ 0, sy,  0, 0},
					{ 0,  0, sz, 0},
					{ 0,  0,  0, 1}
				};
				Matrix4 S = values;
				transformations.push_back(S);
			}
			else{
				// ROTATION
				Rotation *rotation = rotations[transformationId];
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

				double R_x_alpha_values[4][4] = {
					{1,         0, 		    0, 0},
					{0, cos_alpha, -sin_alpha, 0},
					{0, sin_alpha,  cos_alpha, 0},
					{0, 		0, 			0, 1}
				};
				double R_x_minus_alpha_values[4][4] = {
					{1,          0, 	     0, 0},
					{0,  cos_alpha,  sin_alpha, 0},
					{0, -sin_alpha,  cos_alpha, 0},
					{0, 		 0, 		 0, 1}
				};

				// CALCULATE THE Y-AXIS ALIGNMENT MATRIX

				double cos_beta = d;
				double sin_beta = a;

				double R_y_beta_values[4][4] = {
					{cos_beta, 0, -sin_beta, 0},
					{		0, 1, 		  0, 0},
					{sin_beta, 0,  cos_beta, 0},
					{		0, 0, 		  0, 1}
				};
				double R_y_minus_beta_values[4][4] = {
					{ cos_beta, 0, sin_beta, 0},
					{		 0, 1, 		  0, 0},
					{-sin_beta, 0, cos_beta, 0},
					{		 0, 0, 		  0, 1}
				};

				Matrix4 R_x_alpha, R_x_minus_alpha, R_y_beta, R_y_minus_beta;

				R_x_alpha = R_x_alpha_values;
				R_x_minus_alpha = R_x_minus_alpha_values;
				R_y_beta = R_y_beta_values;
				R_y_minus_beta = R_y_minus_beta_values;

				// CALCULATE THE ACTUAL ROTATION MATRIX

				double cos_angle = cos(angle);
				double sin_angle = sin(angle);

				double R_z_angle_values[4][4] = {
					{cos_angle, -sin_angle, 0, 0},
					{sin_angle,  cos_angle, 0, 0},
					{		 0, 		 0, 0, 0},
					{		 0, 		 0, 0, 1}
				};

				Matrix4 R_z_angle;
				
				R_z_angle = R_z_angle_values;

				// CALCULATE THE COMPOSITE ROTATION MATRIX

				Matrix4 R;
				R = multiplyMatrixWithMatrix(R_x_minus_alpha, R_y_beta);
				R = multiplyMatrixWithMatrix(R, R_z_angle);
				R = multiplyMatrixWithMatrix(R, R_y_minus_beta);
				R = multiplyMatrixWithMatrix(R, R_x_alpha);
				
				transformations.push_back(R);
			}
		}

		// CALCULATE COMPOSITE MATRIX M

		double ones[4][4] = {{1,1,1,1},{1,1,1,1},{1,1,1,1},{1,1,1,1}};
		Matrix4 M_transform = ones;
		for(j = 0; j < transformations.size(); j++){
			Matrix4 m = transformations[j];
			M_transform = multiplyMatrixWithMatrix(m, M_transform);
		}

		// APPLY THE COMPOSITE MATRIX M TO ALL VERTICES

		for(j = 0; j < vertices.size(); j++){
			double x = vertices[j]->x;
			double y = vertices[j]->y;
			double z = vertices[j]->z;

			Vec4 point = Vec4(x, y, z, 1);

			Vec4 new_vertex = multiplyMatrixWithVec4(M_transform, point);

			vertices[j]->x = new_vertex.x;
			vertices[j]->y = new_vertex.y;
			vertices[j]->z = new_vertex.z;
		}

		// 1.2 APPLY VIWEING TRANSFORMATIONS

		// TODO: PERFORM CAMERA TRANSFORMATION

		// TRANSLATE "E" TO THE WORLD ORIGIN (0,0,0)

		double e_x, e_y, e_z;
		e_x = camera->position.x;
		e_y = camera->position.y;
		e_z = camera->position.z;

		double translation_values[4][4] = {
			{1, 0, 0, -e_x},
			{0, 1, 0, -e_y},
			{0, 0, 1, -e_z},
			{0, 0, 0,    1}
		};

		Matrix4 T;
		T = translation_values;

		// ROTATE UVW TO ALIGN IT WITH XYZ

		double rotation_values[4][4] = {
			{u.x, u.y, u.z, 0},
			{v.x, v.y, v.z, 0},
			{w.x, w.y, w.z, 0},
			{  0,   0,   0, 1}
		};

		Matrix4 R;
		R = rotation_values;

		// CALCULATE THE COMPOSITE CAMERA TRANSFORMATION MATRIX

		Matrix4 M_cam = multiplyMatrixWithMatrix(R, T);

		// APPLY THE COMPOSITE CAMERA TRANSFORMATION MATRIX TO ALL THE VERTICES

		for(j = 0; j < vertices.size(); j++){
			double x = vertices[j]->x;
			double y = vertices[j]->y;
			double z = vertices[j]->z;

			Vec4 point = Vec4(x, y, z, 1);

			Vec4 new_vertex = multiplyMatrixWithVec4(M_cam, point);

			vertices[j]->x = new_vertex.x;
			vertices[j]->y = new_vertex.y;
			vertices[j]->z = new_vertex.z;
		}

		// PERFORM PERSPECTIVE OR ORTHOGRAPHIC PROJECTION

		Matrix4 M_per;
		
		// CALCULATE THE ORTHOGRAPHIC PROJECTION MATRIX
		
		double r, l, t, b, n, f;
		r = camera->right;
		l = camera->left;
		t = camera->top;
		b = camera->bottom;
		n = camera->near;
		f = camera->far;

		double M_orth_values[4][4] = {
			{2 / (r - l), 			0, 			  0,  -((r + l) / (r - l))},
			{		   0, 2 / (t - b), 			  0,  -((t + b) / (t - b))},
			{		   0, 			0, -2 / (f - n),  -((f + n) / (f - n))},
			{		   0, 			0, 			  0, 					 1}
		};

		Matrix4 M_orth = M_orth_values;

		// IF THE PROJECTION TYPE IS PERSPECTIVE, CALCULATE THE M_P20 MATRIX AS WELL

		if(camera->projectionType == PERSPECTIVE_PROJECTION){
			double M_p2o_values[4][4] = {
				{n, 0, 	   0,     0},
				{0, n, 	   0, 	  0},
				{0, 0, f + n, f * n},
				{0, 0, 	  -1, 	  0}
			};

			Matrix4 M_p2o = M_p2o_values;

			M_per = multiplyMatrixWithMatrix(M_orth, M_p2o);
		}
		else{
			M_per = M_orth;
		}

		

		double M_per_values[4][4] = {
			{(2 * n) / (r - l), 				0, 	  (r + l) / (r - l), 						0},
			{				 0, (2 * n) / (t - b),    (t + b) / (t - b), 						0},
			{				 0, 				0, -((f + n) / (f - n)), -((2 * f * n) / (f - n))},
			{				 0, 				0, 					 -1, 						0}
		};

		Matrix4 M_per = M_per_values;

		// APPLY THE PERSPECTIVE PROJECTION TO ALL THE VERTICES

		for(j = 0; j < vertices.size(); j++){
			double x = vertices[j]->x;
			double y = vertices[j]->y;
			double z = vertices[j]->z;

			Vec4 point = Vec4(x, y, z, 1);

			Vec4 new_vertex = multiplyMatrixWithVec4(M_per, point);

			vertices[j]->x = new_vertex.x / new_vertex.t;
			vertices[j]->y = new_vertex.y / new_vertex.t;
			vertices[j]->z = new_vertex.z / new_vertex.t;
		}

		// 1.3 APPLY VIEWPORT TRANSFORMATION

		int n_x, n_y;
		n_x = camera->horRes;
		n_y = camera->verRes;

		double M_vp_values[4][4] = {
			{n_x / 2, 	    0,   0, (n_x - 1) / 2},
			{	   0, n_y / 2,   0, (n_y - 1) / 2},
			{	   0, 		0, 1/2, 		  1/2},
			{	   0, 		0,   0, 		    0} // TODO: WHAT DO WE PUT IN HERE?
		};
	}

	// 2. Perform clipping and culling operations
	// 3. Rasterize the scene objects onto the image

	// You can add your own code here to implement the forward rendering pipeline
}
