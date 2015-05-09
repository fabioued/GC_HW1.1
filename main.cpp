#include "mesh.h"
#include "glut.h"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
#define TestScene 2
#define object_num 6

struct vertex {
	float x, y, z;
};

void scene();
void viewing();
void light();
void display();
void reshape(GLsizei, GLsizei);
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void rotate(int, int);

string skip;
int windowSize[2];
int xOrigin = -1, yOrigin = -1;
int rotate_value = 0;
int select = 0;

// scene
mesh* object[object_num];
vertex scale_value[object_num];
vertex rotation_axis_vector[object_num];
vertex transfer_vector[object_num];
float angle[object_num];

// viewing
vertex eye, vat, vup;
int fovy, dnear, dfar;
int viewport_x, viewport_y;
GLsizei viewport_width, viewport_height;

int main(int argc,char** argv)
{
	scene();
	viewing();
	glutInit(&argc, argv);
	glutInitWindowSize(viewport_width, viewport_height);
	glutInitWindowPosition(viewport_x, viewport_y);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("HW1");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	glutMainLoop();

	return 0;
}

void scene()
{
	ifstream input;
	if(TestScene == 1)
		input.open("Scene\\TestScene1\\scene.scene");
	if(TestScene == 2)
		input.open("Scene\\TestScene2\\scene2.scene");
	
	int i = 0;
	
	while(input >> skip && skip=="model")
	{
		input >> skip;
		if(TestScene == 1)
			skip = "Scene\\TestScene1\\" + skip;
		if(TestScene == 2)
			skip = "Scene\\TestScene2\\" + skip;
		object[i] = new mesh(skip.c_str());
		input >> scale_value[i].x >> scale_value[i].y >> scale_value[i].z;
		input >> angle[i] >> rotation_axis_vector[i].x >> rotation_axis_vector[i].y >> rotation_axis_vector[i].z;
		input >> transfer_vector[i].x >> transfer_vector[i].y >> transfer_vector[i].z;
		i++;
	}
	
	input.close();
}

void viewing()
{
	ifstream input;
	if(TestScene == 1)
		input.open("Scene\\TestScene1\\view.view");
	if(TestScene == 2)
		input.open("Scene\\TestScene2\\scene2.view");
	
	input >> skip >> eye.x >> eye.y >> eye.z;
	input >> skip >> vat.x >> vat.y >> vat.z;
	input >> skip >> vup.x >> vup.y >> vup.z;
	input >> skip >> fovy;
	input >> skip >> dnear;
	input >> skip >> dfar;
	input >> skip >> viewport_x >> viewport_y >> viewport_width >> viewport_height;
	
	input.close();
	
}

void light()
{	
	ifstream input;
	input.open("Scene\\1\\");
	if(TestScene == 1)
		input.open("Scene\\TestScene1\\light.light");
	if(TestScene == 2)
		input.open("Scene\\TestScene2\\scene2.light");
	
	glShadeModel(GL_SMOOTH);

	// z buffer enable
	glEnable(GL_DEPTH_TEST);

	// enable lighting
	glEnable(GL_LIGHTING);
	
	GLenum gl_light[8] = { GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3, GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7 };
	
	int i=0;
	float temp_x, temp_y, temp_z;
	
	while(input >> skip && skip == "light" && i<8)
	{
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_position[] = {temp_x, temp_y, temp_z, 1.0f};
		
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_ambient[] = {temp_x, temp_y, temp_z, 1.0f};
		
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_diffuse[] = {temp_x, temp_y, temp_z, 1.0f};
		
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_specular[] = {temp_x, temp_y, temp_z, 1.0f};
		
		// set light property
		glEnable(gl_light[i]);
		glLightfv(gl_light[i], GL_POSITION, light_position);
		glLightfv(gl_light[i], GL_DIFFUSE, light_diffuse);
		glLightfv(gl_light[i], GL_SPECULAR, light_specular);
		glLightfv(gl_light[i], GL_AMBIENT, light_ambient);
		i++;
	}
	
	input >> temp_x >> temp_y >> temp_z;
	GLfloat ambient[] = {temp_x, temp_y, temp_z};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

	input.close();
}

void display()
{
	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      //清除用color
	glClearDepth(1.0f);                        // Depth Buffer (就是z buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//這行把畫面清成黑色並且清除z buffer

	// viewport transformation
	glViewport(viewport_x, viewport_y, windowSize[0], windowSize[1]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLfloat)windowSize[0]/(GLfloat)windowSize[1], dnear, dfar);
	
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	eye.x, eye.y, eye.z,	// eye
				vat.x, vat.y, vat.z,    // center
				vup.x, vup.y, vup.z);   // up

	//注意light位置的設定，要在gluLookAt之後
	light();
	
	for(int k=0; k<object_num; k++)
	{
		glPushMatrix();//儲存現在的矩陣 (目前是modelview)
		glTranslatef(transfer_vector[k].x, transfer_vector[k].y, transfer_vector[k].z);
		glRotatef(angle[k], rotation_axis_vector[k].x, rotation_axis_vector[k].y, rotation_axis_vector[k].z);
		glScalef(scale_value[k].x, scale_value[k].y, scale_value[k].z);
		int lastMaterial = -1;
		for(size_t i=0;i < object[k]->fTotal;++i)
		{
			// set material property if this face used different material
			if(lastMaterial != object[k]->faceList[i].m)
			{
				lastMaterial = (int)object[k]->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT  , object[k]->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE  , object[k]->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR , object[k]->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &object[k]->mList[lastMaterial].Ns);

				//you can obtain the texture name by object[k]->mList[lastMaterial].map_Kd
				//load them once in the main function before mainloop
				//bind them in display function here
			}

			glBegin(GL_TRIANGLES);
			for (size_t j=0;j<3;++j)
			{
				//textex corrd. object[k]->tList[object[k]->faceList[i][j].t].ptr
				glNormal3fv(object[k]->nList[object[k]->faceList[i][j].n].ptr);
				glVertex3fv(object[k]->vList[object[k]->faceList[i][j].v].ptr);	
			}
			glEnd();
		}

		glPopMatrix();//拿stack中的矩陣到指定矩陣(modelview),會回到 I * ModelView
	}
	
	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	windowSize[0] = w;
	windowSize[1] = h;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		fovy += 1;
		break;
		
	case 's':
		fovy -= 1;
		break;

	case 'd':
		rotate_value += 1;
		rotate(rotate_value, 0);
		break;

	case 'a':
		rotate_value -= 1;
		rotate(rotate_value, 0);
		break;
		
	case'1':
		if(object_num >= 1) select = 1;
		else select = 0;
		break;
		
	case'2':
		if(object_num >= 2) select = 2;
		else select = 0;
		break;
		
	case'3':
		if(object_num >= 3) select = 3;
		else select = 0;
		break;

	case'4':
		if(object_num >= 4) select = 4;
		else select = 0;
		break;
		
	case'5':
		if(object_num >= 5) select = 5;
		else select = 0;
		break;
		
	case'6':
		if(object_num >= 6) select = 6;
		else select = 0;
		break;
		
	case'7':
		if(object_num >= 7) select = 7;
		else select = 0;
		break;
		
	case'8':
		if(object_num >= 8) select = 8;
		else select = 0;
		break;
		
	case'9':
		if(object_num >= 9) select = 9;
		else select = 0;
		break;
	
	default:
		break;
	}

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && select != 0)
	{
		if (state == GLUT_DOWN)
		{
			xOrigin = x;
			yOrigin = y;
		}
		else if(state == GLUT_UP)
		{
			transfer_vector[select-1].x += (x - xOrigin) / 50;
			transfer_vector[select-1].y -= (y - yOrigin) / 50;
			glutPostRedisplay();
		}
	}
}

void rotate(int x, int y)
{
	float theta, phi;
	float toRadian = 0.01745;
	float radius = sqrt(eye.x*eye.x + eye.y*eye.y + eye.z*eye.z);

	if(TestScene == 1) {
		theta = 82 - (360.0 / viewport_height)*y*2.0;
		phi = 360 - (360.0 / viewport_width)*x*2.0;
	}
	if(TestScene == 2) {
		theta = 54 - (360.0 / viewport_height)*y*2.0;
		phi = 43 - (360.0 / viewport_width)*x*2.0;
	}
	
	eye.x = radius * sin(theta*toRadian) * sin(phi*toRadian);
	eye.y = radius * cos(theta*toRadian);
	eye.z = radius * sin(theta*toRadian) * cos(phi*toRadian);

	glutPostRedisplay();
}