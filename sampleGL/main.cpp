#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[3];
int gWidth, gHeight;

GLint modelingMatrixLoc[3];
GLint viewingMatrixLoc[3];
GLint projectionMatrixLoc[3];
GLint eyePosLoc[3];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

float bunny_x = 0;
float bunny_y = 0;

int activeProgramIndex = 0;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

vector<Vertex> gVertices[3];
vector<Texture> gTextures[3];
vector<Normal> gNormals[3];
vector<Face> gFaces[3];

GLuint gVertexAttribBuffer[3], gIndexBuffer[3];
GLint gInVertexLoc[3], gInNormalLoc[3];
int gVertexDataSizeInBytes[3], gNormalDataSizeInBytes[3];

bool ParseObj(const string& fileName, int i)
{
	fstream myfile;
	
	// rewrite this function using indices

	// fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						gTextures[i].push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals[i].push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices[i].push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					gFaces[i].push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					cout << "Ignoring unidentified line in obj file: " << curLine << endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x,
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x,
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x,
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(gVertices[i].size() == gNormals[i].size());

	return true;			
}

bool ReadDataFromFile(
	const string& fileName, ///< [in]  Name of the shader file
	string& data)     ///< [out] The contents of the file
{
	fstream myfile;

	// Open the input 
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	string shaderSource;

	string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		cout << "Cannot find file name: " + filename << endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

	gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();

	// Create the shaders for both programs

	GLuint vs1 = createVS("bunny_vert.glsl");
	GLuint fs1 = createFS("bunny_frag.glsl");

	GLuint vs2 = createVS("cube_vert.glsl");
	GLuint fs2 = createFS("cube_frag.glsl");

	GLuint vs3 = createVS("quad_vert.glsl");
	GLuint fs3 = createFS("quad_frag.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	// Link the programs
	GLint status;

	glLinkProgram(gProgram[0]);
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[2]);
	glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 3; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO(int i)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	assert(vao > 0);
	glBindVertexArray(vao);
	cout << "vao = " << vao << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer[i]);
	glGenBuffers(1, &gIndexBuffer[i]);

	assert(gVertexAttribBuffer[i] > 0 && gIndexBuffer[i] > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[i]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[i]);

	gVertexDataSizeInBytes[i] = gVertices[i].size() * 3 * sizeof(GLfloat);
	gNormalDataSizeInBytes[i] = gNormals[i].size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = gFaces[i].size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat[gVertices[i].size() * 3];
	GLfloat* normalData = new GLfloat[gNormals[i].size() * 3];
	GLuint* indexData = new GLuint[gFaces[i].size() * 3];

	float minX = 1e6, maxX = -1e6;
	float minY = 1e6, maxY = -1e6;
	float minZ = 1e6, maxZ = -1e6;

	for (int j = 0; j < gVertices[i].size(); ++j)
	{
		vertexData[3 * j] = gVertices[i][j].x;
		vertexData[3 * j + 1] = gVertices[i][j].y;
		vertexData[3 * j + 2] = gVertices[i][j].z;

		minX = std::min(minX, gVertices[i][j].x);
		maxX = std::max(maxX, gVertices[i][j].x);
		minY = std::min(minY, gVertices[i][j].y);
		maxY = std::max(maxY, gVertices[i][j].y);
		minZ = std::min(minZ, gVertices[i][j].z);
		maxZ = std::max(maxZ, gVertices[i][j].z);
	}

	std::cout << "minX = " << minX << std::endl;
	std::cout << "maxX = " << maxX << std::endl;
	std::cout << "minY = " << minY << std::endl;
	std::cout << "maxY = " << maxY << std::endl;
	std::cout << "minZ = " << minZ << std::endl;
	std::cout << "maxZ = " << maxZ << std::endl;

	for (int j = 0; j < gNormals[i].size(); ++j)
	{
		normalData[3 * j] = gNormals[i][j].x;
		normalData[3 * j + 1] = gNormals[i][j].y;
		normalData[3 * j + 2] = gNormals[i][j].z;
	}

	for (int j = 0; j < gFaces[i].size(); ++j)
	{
		indexData[3 * j] = gFaces[i][j].vIndex[0];
		indexData[3 * j + 1] = gFaces[i][j].vIndex[1];
		indexData[3 * j + 2] = gFaces[i][j].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[i] + gNormalDataSizeInBytes[i], 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes[i], vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[i], gNormalDataSizeInBytes[i], normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying to GPU memory; can free now from CPU memory
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[i]));
}

void init()
{
	ParseObj("bunny.obj", 0);
	glEnable(GL_DEPTH_TEST);
	ParseObj("cube.obj", 1);
	glEnable(GL_DEPTH_TEST);
	ParseObj("quad.obj", 2);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_DEPTH_TEST);
	initShaders();

	initVBO(0);
	initVBO(1);
	initVBO(2);
}

void drawModel(int i)
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[i]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[i]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[i]));

	glDrawElements(GL_TRIANGLES, gFaces[i].size() * 3, GL_UNSIGNED_INT, 0);
}

glm::vec3 getBunnyPosition() {
    glm::vec3 bunnyPosition(0.0f);
    int numVertices = gVertices[0].size(); // Assuming bunny vertices are at index 0

    for (int i = 0; i < numVertices; ++i) {
        bunnyPosition.x += gVertices[0][i].x;
        bunnyPosition.y += gVertices[0][i].y;
        bunnyPosition.z += gVertices[0][i].z;
    }

    bunnyPosition /= static_cast<float>(numVertices);
    return bunnyPosition;
}

void display()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;

	float knocked = glm::radians(0.0f);

	float angleRad = (float)(angle / 180.0) * M_PI;

	float rotationAngle = glm::radians(270.0f);

	glm::mat4 transformMatrix = glm::mat4(1.0);
	// Compute the modeling matrix for the bunny
    transformMatrix = glm::translate(transformMatrix, glm::vec3(bunny_x, 0.3 * sin(10 * bunny_y * 1.0f) - 0.3f, -3.f));
	// Add rotation to the bunny's modeling matrix with 270 degrees rotation around the y axis
	transformMatrix = glm::rotate(transformMatrix, rotationAngle, glm::vec3(0, 1, 0));
	// Add rotation to the bunny's modeling matrix with 270 degrees rotation around the y axis
	transformMatrix = glm::rotate(transformMatrix, knocked, glm::vec3(1, 0, 0));
	// Scale the bunny to 1/10 of its original size
	transformMatrix = glm::scale(transformMatrix, glm::vec3(0.3f, 0.3f, 0.3f));

    // Set the active program and the values of its uniform variables for the bunny
    glUseProgram(gProgram[0]);
    glUniformMatrix4fv(projectionMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(transformMatrix));
    glUniform3fv(eyePosLoc[0], 1, glm::value_ptr(eyePos));

    // Draw Bunny
    drawModel(0);

	// Compute the modeling matrix for the cube
	glm::mat4 cubeTransformMatrix = glm::mat4(1.0);
	// Add rotation to the cube's modeling matrix with 270 degrees rotation around the y axis
	cubeTransformMatrix = glm::rotate(cubeTransformMatrix, rotationAngle, glm::vec3(0, 1, 0));
	// Scale the cube to 1/10 of its original size
	cubeTransformMatrix = glm::scale(cubeTransformMatrix, glm::vec3(0.99f, 0.99f, 0.99f));

	// Set the active program and the values of its uniform variables for the cube
	glUseProgram(gProgram[1]);

	glUniformMatrix4fv(projectionMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(cubeTransformMatrix));
	glUniform3fv(eyePosLoc[1], 1, glm::value_ptr(eyePos));

	// Draw Cube
	drawModel(1);

	float groundRotationAngle = glm::radians(-70.0f);
	glm::vec3 groundTranslation(0.0f, 0.0f, -3.0f);
	glm::mat4 groundTransformMatrix = glm::mat4(1.0);
	groundTransformMatrix = glm::rotate(groundTransformMatrix, groundRotationAngle, glm::vec3(1, 0, 0));
	groundTransformMatrix = glm::translate(groundTransformMatrix, groundTranslation);
	groundTransformMatrix = glm::scale(groundTransformMatrix, glm::vec3(5.0f, 100.0f, 1.0f)); // Adjust scale as needed

	glUseProgram(gProgram[2]);

	// Set uniform variables for checkerboard pattern
	glUniform1f(glGetUniformLocation(gProgram[2], "scale"), 1.0f); // Adjust scale as needed
	glUniform1f(glGetUniformLocation(gProgram[2], "offset"), 5.0f); // Adjust width as needed
	glUniform1f(glGetUniformLocation(gProgram[2], "offsetZ"), 1.0f); // Adjust Z offset as needed

	// Set the active program and the values of its uniform variables for the quad
	glUniformMatrix4fv(projectionMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(groundTransformMatrix));
	glUniform3fv(eyePosLoc[2], 1, glm::value_ptr(eyePos));

	// Draw Quad
	drawModel(2);
}

void reshape(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	// Use perspective projection
	float fovyRad = (float)(90.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)
	// 
	//viewingMatrix = glm::mat4(1);
	viewingMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) + glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	else if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		activeProgramIndex = 0;
	}
	else if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		activeProgramIndex = 1;
	}
	else if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		glShadeModel(GL_FLAT);
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		glShadeModel(GL_SMOOTH);
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
	GLFWwindow* window;
	if (!glfwInit())
	{
		exit(-1);
	}

	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this if on MacOS

	int width = 1000, height = 800;
	window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	char rendererInfo[512] = { 0 };
	strcpy(rendererInfo, (const char*)glGetString(GL_RENDERER)); // Use strcpy_s on Windows, strcpy on Linux
	strcat(rendererInfo, " - "); // Use strcpy_s on Windows, strcpy on Linux
	strcat(rendererInfo, (const char*)glGetString(GL_VERSION)); // Use strcpy_s on Windows, strcpy on Linux
	glfwSetWindowTitle(window, rendererInfo);

	init();

	glfwSetKeyCallback(window, keyboard);
	glfwSetWindowSizeCallback(window, reshape);

	reshape(window, width, height); // need to call this once ourselves
	mainLoop(window); // this does not return unless the window is closed

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
