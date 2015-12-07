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
	relativePoints = new float[bufferByteSize];

	const float PI = 3.14159265f;
	float division = (2.0f * PI) / (float)nrTriangles;

	//triangles.push_back(glm::vec2(position.x, position.y));

	SetPointPos(relativePoints, 0.0f, 0.0f);
	SetPointPos(colors, color.x, color.y, color.z);

	for (int i = 0; i < nrTriangles; ++i)
	{
		//triangles.push_back(glm::vec2(position.x + cos(i * division), position.y + sin(i * division)));
		SetPointPos(relativePoints + (i + 1) * 4, radius * cos(i * division), radius * sin(i * division));
		SetPointPos(colors + (i + 1) * 4, color.x, color.y, color.z);
	}
	SetPointPos(relativePoints + (nrTriangles + 1) * 4, radius * cos(0.0f), radius * sin(0.0f));
	SetPointPos(colors + (nrTriangles + 1) * 4, color.x, color.y, color.z);

	UpdatePoints();
}

void Ball::UpdatePoints()
{
	SetPointPos(points, position.x, position.y);
	for (int i = 0; i <= nrTriangles; ++i)
	{
		SetPointPos(points + (i + 1) * 4, position.x + relativePoints[(i + 1) * 4], position.y + relativePoints[(i + 1) * 4 + 1]);
	}
}

void Ball::Draw()
{
	CreateVBO();
	glDrawArrays(GL_TRIANGLE_FAN, 0, nrTriangles + 2);
}

void Ball::Update()
{
	SetPosition(glm::vec3(position.x + velocity.x, position.y + velocity.y, 0.0f));
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

void Ball::SetPosition(glm::vec3 _position)
{
	position = _position;
	UpdatePoints();
}

glm::vec3 Ball::GetPosition() const
{
	return position;
}