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

int WINDOW_WIDTH = 600;
int WINDOW_HEIGHT = 800;

const double ORTHO_X = 600;
const double ORTHO_Y = 800;

const double eps = 1e-7;

vector< Ball > balls;
vector< Ball > blackholes;
Ball whiteBall;
Ball testBall;

int ballCounter = 0;

vector< glm::vec3 > ballsColors;

float lastMouseX, lastMouseY;

bool inputState = true, leftButtonIsDown = false;
vector< float > inputPoints;

int fixedPointIndex = -1;
float RotAngle = 0.0f;
float *points, *colors;

int currTime, timebase, lastRenderTime;

int nrColission = 0, updCounter = 0;

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
	whiteBall = Ball(++ballCounter, 10.0f, 20, glm::vec2(0.0f, -200.0f));

	//testBall = Ball(10.0f, 20, glm::vec2(0.0f, 200.0f), glm::vec3(0.0f, 0.0f, 0.0f));

	for (int i = 0; i < 6; ++i)
	{
		blackholes.push_back(Ball(++ballCounter, 30.0f, 20, glm::vec2((i / 3) * 600.0f - 300.0f, (i % 3 - 1) * 400.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
	}
	//1
	ballsColors.push_back(glm::vec3(255, 204, 42));
	ballsColors.push_back(glm::vec3(0, 86, 171));
	ballsColors.push_back(glm::vec3(0, 81, 170));
	ballsColors.push_back(glm::vec3(85, 49, 61));
	ballsColors.push_back(glm::vec3(38, 47, 104));

	// 2
	ballsColors.push_back(glm::vec3(43, 52, 131));
	ballsColors.push_back(glm::vec3(84, 54, 66));
	ballsColors.push_back(glm::vec3(0, 0, 0));
	ballsColors.push_back(glm::vec3(254, 206, 46));

	// 3
	ballsColors.push_back(glm::vec3(229, 112, 69));
	ballsColors.push_back(glm::vec3(250, 136, 74));
	ballsColors.push_back(glm::vec3(238, 97, 90));

	// 4
	ballsColors.push_back(glm::vec3(33, 90, 83));
	ballsColors.push_back(glm::vec3(205, 77, 76));

	// 5
	ballsColors.push_back(glm::vec3(1, 128, 107));

	for (auto & ball : ballsColors)
	{
		ball = glm::vec3((float)ball.x / 256.0f, (float)ball.y / 256.0f, (float)ball.z / 256.0f);
	}

	float startX = 0, startY = 100, r = 10;
	int k = 0;

	for (int i = -8; i <= 0; i += 2)
	{
		int nriter = (i + 8) / 2;
		for (int j = -4 + nriter; j <= 4 - nriter; j += 2)
		{
			balls.push_back(Ball(++ballCounter, 10.0f, 20, glm::vec2(startX + j * 1.05f * r, startY - i * 1.05f * r), ballsColors[k++]));
		}
	}
	/*
	for (int i = 0; i < 15; ++i)
	{
		balls.push_back(Ball(20.0f, 20, glm::vec2(rand() % 600 - 300, rand() % 800 - 400)));
	}
	*/
	points = new float[4];
	colors = new float[6];

	points[0];

	colors[0] = colors[1] = colors[3] = colors[4] = 1.0f;
	colors[2] = colors[5] = 0.0f;

	currTime = timebase = lastRenderTime = glutGet(GLUT_ELAPSED_TIME);
}

void CreateVBO(void)
{
	// se creeaza un buffer nou se seteaza ca buffer curent si punctele sunt "copiate" in bufferul curent
	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, 8 * 2, points, GL_STATIC_DRAW);

	// se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	// se activeaza lucrul cu atribute; atributul 0 = pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// un nou buffer, pentru culoare
	glGenBuffers(1, &ColorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
	glBufferData(GL_ARRAY_BUFFER, 12 * 2, colors, GL_STATIC_DRAW);
	// atributul 1 =  culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
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

	points[2] = x_coord;
	points[3] = y_coord;

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
			glm::vec2 vel = glm::vec2(points[2] - points[0], points[3] - points[1]);
			float fact = glm::length(vel) / 200.0f;
			vel = glm::normalize(vel);

			whiteBall.velocity = glm::vec2(vel.x, vel.y);
			whiteBall.velocityEnergyInPercents = glm::min(fact, 1.0f);
			whiteBall.lastCollisionId = -10;
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
	//testBall.Draw();

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

	lastRenderTime = glutGet(GLUT_ELAPSED_TIME);
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
	glutSwapBuffers();
}

void update()
{
	currTime = glutGet(GLUT_ELAPSED_TIME);
	int deltatime = currTime - timebase;

	if (deltatime < 1)
	{
		return;
	}

	timebase = currTime;

	++updCounter;
	//cout << updCounter << '\n';
	
	points[0] = whiteBall.position.x;
	points[1] = whiteBall.position.y;

	for (size_t b1 = 0; b1 < balls.size(); ++b1)
	{
		for (size_t b2 = b1 + 1; b2 < balls.size(); ++b2)
		if (b1 != b2 && (balls[b1].lastCollisionId != balls[b2].id || balls[b2].lastCollisionId != balls[b1].id))
		{
			if (balls[b1].collidesWith(balls[b2]))
			{
				balls[b1].ComputeCollisionPhysics(balls[b2]);
				balls[b1].Update(deltatime);
				balls[b2].Update(deltatime);
			}
				
		}
	}

	for (auto & ball : balls)
	{
		if (ball.collidesWith(whiteBall) &&
			(ball.lastCollisionId != whiteBall.id || whiteBall.lastCollisionId != ball.id))
		{
			ball.ComputeCollisionPhysics(whiteBall);
			ball.Update(deltatime);
			whiteBall.Update(deltatime);
		}
	}

	if ((whiteBall.position.x - whiteBall.radius <= -ORTHO_X / 2 && whiteBall.lastCollisionId != -1) ||
		(whiteBall.position.x + whiteBall.radius >= ORTHO_X / 2 && whiteBall.lastCollisionId != -2))
	{
		if (whiteBall.position.x - whiteBall.radius <= -ORTHO_X / 2)
		{
			whiteBall.lastCollisionId = -1;
		}
			
		if (whiteBall.position.x + whiteBall.radius >= ORTHO_X / 2)
		{
			whiteBall.lastCollisionId = -2;
		}
			
		whiteBall.ComputeSurfaceCollisionPhysics(glm::vec2(-1.0f, 1.0f));
		whiteBall.Update(deltatime);
	}
	else
	if ((whiteBall.position.y - whiteBall.radius <= -ORTHO_Y / 2 && whiteBall.lastCollisionId != -3) ||
		(whiteBall.position.y + whiteBall.radius >= ORTHO_Y / 2 && whiteBall.lastCollisionId != -4))
	{
		if (whiteBall.position.y - whiteBall.radius <= -ORTHO_Y / 2)
		{
			whiteBall.lastCollisionId = -3;
		}
			
		if (whiteBall.position.y + whiteBall.radius >= ORTHO_Y / 2)
		{
			whiteBall.lastCollisionId = -4;
		}

		whiteBall.ComputeSurfaceCollisionPhysics(glm::vec2(1.0f, -1.0f));
		whiteBall.Update(deltatime);
	}

	for (auto & ball : balls)
	{
		if ((ball.position.x - ball.radius <= -ORTHO_X / 2 && ball.lastCollisionId != -1) ||
			(ball.position.x + ball.radius >= ORTHO_X / 2 && ball.lastCollisionId != -2))
		{
			if (ball.position.x - ball.radius <= -ORTHO_X / 2)
			{
				ball.lastCollisionId = -1;
			}

			if (ball.position.x + ball.radius >= ORTHO_X / 2)
			{
				//ball.lastCollisionId = -2;
			}

			ball.ComputeSurfaceCollisionPhysics(glm::vec2(-1.0f, 1.0f));
			ball.Update(deltatime);
		}
		else
		if ((ball.position.y - ball.radius <= -ORTHO_Y / 2 && ball.lastCollisionId != -3) ||
			(ball.position.y + ball.radius >= ORTHO_Y / 2 && ball.lastCollisionId != -4))
		{
			if (ball.position.y - ball.radius <= -ORTHO_Y / 2)
			{
				ball.lastCollisionId = -3;
			}

			if (ball.position.y + ball.radius >= ORTHO_Y / 2)
			{
				ball.lastCollisionId = -4;
			}
			ball.ComputeSurfaceCollisionPhysics(glm::vec2(1.0f, -1.0f));
			ball.Update(deltatime);
		}
	}
	
	whiteBall.Update(deltatime);

	// Check for collision with black holes
	for (auto & b : blackholes)
	{
		for (int i = 0; i < balls.size(); ++i)
		if (b.collidesWith(balls[i]))
		{
			balls.erase(balls.begin() + i);
			--i;
		}

		if (b.collidesWith(whiteBall))
		{
			whiteBall.position = glm::vec2(0.0f, -200.0f);
			whiteBall.velocity = glm::vec2(0.0f, 0.0f);
			whiteBall.velocityEnergyInPercents = 0.0f;
		}
	}

	for (auto & ball : balls)
	{
		ball.Update(deltatime);
	}

	if (currTime - lastRenderTime >= 17) // 60 fps
	{
		glutPostRedisplay();
		lastRenderTime = currTime;
	}
}

void reshapeFunc(int w, int h)
{
	WINDOW_WIDTH = w;
	WINDOW_HEIGHT = h;
}

void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
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
	glutReshapeFunc(reshapeFunc);
	glutMainLoop();

	getchar();
}