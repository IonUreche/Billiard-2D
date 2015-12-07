#include "Ball.h"


Ball::Ball(float _radius, int _nrTriangles, glm::vec3 _pos, glm::vec3 _color)
{
	nrTriangles = _nrTriangles;
	radius = _radius;
	color = _color;
	position = _pos;
	BuildCircle();
}


Ball::~Ball()
{
}

void Ball::SetPointPos(float *arr, float x, float y, float z, float d)
{
	*arr = x;
	*(arr + 1) = y;
	*(arr + 2) = z;
	*(arr + 3) = d;
}

void Ball::BuildCircle()
{
	if (nrTriangles < 0) return;

	int nr = nrTriangles + 2;
	bufferByteSize = 16 * nr;
	points = new float[bufferByteSize];
	colors = new float[bufferByteSize];

	const float PI = 3.14159265f;
	float division = (2.0f * PI) / (float)nrTriangles;

	//triangles.push_back(glm::vec2(position.x, position.y));

	SetPointPos(points, position.x, position.y);
	SetPointPos(colors, color.x, color.y, color.z);

	for (int i = 0; i < nrTriangles; ++i)
	{
		//triangles.push_back(glm::vec2(position.x + cos(i * division), position.y + sin(i * division)));
		SetPointPos(points + (i + 1) * 4, position.x + radius * cos(i * division), position.y + radius * sin(i * division));
		SetPointPos(colors + (i + 1) * 4, color.x, color.y, color.z);
	}
	SetPointPos(points + (nrTriangles + 1) * 4, position.x + radius * cos(0.0f), position.y + radius * sin(0.0f));
	SetPointPos(colors + (nrTriangles + 1) * 4, color.x, color.y, color.z);
}

void Ball::draw()
{
	CreateVBO();
	glDrawArrays(GL_TRIANGLE_FAN, 0, nrTriangles + 2);
}

void Ball::CreateVBO()
{
	
	// se creeaza un buffer nou se seteaza ca buffer curent si punctele sunt "copiate" in bufferul curent
	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, bufferByteSize, points, GL_STATIC_DRAW);

	// se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	// se activeaza lucrul cu atribute; atributul 0 = pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// un nou buffer, pentru culoare
	glGenBuffers(1, &ColorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
	glBufferData(GL_ARRAY_BUFFER, bufferByteSize, colors, GL_STATIC_DRAW);
	// atributul 1 =  culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

void Ball::DestroyVBO()
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ColorBufferId);
	glDeleteBuffers(1, &VboId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}