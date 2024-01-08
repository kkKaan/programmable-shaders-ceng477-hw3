#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cassert>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <ft2build.h>
#include <ctime>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[4];
int gWidth, gHeight;

GLint modelingMatrixLoc[3];
GLint viewingMatrixLoc[3];
GLint projectionMatrixLoc[3];
GLint eyePosLoc[3];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 0, 0);

float bunny_x = 0.0f;
float bunny_y = 0.0f;
float box_position_z = 0.0f;
float quad_position_z = 0.0f;

float bunny_rotation_angle = 0.0f;

float speed = 1.0f;

float offset_before_hit_red;
float coef_before_hit_red;

int hit_red_index;
// initialize yellow_cube_index to a random number between 0 and 2 give time seed

int yellow_cube_index = 0;

bool move_left = false;
bool move_right = false;

bool hit_red = false;
bool hit_yellow = false;
bool restart = false;

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

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

vector<Vertex> gVertices[3];
vector<Texture> gTextures[3];
vector<Normal> gNormals[3];
vector<Face> gFaces[3];

GLuint gTextVBO;

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
	gProgram[3] = glCreateProgram();

	// Create the shaders for both programs

	GLuint vs1 = createVS("bunny_vert.glsl");
	GLuint fs1 = createFS("bunny_frag.glsl");

	GLuint vs2 = createVS("cube_vert.glsl");
	GLuint fs2 = createFS("cube_frag.glsl");

	GLuint vs3 = createVS("quad_vert.glsl");
	GLuint fs3 = createFS("quad_frag.glsl");

	GLuint vs4 = createVS("text_vert.glsl");
	GLuint fs4 = createFS("text_frag.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	glAttachShader(gProgram[3], vs4);
	glAttachShader(gProgram[3], fs4);

    glBindAttribLocation(gProgram[3], 2, "vertex");

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

	glLinkProgram(gProgram[3]);
	glGetProgramiv(gProgram[3], GL_LINK_STATUS, &status);

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

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[3]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	initFonts(gWidth, gHeight);
}

void drawModel(int i)
{
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[i]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[i]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[i]));

	glDrawElements(GL_TRIANGLES, gFaces[i].size() * 3, GL_UNSIGNED_INT, 0);
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[3]);
    glUniform3f(glGetUniformLocation(gProgram[3], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
		
        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void display()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	float knocked = glm::radians(0.0f);

	if (speed >= 1.0f) speed += 0.0015f;

	if (move_left && bunny_x > -3.5f) {
		bunny_x -= 0.05f * speed;
	}

	if (move_right && bunny_x < 3.5f) {
		bunny_x += 0.05f * speed;
	}

	// Adjust bunny_y to move bunny up and down in time
	bunny_y += 0.008f * speed;

	box_position_z += 0.1f;

	quad_position_z += 0.05f;

	// Compute the modeling matrix for the bunny
	glm::mat4 transformMatrix = glm::mat4(1.0);
    transformMatrix = glm::translate(transformMatrix, glm::vec3(bunny_x, 0.2 * cos(10 * bunny_y * 1.0f) - 2.0f, -3.0f));
	// Add rotation to the bunny's modeling matrix with 270 degrees rotation around the y axis
	transformMatrix = glm::rotate(transformMatrix, glm::radians(270.0f), glm::vec3(0, 1, 0));

	if (hit_yellow && bunny_rotation_angle < 360.0f) {
		bunny_rotation_angle = min(360.0f, bunny_rotation_angle + 3.0f * speed);
		transformMatrix = glm::rotate(transformMatrix, glm::radians(bunny_rotation_angle), glm::vec3(0, 1, 0));
	}

	else if (hit_yellow && bunny_rotation_angle >= 360.0f) {
		hit_yellow = false;
		bunny_rotation_angle = 0.0f;
	}

	if (hit_red) {
		transformMatrix = glm::rotate(transformMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	}

	// Scale the bunny to 1/10 of its original size
	transformMatrix = glm::scale(transformMatrix, glm::vec3(0.4f, 0.4f, 0.4f));

    // Set the active program and the values of its uniform variables for the bunny
    glUseProgram(gProgram[0]);
    glUniformMatrix4fv(projectionMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc[0], 1, GL_FALSE, glm::value_ptr(transformMatrix));
    glUniform3fv(eyePosLoc[0], 1, glm::value_ptr(eyePos));

    // Draw Bunny
    drawModel(0);

	if (-30.0f + box_position_z * speed * 0.7f > -1.0f) {
		box_position_z = 0.0f;
		yellow_cube_index = rand() % 3;
	}

	for (int i = 0; i < 3; i++) {
		float position_x;
		switch (i) {
			case 0:
				position_x = -2.5f;
				break;
			case 1:
				position_x = 0.0f;
				break;
			case 2:
				position_x = 2.5f;
				break;
		}

		if (bunny_x < position_x + 0.75f && bunny_x > position_x - 0.75f && -30.0f + box_position_z * speed * 0.7f >= -4.25f) {
			if (i == yellow_cube_index) {
				cout << "You win!" << endl;
				hit_yellow = true;
			}
			else {
				cout << "You lose!" << endl;
				hit_red_index = i;
				hit_red = true;
			}

			continue;
		}

		if (hit_red && i == hit_red_index) continue;

		if (!hit_red) coef_before_hit_red = box_position_z * speed;

		// Compute the modeling matrix for the cube
		glm::mat4 cubeTransformMatrix = glm::mat4(1.0);

		if (hit_red) {
			cubeTransformMatrix = glm::translate(cubeTransformMatrix, glm::vec3(position_x, -1.5f, -30.0f + coef_before_hit_red * 0.7f));
		}
		else {
			cubeTransformMatrix = glm::translate(cubeTransformMatrix, glm::vec3(position_x, -1.5f, -30.0f + box_position_z * speed * 0.7f));
		}

		// Add rotation to the cube's modeling matrix with 270 degrees rotation around the y axis
		cubeTransformMatrix = glm::rotate(cubeTransformMatrix, glm::radians(5.0f), glm::vec3(1, 0, 0));
		// Scale the cube to 1/10 of its original size
		cubeTransformMatrix = glm::scale(cubeTransformMatrix, glm::vec3(0.4f, 1.0f, 0.25f));

		// Set the active program and the values of its uniform variables for the cube
		glUseProgram(gProgram[1]);

		if (i == yellow_cube_index) {
			glUniform3f(glGetUniformLocation(gProgram[1], "kd"), 1.0f, 1.0f, 0.0f);
		}
		else {
			glUniform3f(glGetUniformLocation(gProgram[1], "kd"), 1.0f, 0.0f, 0.0f);
		}

		glUniformMatrix4fv(projectionMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
		glUniformMatrix4fv(viewingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
		glUniformMatrix4fv(modelingMatrixLoc[1], 1, GL_FALSE, glm::value_ptr(cubeTransformMatrix));
		glUniform3fv(eyePosLoc[1], 1, glm::value_ptr(eyePos));

		// Draw Cube
		drawModel(1);
	}

	if (!hit_red) offset_before_hit_red = quad_position_z * speed;

	float groundRotationAngle = glm::radians(-90.0f);
	glm::vec3 groundTranslation(0.0f, 0.0f, -3.0f);
	glm::mat4 groundTransformMatrix = glm::mat4(1.0);
	groundTransformMatrix = glm::rotate(groundTransformMatrix, groundRotationAngle, glm::vec3(1, 0, 0));
	groundTransformMatrix = glm::translate(groundTransformMatrix, groundTranslation);
	groundTransformMatrix = glm::scale(groundTransformMatrix, glm::vec3(5.0f, 100.0f, 1.0f)); // Adjust scale as needed

	glUseProgram(gProgram[2]);

	// Set uniform variables for checkerboard pattern
	glUniform1f(glGetUniformLocation(gProgram[2], "scale"), 0.5f); // Adjust scale as needed

	if (hit_red) {
		glUniform1f(glGetUniformLocation(gProgram[2], "offset"), offset_before_hit_red);
	}
	else {
		glUniform1f(glGetUniformLocation(gProgram[2], "offset"), quad_position_z * speed); // Adjust width as needed
	}

	// Set the active program and the values of its uniform variables for the quad
	glUniformMatrix4fv(projectionMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(groundTransformMatrix));
	glUniform3fv(eyePosLoc[2], 1, glm::value_ptr(eyePos));

	// Draw Quad
	drawModel(2);

	if (hit_red) speed = 0.0f;

	// Draw text
	std::string text = "Press R to restart";
	renderText(text, 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 0.0f));
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
	else if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{	if(!hit_red){
			move_left = true;
		}
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{	
		move_left = false;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{	if(!hit_red){
			move_right = true;
		}
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		move_right = false;
	}
	else if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		restart = true;
	}
	else if (key == GLFW_KEY_R && action == GLFW_RELEASE)
	{
		restart = false;
	}
}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		if (restart) {
			bunny_x = 0.0f;
			bunny_y = 0.0f;
			box_position_z = 0.0f;
			quad_position_z = 0.0f;

			bunny_rotation_angle = 0.0f;

			speed = 1.0f;

			yellow_cube_index = rand() % 3;

			move_left = false;
			move_right = false;

			hit_red = false;
			hit_yellow = false;
		}

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
	srand(static_cast<unsigned int>(time(NULL)));
	yellow_cube_index = rand() % 3;

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
