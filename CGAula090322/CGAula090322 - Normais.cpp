#include <iostream>
#include <fstream>
#include <vector>
#include <GL/freeglut.h>
#include <string>
using namespace std;

//globals

unsigned int elefante;
vector<vector<float>> vertices;
vector<vector<float>> normais;
vector<vector<int>> faces;
vector<vector<int>> faces_n;
float rot_ele_y;
float rot_ele_x;


void loadObj(string fname)
{
    int read;
    float x, y, z;
    ifstream arquivo(fname);
    if (!arquivo.is_open()) {
        cout << "arquivo nao encontrado";
        exit(1);
    }
    else {
        string tipo;
        while (arquivo >> tipo)
        {

            if (tipo == "v")
            {
                vector<float> vertice;
                float x, y, z;
                arquivo >> x >> y >> z;
                vertice.push_back(x);
                vertice.push_back(y);
                vertice.push_back(z);
                vertices.push_back(vertice);
            }

            if (tipo == "vn")
            {
                vector<float> normal;
                float x, y, z;
                arquivo >> x >> y >> z;
                normal.push_back(x);
                normal.push_back(y);
                normal.push_back(z);
                normais.push_back(normal);
            }

            if (tipo == "f")
            {
                vector<int> face, face_n;
                string x, y, z;
                arquivo >> x >> y >> z;

                int fp = stoi(x.substr(0, x.find("/"))) - 1;
                int fs = stoi(y.substr(0, y.find("/"))) - 1;
                int ft = stoi(z.substr(0, z.find("/"))) - 1;

                int fpn = stoi(x.substr(x.find("/") + 2, 100)) - 1;
                int fsn = stoi(y.substr(y.find("/") + 2, 100)) - 1;
                int ftn = stoi(z.substr(z.find("/") + 2, 100)) - 1;

                face.push_back(fp);
                face.push_back(fs);
                face.push_back(ft);
                faces.push_back(face);

                face_n.push_back(fpn);
                face_n.push_back(fsn);
                face_n.push_back(ftn);
                faces_n.push_back(face_n);
            }
        }
    }


    elefante = glGenLists(1);
    glPointSize(2.0);
    glNewList(elefante, GL_COMPILE);
    {
        glPushMatrix();
        glBegin(GL_TRIANGLES);
        glColor3f(0.0, 0.23, 0.87);
        for(int i = 0; i < faces.size(); i++)
        {
            vector<int> face = faces[i];

            glVertex3f(vertices[face[0]][0], vertices[face[0]][1], vertices[face[0]][2]);
            glVertex3f(vertices[face[1]][0], vertices[face[1]][1], vertices[face[1]][2]);

            //glVertex3f(vertices[face[1]][0], vertices[face[1]][1], vertices[face[1]][2]);
            glVertex3f(vertices[face[2]][0], vertices[face[2]][1], vertices[face[2]][2]);

            //glVertex3f(vertices[face[2]][0], vertices[face[2]][1], vertices[face[2]][2]);
            //glVertex3f(vertices[face[0]][0], vertices[face[0]][1], vertices[face[0]][2]);
            
        }
        glEnd();


        glPushMatrix();
        glBegin(GL_LINES);
        glColor3f(0, 0.5, 0);
        for (int i = 0; i < faces.size(); i++)
        {
            float escala_n = 30;

            vector<int> face = faces[i];
            vector<int> face_n = faces_n[i];

            float fim_linha_0_x = vertices[face[0]][0] + normais[face_n[0]][0] * escala_n;
            float fim_linha_0_y = vertices[face[0]][1] + normais[face_n[0]][1] * escala_n;
            float fim_linha_0_z = vertices[face[0]][2] + normais[face_n[0]][2] * escala_n;
            glVertex3f(vertices[face[0]][0], vertices[face[0]][1], vertices[face[0]][2]);
            glVertex3f(fim_linha_0_x, fim_linha_0_y, fim_linha_0_z);

            float fim_linha_1_x = vertices[face[1]][0] + normais[face_n[1]][0] * escala_n;
            float fim_linha_1_y = vertices[face[1]][1] + normais[face_n[1]][1] * escala_n;
            float fim_linha_1_z = vertices[face[1]][2] + normais[face_n[1]][2] * escala_n;
            glVertex3f(vertices[face[1]][0], vertices[face[1]][1], vertices[face[1]][2]);
            glVertex3f(fim_linha_1_x, fim_linha_1_y, fim_linha_1_z);

            float fim_linha_2_x = vertices[face[2]][0] + normais[face_n[2]][0] * escala_n;
            float fim_linha_2_y = vertices[face[2]][1] + normais[face_n[2]][1] * escala_n;
            float fim_linha_2_z = vertices[face[2]][2] + normais[face_n[2]][2] * escala_n;
            glVertex3f(vertices[face[2]][0], vertices[face[2]][1], vertices[face[2]][2]);
            glVertex3f(fim_linha_2_x, fim_linha_2_y, fim_linha_2_z);

        }
        glEnd();

    }
    glPopMatrix();
    glEndList();
    arquivo.close();

}


void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w / (GLfloat)h, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
}
void drawElephant()
{
    glPushMatrix();
    glTranslatef(0, -40.00, -105);
    glScalef(0.1, 0.1, 0.1);
    glRotatef(rot_ele_y, 0, 1, 0);
    glRotatef(rot_ele_x, 1, 0, 0);
    glCallList(elefante);
    glPopMatrix();
    
}
void display(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    drawElephant();
    glutSwapBuffers();
}

void timer(int value) {
    glutPostRedisplay();
    glutTimerFunc(10, timer, 0);
}

void teclado(unsigned char tecla, int x, int y) {
    if (tecla == 'd') {
        rot_ele_y = rot_ele_y + 1.6;
    }
    else if (tecla == 'a') {
        rot_ele_y = rot_ele_y - 1.6;
    }
    if (rot_ele_y > 360) rot_ele_y = rot_ele_y - 360;

    if (tecla == 's') {
        rot_ele_x = rot_ele_x + 1.6;
    }
    else if (tecla == 'w') {
        rot_ele_x = rot_ele_x - 1.6;
    }
    if (rot_ele_x > 360) rot_ele_x = rot_ele_x - 360;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 450);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Carregar OBJ");
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutTimerFunc(10, timer, 0);
    glutKeyboardFunc(teclado);

    loadObj("data/elepham.obj");
    glutMainLoop();
    return 0;
}