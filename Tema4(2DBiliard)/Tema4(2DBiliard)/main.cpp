#include <windows.h>
#include <cstdlib>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Ball.h"

using namespace std;

// pool table color RGB = (34, 139, 34), (0.1328125,  054296875, 0.1328125)

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 800

const double ORTHO_X = 600;
const double ORTHO_Y = 800;

const double eps = 1e-7;

vector< Ball > balls;
vector< Ball > blackholes;
Ball whiteBall;
Ball testBall;

float lastMouseX, lastMouseY;

bool inputState = true, leftButtonIsDown = false;
vector< float > inputPoints;

int fixedPointIndex = -1;
float RotAngle = 0.0f;
float *points, *colors;

int nrColission = 0;

glm::mat4 transfMatrix;

float CenterX = 0.0f, CenterY = 0.0f;

GLuint
VaoId,
VboId,
ColorBufferId,
VertexShaderId,
FragmentShaderId,
ProgramId;

const GLchar *VertexShader, *FragmentShader;

const GLchar* LoadShaderFromFile(char * filename){

	ifstream f(filename);
	string s;

	getline(f, s, char(EOF));

	f.close();

	const char *cShader = s.c_str();
	int l = strlen(cShader);

	GLchar* shader = new GLchar[l];
	strcpy(shader, cShader);

	return shader;
}

void DestroyVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ColorBufferId);
	glDeleteBuffers(1, &VboId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

bool CreateShaders(void)
{
	GLint compiled_ok;

	VertexShader = LoadShaderFromFile("vertex_shader.vert");
	FragmentShader = LoadShaderFromFile("fragment_shader.frag");

	VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
	glCompileShader(VertexShaderId);
	glGetShaderiv(VertexShaderId, GL_COMPILE_STATUS, &compiled_ok);
	if (!compiled_ok){ printf("Vertex shader didn't compile!\n"); return false; }

	FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
	glCompileShader(FragmentShaderId);

	glGetShaderiv(FragmentShaderId, GL_COMPILE_STATUS, &compiled_ok);
	if (!compiled_ok){ printf("Fragment shader didn't compile!\n"); return false; }

	ProgramId = glCreateProgram();
	glAttachShader(ProgramId, VertexShaderId);
	glAttachShader(ProgramId, FragmentShaderId);
	glLinkProgram(ProgramId);
	glUseProgram(ProgramId);
	return true;
}

void DestroyShaders(void)
{
	glUseProgram(0);

	glDetachShader(ProgramId, VertexShaderId);
	glDetachShader(ProgramId, FragmentShaderId);

	glDeleteShader(FragmentShaderId);
	glDeleteShader(VertexShaderId);

	glDeleteProgram(ProgramId);
}

void InitScene()
{
	whiteBall = Ball(10.0f, 20, glm::vec3(0.0f, -200.0f, 0.0f));

	testBall = Ball(10.0f, 20, glm::vec3(0.0f, 200.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	for (int i = 0; i < 6; ++i)
	{
		blackholes.push_back(Ball(20.0f, 20, glm::vec3((i/3) * 600.0f - 300.0f, (i%3 - 1) * 400.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	}

	for (int i = 0; i < 15; ++i)
	{
		balls.push_back(Ball(20.0f, 20, glm::vec3(rand() % 600 - 300, rand() % 800 - 400, 0.0f)));
	}

	points = new float[8];
	colors = new float[8];

	points[2] = points[6] = 0.0f;
	points[3] = points[7] = 1.0f;

	colors[0] = colors[1] = colors[3] = colors[4] = colors[5] = colors[7] = 1.0f;
	colors[2] = colors[6] = 0.0f;
}

void CreateVBO(void)
{
	// se creeaza un buffer nou se seteaza ca buffer curent si punctele sunt "copiate" in bufferul curent
	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, 16 * 2, points, GL_STATIC_DRAW);

	// se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	// se activeaza lucrul cu atribute; atributul 0 = pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// un nou buffer, pentru culoare
	glGenBuffers(1, &ColorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
	glBufferData(GL_ARRAY_BUFFER, 16 * 2, colors, GL_STATIC_DRAW);
	// atributul 1 =  culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}


bool Initialize(void)
{
	if (glewInit() != GLEW_OK)
	{
		printf("%s\n", "GLEW has failed to initialize");
		return false;
	}
	if (CreateShaders() == false)
	{
		printf("%s\n", "Shader could not be created");
		return false;
	}

	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0 / ORTHO_X, 2.0 / ORTHO_Y, 0));
	glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), RotAngle, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 translMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(CenterX - 0.0, CenterY - 0.0, 0.0));
	transfMatrix = scaleMatrix * rotMatrix * translMatrix;
	GLint loc = glGetUniformLocation(ProgramId, "matTransform");
	if (loc != -1)
	{
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transfMatrix));
	}

	glClearColor(0.1328125f, 0.54296875f, 0.1328125f, 0.0f); // culoarea de fond a ecranului

	InitScene();
	return true;
}

void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

void MousePassiveMotionFunction(int x, int y)
{
	
	float x_coord, y_coord;

	float xp = (float)x / (float)(WINDOW_WIDTH);
	float yp = (float)(WINDOW_HEIGHT - y) / (float)(WINDOW_HEIGHT);

	x_coord = xp * ORTHO_X - ORTHO_X / 2;
	y_coord = yp * ORTHO_Y - ORTHO_Y / 2;

	// cout << x_coord << ' ' << y_coord << '\n';

	points[0] = whiteBall.position.x;
	points[1] = whiteBall.position.y;

	points[4] = x_coord;
	points[5] = y_coord;

	
	/*
	lastMouseX = (float)x / (float)(2 * ORTHO_X);
	lastMouseY = (float)(WINDOW_HEIGHT - y) / (float)(2 * ORTHO_X);

	if (inputState == false && leftButtonIsDown == true)
	{ 
	if (fixedPointIndex == -1)
	{
	for (int i = 0; i < K; ++i)
	if (fabs(points[i * 4] - lastMouseX) < 0.5f && fabs(points[i * 4 + 1] - lastMouseY) < 0.5f)
	{
	fixedPointIndex = i;
	break;
	}
	//fixedPointIndex = 2;
	}
	else
	{
	points[fixedPointIndex * 4] = lastMouseX;
	points[fixedPointIndex * 4 + 1] = lastMouseY;

	if (fixedPointIndex != 0 && fixedPointIndex != K - 1)
	{
	points[(fixedPointIndex + 1) * 4] = lastMouseX;
	points[(fixedPointIndex + 1) * 4 + 1] = lastMouseY;
	}
	}
	//compute();
	glutPostRedisplay();
	}
	*/
	glutPostRedisplay();
}

void MouseFunction(int button, int state, int x, int y)
{
	float x_coord, y_coord;

	float xp = (float)x / (float)(WINDOW_WIDTH);
	float yp = (float)(WINDOW_HEIGHT - y) / (float)(WINDOW_HEIGHT);

	x_coord = xp * ORTHO_X - ORTHO_X / 2;
	y_coord = yp * ORTHO_Y - ORTHO_Y / 2;

	//cout << x_coord << ' ' << y_coord << '\n';

	//x_coord = (2.0f * x / (float)(WINDOW_WIDTH)) - 1.0f;
	//y_coord = (2.0f * y / (float)(WINDOW_HEIGHT)) - 1.0f;

	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			//balls.push_back(Ball(10, 10, glm::vec3(x_coord, y_coord, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
			glm::vec2 vel = glm::vec2(points[4] - points[0], points[5] - points[1]);
			vel = glm::normalize(vel);

			whiteBall.velocity = glm::vec3(vel.x, vel.y, 0.0f);

			//cout << vel.x << vel.y << '\n';
			
			//glutPostRedisplay();

			/*
			glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), RotAngle, glm::vec3(0.0, 0.0, 1.0));
			glm::mat4 translMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-CenterX, -CenterY, 0.0f));
			glm::mat4 revTranslMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(CenterX, CenterY, 0.0f));

			for (int i = 1; i <= 4; ++i)
			{
				glm::vec4 pos(points[i * 4], points[i * 4 + 1], 0.0f, 1.0f);
				pos = revTranslMatrix * rotMatrix * translMatrix * pos;
				points[i * 4] = pos.x;
				points[i * 4 + 1] = pos.y;
			}
			RotAngle = 0;

			leftButtonIsDown = true;
			CenterX = x_coord;
			CenterY = y_coord;

			points[0] = x_coord;
			points[1] = y_coord;
			points[2] = 0.0f;
			points[3] = 1.0f;

			colors[0] = 1.0f;
			colors[1] = 0.0f;
			colors[2] = 0.0f;
			colors[3] = 1.0f;
			*/
		}
		else
		{
			leftButtonIsDown = false;
			fixedPointIndex = -1;
		}
	}
}

void KeyboardFunction(unsigned char key, int x, int y)
{
	/*
	bool change = 0;
	switch (key)
	{
	case 'a': RotAngle = RotAngle + 0.05; change = 1; break;
	case 'd': RotAngle = RotAngle - 0.05; change = 1; break;
	}

	//if (RotAngle < -eps) RotAngle += 360.0f;
	//if (RotAngle >= 390.0f) RotAngle -= 360.0f;

	if (change)
	{
		float dx = CenterX - 0.0f;
		float dy = CenterY - 0.0f;

		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0 / ORTHO_X, 2.0 / ORTHO_Y, 0));
		glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), RotAngle, glm::vec3(0.0, 0.0, 1.0));
		glm::mat4 translMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-dx, -dy, 0.0f));
		glm::mat4 revTranslMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(dx, dy, 0.0f));
		transfMatrix = scaleMatrix * revTranslMatrix * rotMatrix * translMatrix;
		GLint loc = glGetUniformLocation(ProgramId, "matTransform");
		if (loc != -1)
		{
			glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transfMatrix));
		}
		glutPostRedisplay();
	}
	*/
}

void desen(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0 / ORTHO_X, 2.0 / ORTHO_Y, 0));
	transfMatrix = scaleMatrix;
	GLint loc = glGetUniformLocation(ProgramId, "matTransform");
	if (loc != -1)
	{
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transfMatrix));
	}

	whiteBall.Draw();
	testBall.Draw();

	for (auto &ball : blackholes)
	{
		ball.Draw();
	}

	for (auto &ball : balls)
	{
		ball.Draw();
	}

	CreateVBO();
	glDrawArrays(GL_LINES, 0, 2);

	/*
	scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0 / ORTHO_X, 2.0 / ORTHO_Y, 0));
	glm::mat4 rotMatrix = glm::rotate(glm::mat4(1.0f), RotAngle, glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 translMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-CenterX, -CenterY, 0.0f));
	glm::mat4 revTranslMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(CenterX, CenterY, 0.0f));
	transfMatrix = scaleMatrix * revTranslMatrix * rotMatrix * translMatrix;

	loc = glGetUniformLocation(ProgramId, "matTransform");
	if (loc != -1)
	{
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transfMatrix));
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 1, 4);
	//glLineWidth(2.0f);
	//glDrawArrays(GL_LINES, 0, K);
	//glPointSize(5.0f);
	*/
	glFlush();
	
}

void update()
{
	points[0] = whiteBall.position.x;
	points[1] = whiteBall.position.y;

	whiteBall.Update();
	//testBall.Update();
	for (auto & ball : balls)
	{
		ball.Update();
	}

	for (size_t b1 = 0; b1 < balls.size(); ++b1)
	{
		for (size_t b2 = b1 + 1; b2 < balls.size(); ++b2)
		if (b1 != b2)
		{
			if (balls[b1].collidesWith(balls[b2]))
				balls[b1].ComputeCollisionPhysics(balls[b2]);
		}
	}

	for (auto & ball : balls)
	{
		if (ball.collidesWith(whiteBall))
			ball.ComputeCollisionPhysics(whiteBall);
	}

	//
	//whiteBall.SetPosition(glm::vec3(whiteBall.position.x + 0.1f * whiteBall.velocity.x, whiteBall.position.y += 0.1f * whiteBall.velocity.y, 0.0f));
	
	//cout << whiteBall.position.x << ' ' << whiteBall.position.y << '\n';
	/*
	if (whiteBall.collidesWith(testBall))
	{
		whiteBall.ComputeCollisionPhysics(testBall);
		++nrColission;
		cout << "collides " << nrColission << "!!!\n";
	}*/

	if (whiteBall.position.x - whiteBall.radius <= -ORTHO_X / 2)
	{
		whiteBall.ComputeSurfaceCollisionPhysics(glm::vec3(-1.0f, 1.0f, 0.0f));
		//cout << "achtung!!111";
	}
	else
	if (whiteBall.position.y - whiteBall.radius <= -ORTHO_Y / 2)
	{
		whiteBall.ComputeSurfaceCollisionPhysics(glm::vec3(1.0f, -1.0f, 0.0f));
		//cout << "achtung!!222";
	}
	else
	if (whiteBall.position.x + whiteBall.radius >= ORTHO_X / 2)
	{
		whiteBall.ComputeSurfaceCollisionPhysics(glm::vec3(-1.0f, 1.0f, 0.0f));
		//cout << "achtung!!333";
	}
	else
	if (whiteBall.position.y + whiteBall.radius >= ORTHO_Y / 2)
	{
		whiteBall.ComputeSurfaceCollisionPhysics(glm::vec3(1.0f, -1.0f, 0.0f));
		//cout << "achtung!!444";
	}

	for (auto & ball : balls)
	{
		if (ball.position.x - ball.radius <= -ORTHO_X / 2)
		{
			ball.ComputeSurfaceCollisionPhysics(glm::vec3(-1.0f, 1.0f, 0.0f));
			//cout << "achtung!!111";
		}
		else
		if (ball.position.y - ball.radius <= -ORTHO_Y / 2)
		{
			ball.ComputeSurfaceCollisionPhysics(glm::vec3(1.0f, -1.0f, 0.0f));
			//cout << "achtung!!222";
		}
		else
		if (ball.position.x + ball.radius >= ORTHO_X / 2)
		{
			ball.ComputeSurfaceCollisionPhysics(glm::vec3(-1.0f, 1.0f, 0.0f));
			//cout << "achtung!!333";
		}
		else
		if (ball.position.y + ball.radius >= ORTHO_Y / 2)
		{
			ball.ComputeSurfaceCollisionPhysics(glm::vec3(1.0f, -1.0f, 0.0f));
			//cout << "achtung!!444";
		}
	}

	glutPostRedisplay();
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(400, 25);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Biliard 2D");
	if (!Initialize())
	{
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glutMouseFunc(MouseFunction);
	//glutMotionFunc(MousePassiveMotionFunction);

	glutKeyboardFunc(KeyboardFunction);
	glutPassiveMotionFunc(MousePassiveMotionFunction);
	glutDisplayFunc(desen);
	glutIdleFunc(update);
	glutMainLoop();

	getchar();
}