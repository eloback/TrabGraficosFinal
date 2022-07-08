/*
author: Eduardo Loback, Enzo Redivo
repo: https://github.com/eloback/comp-grafica-T1

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glew.h>
#include <GL/freeglut.h>    
#include <string>
#include <chrono>

using namespace std;

struct BitMapFile
{
    int sizeX;
    int sizeY;
    unsigned char* data;
};

struct vec3 {
    float x, y, z;
	
	vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	vec3(float x) : x(x), y(x), z(x) {}
};


struct vec2 {
    float x, y;

	vec2(float x, float y) : x(x), y(y) {}
	vec2(float x) : x(x), y(x) {}
};

struct ivec3 {
    int v, t, n;
	
	ivec3(int v, int t, int n) : v(v), t(t), n(n) {}
	ivec3(int v) : v(v), t(v), n(v) {}
};

// mM, mV, mP
struct mat4 {
	float m[4][4];

	mat4() {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m[i][j] = 0;
			}
		}
	}
};

mat4 mul(mat4 a, mat4 b) {
	mat4 c;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			c.m[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				c.m[i][j] += a.m[i][k] * b.m[k][j];
			}
		}
	}
	return c;
}

mat4 identity() {
	mat4 m;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m.m[i][j] = (i == j) ? 1 : 0;
		}
	}
	return m;
}

mat4 translateM(mat4 m, vec3 v) {
	mat4 t = identity();
	t.m[0][3] = v.x;
	t.m[1][3] = v.y;
	t.m[2][3] = v.z;
	return mul(m, t);
}

mat4 scaleM(mat4 m, vec3 v) {
	mat4 s = identity();
	s.m[0][0] = v.x;
	s.m[1][1] = v.y;
	s.m[2][2] = v.z;
	return mul(m, s);
}

mat4 rotateM(mat4 m, float angle, vec3 v) {
	mat4 r = identity();
	float c = cos(angle);
	float s = sin(angle);
	float t = 1 - c;
	r.m[0][0] = t * v.x * v.x + c;
	r.m[0][1] = t * v.x * v.y - s * v.z;
	r.m[0][2] = t * v.x * v.z + s * v.y;
	r.m[1][0] = t * v.x * v.y + s * v.z;
	r.m[1][1] = t * v.y * v.y + c;
	r.m[1][2] = t * v.y * v.z - s * v.x;
	r.m[2][0] = t * v.x * v.z - s * v.y;
	r.m[2][1] = t * v.y * v.z + s * v.x;
	r.m[2][2] = t * v.z * v.z + c;
	return mul(m, r);
}

vec3 normalize(vec3 v) {
	float l = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return vec3(v.x / l, v.y / l, v.z / l);
}

vec3 cross(vec3 a, vec3 b) {
	return vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

vec3 minusV (vec3 a, vec3 b) {
	return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

float dot(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
	vec3 f = normalize(minusV(center, eye));
	vec3 u = normalize(up);
	vec3 s = normalize(cross(f, u));
	u = cross(s, f);
	mat4 m;
	m.m[0][0] = s.x;
	m.m[1][0] = s.y;
	m.m[2][0] = s.z;
	m.m[0][1] = u.x;
	m.m[1][1] = u.y;
	m.m[2][1] = u.z;
	m.m[0][2] = -f.x;
	m.m[1][2] = -f.y;
	m.m[2][2] = -f.z;
	m.m[3][0] = -dot(s, eye);
	m.m[3][1] = -dot(u, eye);
	m.m[3][2] = dot(f, eye);
	m.m[3][3] = 1;
	return m;
}

mat4 perspective(float fov, float aspect, float nearPlane, float farPlane) {
	mat4 m;
	float f = 1 / tan(fov / 2);
	m.m[0][0] = f / aspect;
	m.m[1][1] = f;
	m.m[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
	m.m[2][3] = -1;
	m.m[3][2] = (2 * farPlane * nearPlane) / (nearPlane - farPlane);
	return m;
}
vec3 eye = vec3(0.0f, 0.0f, 5.0f);
vec3 center = vec3(0.0f);
vec3 up = vec3(0.0f, 1.0f, 0.0f);

float fov, aspect, nearPlane, farPlane, theta, scale_amount;
// Model Matrix, View Matrix, Perspective Matrix
mat4 mV = identity();
mat4 mP = identity();
mat4 mM = identity();

//globals
std::vector <vec3> vertices;
std::vector <vec2> textCoords;
std::vector <vec3> normals;
std::vector <ivec3> indexes;
vec3 mov{ 0, 0, 0 };
enum mode { cam = 0, movement = 1, rotation = 2, lightPos = 3 };
mode actual = cam;
bool rotating = true;
bool spotlight = false;
bool activeLights[3] = { true, false, false };

GLuint v, f; // vertex shader and fragment shader
GLuint p;

GLuint VAO;
GLuint verticesBuffer, textCoordsBuffer, normalsBuffer, facesBuffer;

static unsigned int texturas[2];

float intensidadeLuzAmbiente = 0.1;
float intensidadeLuzDifusa = 0.9;
float corLuz[3] = { 1, 1, 1 };
float posLuz[3] = { 0, 1, 0.25 };

unsigned int _model;
float rot;

vec3 diferrence(vec3 a, vec3 b) {
    return vec3{ a.x - b.x, a.y - b.y, a.z - b.z };
}

vec3 sum(vec3 a, vec3 b) {
    return vec3{ a.x + b.x, a.y + b.y, a.z + b.z };
}


std::vector<string> split(string str, char delimiter);
void createFace(string line);
vec2 parseVector2(string line);
vec3 parseVector3(string line);
void loadObj(std::string fileName);
void reshape(int w, int h);
void createVAOFromBufferInfo();
void setShaders();
void light();
void display(void);
void timer(int value);
void Initialize();
void generateNewIndexes();
BitMapFile* getBMPData(string filename);
void loadExternalTextures(std::string nome_arquivo, int id_textura);

void keyboard(unsigned char key, int x, int y);

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Carregar OBJ");
    Initialize();

	glGenTextures(2, texturas);
	loadExternalTextures("Textures/brickwall.bmp", 0);
	loadExternalTextures("Textures/brickwall_normal.bmp", 1);
	glEnable(GL_TEXTURE_2D);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(10, timer, 0);
    loadObj("data/capsule.obj"); // filename
	generateNewIndexes();
    setShaders();
    glutMainLoop();
    return 0;
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}



char* readStringFromFile(char* fn) {

    FILE* fp;
    char* content = NULL;
    int count = 0;

    if (fn != NULL) {
        fopen_s(&fp, fn, "rt");

        if (fp != NULL) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) {
                content = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;
}

// generate new vertices, textcoords and normals with indexes
void generateNewIndexes() {
	vector<vec3> newVertices;
	vector<vec2> newTextCoords;
	vector<vec3> newNormals;
    for (unsigned int i = 0; i < indexes.size(); i++) {
        unsigned int vertexIndex = indexes[i].v;
		unsigned int normalIndex = indexes[i].n;
		newVertices.push_back(vertices[vertexIndex]);
		newNormals.push_back(normals[normalIndex]);
		if (textCoords.size() > 0) {
			unsigned int textCoordIndex = indexes[i].t;
			newTextCoords.push_back(textCoords[textCoordIndex]);
		}
	}
	vertices = newVertices;
	textCoords = newTextCoords;
	normals = newNormals;
}

void createVAOfromVerticesNormalsTexts() {
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3) + textCoords.size() * sizeof(vec2) + normals.size() * sizeof(vec3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(vec3), &vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), textCoords.size() * sizeof(vec2), &textCoords[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3) + textCoords.size() * sizeof(vec2), normals.size() * sizeof(vec3), &normals[0]);

	size_t tam = sizeof(vec3) + sizeof(vec2) + sizeof(vec3);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, tam, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, tam, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, tam, (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void createVAOFromBufferInfo()
{	
    glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &verticesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticesBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
    if (textCoords.size() > 0) {
        glGenBuffers(1, &textCoordsBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, textCoordsBuffer);
        glBufferData(GL_ARRAY_BUFFER, textCoords.size() * sizeof(vec2), &textCoords[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
    }
	
	glGenBuffers(1, &normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	
    /*glGenBuffers(1, &facesBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facesBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, newIndices.size() * sizeof(ivec3), &newIndices[0], GL_STATIC_DRAW);
    glBindVertexArray(0);*/
}

void setShaders()
{
    cout << "setting shaders" << endl;
    char* vs = NULL, * fs = NULL, * fs2 = NULL;


    glewInit();

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    char vertex_shader[] = "simples.vert";
    char fragment_shader[] = "simples.frag";
    vs = readStringFromFile(vertex_shader);
    fs = readStringFromFile(fragment_shader);
    cout << "shader loaded" << endl;

    const char* vv = vs;
    const char* ff = fs;

    glShaderSource(v, 1, &vv, NULL);
    glShaderSource(f, 1, &ff, NULL);

    free(vs); free(fs);

    glCompileShader(v);
    glCompileShader(f);

	int result;
	glGetShaderiv(v, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int tam;
		glGetShaderiv(v, GL_INFO_LOG_LENGTH, &tam);
		char* mensagem = new char[tam];
		glGetShaderInfoLog(v, tam, &tam, mensagem);
		std::cout << mensagem << std::endl;
	}

	glGetShaderiv(f, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int tam;
		glGetShaderiv(f, GL_INFO_LOG_LENGTH, &tam);
		char* mensagem = new char[tam];
		glGetShaderInfoLog(f, tam, &tam, mensagem);
		std::cout << mensagem << std::endl;
	}

    p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
	
	createVAOFromBufferInfo();

	glUseProgram(p);
	
	cout<< "shaders set" << endl;
}

void display(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

	glPushMatrix();
	if(rotating) theta += 0.01f;
    mat4 scaled_mtx = scaleM(identity(), vec3(scale_amount));
    mat4 trans_mtx = translateM(scaled_mtx, vec3(0));
    mat4 rot_mtx = rotateM(trans_mtx, theta, vec3(0.0f, 1.0f, 0.0f));

	mV = lookAt(eye, center, up);
	
    mM = rot_mtx;
	

	// Upload mM to the GPU shader
	GLuint mLoc = glGetUniformLocation(p, "mM");
	glUniformMatrix4fv(mLoc, 1, GL_FALSE, &mM.m[0][0]);

	// Upload mP to the GPU shader
	GLuint pLoc = glGetUniformLocation(p, "mP");
	glUniformMatrix4fv(pLoc, 1, GL_FALSE, &mP.m[0][0]);

	// Upload mV to the GPU shader
	GLuint vLoc = glGetUniformLocation(p, "mV");
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, &mV.m[0][0]);


	int id_int_luz_amb = glGetUniformLocation(p, "luz_int_amb");
	glUniform1f(id_int_luz_amb, intensidadeLuzAmbiente);

	int id_int_luz_dif = glGetUniformLocation(p, "luz_int_dif");
	glUniform1f(id_int_luz_dif, intensidadeLuzDifusa);

	int id_cor_luz = glGetUniformLocation(p, "cor_luz");
	glUniform3f(id_cor_luz, corLuz[0], corLuz[1], corLuz[2]);

	int id_pos_luz = glGetUniformLocation(p, "pos_luz");
	glUniform3f(id_pos_luz, posLuz[0], posLuz[1], posLuz[2]);
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texturas[0]);
	int sp_texture = glGetUniformLocation(p, "textura");
	glUniform1i(sp_texture, 0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, texturas[1]);
	int sp_texture_normal = glGetUniformLocation(p, "textura_normal");
	glUniform1i(sp_texture_normal, 1);
	glBindTexture(GL_TEXTURE_2D, texturas[1]);
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glPopMatrix();
	glEnd();

    glFlush();
    glutSwapBuffers();
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(10, timer, 0);
}

void Initialize() {
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat diffuseLight[] = { 0.7, 0.7, 0.7, 1.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glEnable(GL_LIGHT0);

	GLfloat ambientLight[] = { 0.05, 0.05, 0.05, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

	GLfloat specularLight[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat spectre[] = { 1.0, 1.0, 1.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spectre);
	glMateriali(GL_FRONT, GL_SHININESS, 128);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_LIGHTING);


    theta = 0.0f;
    //scale_amount = 100.0f;
    //scale_amount = 40.0f;
    scale_amount = 0.01f;
    //scale_amount = 0.15f;
    //scale_amount = 3.0f;
	fov = 45.0f; aspect = 1.0f; nearPlane = 0.1f; farPlane = 100.0f;

	mV = lookAt(eye, center, up);
	mP = perspective(fov, aspect, nearPlane, farPlane);
}



std::vector<string> split(string str, char delimiter) {
    std::vector<string> internal;
    stringstream ss(str); // Turn the string into a stream.
    string tok;

    while (getline(ss, tok, delimiter)) {
        // evades delimiter if it is used in sequence
        if (tok.size() > 0) {
            internal.push_back(tok);
        }
    }

    return internal;
}

void createFace(string line) {
    std::vector<std::string> tokens = split(line, ' ');
    const int size = tokens.size();
    for (int i = 1; i < size; i++) {
        std::vector<std::string> sVertex = split(tokens[i], '/');
        int sVertexSize = sVertex.size();
        if (sVertexSize == 2) {
			indexes.push_back(ivec3{ stoi(sVertex[0]) - 1, 0, stoi(sVertex[1]) - 1 });
		}
        else {
			indexes.push_back(ivec3{ stoi(sVertex[0]) - 1, stoi(sVertex[1]) - 1, stoi(sVertex[2]) - 1 });
        }
    }
}


vec2 parseVector2(string line) {
    std::vector<std::string> tokens = split(line, ' ');
    return vec2{ stof(tokens[1]), stof(tokens[2]) };
}

vec3 parseVector3(string line) {
    std::vector<std::string> tokens = split(line, ' ');
    return vec3{ stof(tokens[1]), stof(tokens[2]), stof(tokens[3]) };
}

void loadObj(std::string fileName) {
    _model = glGenLists(1);
    glPointSize(2.0);
    std::ifstream file;
    file.open(fileName.c_str());

    std::string line;
    if (file.is_open())
    {
        while (file.good())
        {
            getline(file, line);

            unsigned int lineLength = line.length();

            //cout << line << endl;

            if (lineLength < 2)
                continue;

            const char* lineCStr = line.c_str();
            switch (lineCStr[0])
            {
            case 'v':
                if (lineCStr[1] == 't')
                    textCoords.push_back(parseVector2(line));
                else if (lineCStr[1] == 'n') {
                    normals.push_back(parseVector3(line));
                }
                else if (lineCStr[1] == ' ' || lineCStr[1] == '\t')
                    vertices.push_back(parseVector3(line));
                break;
            case 'f':
                createFace(line);
                break;
            default: break;
            };
        }
    }
    else
    {
        std::cerr << "Unable to load mesh: " << fileName << std::endl;
    }

    cout << "obj parsed" << endl;
}

void keyboard(unsigned char key, int x, int y) {
    //std::cout << key;

    static constexpr float modifier = 0.01;

	switch (key) {
	case 27:
		exit(0);
		break;
	case 's':
		if (actual == mode(0)) eye.y = eye.y - 10;
		else if (actual == mode(1)) mov.y = mov.y - 10;
		else if (actual == mode(3)) posLuz[1] -= 0.05;
		break;
	case 'w':
		if (actual == mode(0)) eye.y = eye.y + 10;
		else if (actual == mode(1)) mov.y = mov.y + 10;
		else if (actual == mode(3))  posLuz[1] += 0.05;
		break;
	case 'a':
		if (actual == mode(0)) eye.x = eye.x - 10;
		else if (actual == mode(1)) mov.x = mov.x - 1;
		else if (actual == mode(3)) posLuz[0] -= 0.05;
		break;
	case 'd':
		if (actual == mode(0)) eye.x = eye.x + 10;
		else if (actual == mode(1)) mov.x = mov.x + 1;
		else if (actual == mode(3)) posLuz[0] += 0.05;
		break;
	case 'f':
		if (actual == mode(0)) eye.z = eye.z - 10;
		else if (actual == mode(1)) mov.z = mov.z - 10;
		break;
	case 'g':
		if (actual == mode(0)) eye.z = eye.z + 10;
		else if (actual == mode(1)) mov.z = mov.z + 10;
		break;
	case 'q':
		if (actual == mode(0)) actual = mode(1);
		else if (actual == mode(1)) actual = mode(2);
		else if (actual == mode(2)) actual = mode(3);
		else actual = mode(0);
		cout << "actual mode is " << actual << endl;
		break;
	case 'r':
		rotating = !rotating;
		break;
	case '1':
		activeLights[0] = !activeLights[0];
		break;
	case '2':
		activeLights[1] = !activeLights[1];
		break;
	case '3':
		activeLights[2] = !activeLights[2];
		break;
	case 'z':
		intensidadeLuzAmbiente -= 0.05;
		break;
	case 'x':
		intensidadeLuzAmbiente += 0.05;
		break;
	case ',':
		scale_amount -= 0.05;
		break;
	case '.':
		scale_amount += 0.05;
		break;
	}

    if (key == '1' || key == '2' || key == '3') cout << "Active Lights: " << activeLights[0] << ", " << activeLights[1] << ", " << activeLights[2] << "." << endl;
}

// Funciona somente com bmp de 24 bits
BitMapFile* getBMPData(string filename)
{
	BitMapFile* bmp = new BitMapFile;
	unsigned int size, offset, headerSize;

	// Ler o arquivo de entrada
	ifstream infile(filename.c_str(), ios::binary);

	// Pegar o ponto inicial de leitura
	infile.seekg(10);
	infile.read((char*)&offset, 4);

	// Pegar o tamanho do cabeçalho do bmp.
	infile.read((char*)&headerSize, 4);

	// Pegar a altura e largura da imagem no cabeçalho do bmp.
	infile.seekg(18);
	infile.read((char*)&bmp->sizeX, 4);
	infile.read((char*)&bmp->sizeY, 4);

	// Alocar o buffer para a imagem.
	size = bmp->sizeX * bmp->sizeY * 24;
	bmp->data = new unsigned char[size];

	// Ler a informação da imagem.
	infile.seekg(offset);
	infile.read((char*)bmp->data, size);

	// Reverte a cor de bgr para rgb
	int temp;
	for (int i = 0; i < size; i += 3)
	{
		temp = bmp->data[i];
		bmp->data[i] = bmp->data[i + 2];
		bmp->data[i + 2] = temp;
	}

	return bmp;
}

void loadExternalTextures(std::string nome_arquivo, int id_textura)
{
	BitMapFile* image[1];

	image[0] = getBMPData(nome_arquivo);

	glBindTexture(GL_TEXTURE_2D, texturas[id_textura]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0,
		GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);
}