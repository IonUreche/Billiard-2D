#include "Ball.h"
#include <algorithm>

Ball::Ball(float _radius, int _nrTriangles, glm::vec2 _pos, glm::vec3 _color)
{
	nrTriangles = _nrTriangles;
	radius = _radius;
	color = _color;
	position = _pos;
	BuildCircle();
	velocityEnergyInPercents = 1.0f;
}


Ball::~Ball()
{
}

void Ball::SetPointPos(float *arr, float x, float y)
{
	*arr = x;
	*(arr + 1) = y;
}

void Ball::SetPointColor(float *arr, float x, float y, float z)
{
	*arr = x;
	*(arr + 1) = y;
	*(arr + 2) = z;
}

void Ball::BuildCircle()
{
	if (nrTriangles < 0) return;

	int nr = nrTriangles + 2;
	bufferByteSize = 16 * nr;
	points = new float[2 * nr];
	colors = new float[3 * nr];
	relativePoints = new float[2 * nr];

	const float PI = 3.14159265f;
	float division = (2.0f * PI) / (float)nrTriangles;

	SetPointPos(relativePoints, 0.0f, 0.0f);
	SetPointColor(colors, color.x, color.y, color.z);

	for (int i = 0; i < nrTriangles; ++i)
	{
		//triangles.push_back(glm::vec2(position.x + cos(i * division), position.y + sin(i * division)));
		SetPointPos(relativePoints + (i + 1) * 2, radius * cos(i * division), radius * sin(i * division));
		SetPointColor(colors + (i + 1) * 3, color.x, color.y, color.z);
	}
	SetPointPos(relativePoints + (nrTriangles + 1) * 2, radius * cos(0.0f), radius * sin(0.0f));
	SetPointColor(colors + (nrTriangles + 1) * 3, color.x, color.y, color.z);

	UpdatePoints();
}

void Ball::UpdatePoints()
{
	SetPointPos(points, position.x, position.y);
	for (int i = 0; i <= nrTriangles; ++i)
	{
		SetPointPos(points + (i + 1) * 2, position.x + relativePoints[(i + 1) * 2], position.y + relativePoints[(i + 1) * 2 + 1]);
	}
}

void Ball::Draw()
{
	CreateVBO();
	glDrawArrays(GL_TRIANGLE_FAN, 0, nrTriangles + 2);
}

void Ball::Update(int deltaTime)
{
	SetPosition(glm::vec2(position.x + deltaTime * 0.3f * velocityEnergyInPercents * velocity.x, position.y + deltaTime * 0.3f * velocityEnergyInPercents * velocity.y));
	velocityEnergyInPercents = std::max(velocityEnergyInPercents - 0.001f, 0.0f);
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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// un nou buffer, pentru culoare
	glGenBuffers(1, &ColorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
	glBufferData(GL_ARRAY_BUFFER, bufferByteSize, colors, GL_STATIC_DRAW);
	// atributul 1 =  culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
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

void Ball::SetPosition(glm::vec2 _position)
{
	position = _position;
	UpdatePoints();
}

glm::vec2 Ball::GetPosition() const
{
	return position;
}

bool Ball::collidesWith(Ball & ball)
{
	return glm::distance(position, ball.GetPosition()) <= radius + ball.radius;
}

void Ball::ComputeCollisionPhysics(Ball & ball)
{
	glm::vec2 d = position - ball.position;
	glm::vec2 nd = glm::normalize(d);
	float k = glm::dot(velocity - ball.velocity, d);
	glm::vec2 v1p = velocity - k * nd;

	d = ball.position - position;
	nd = glm::normalize(d);
	k = glm::dot(ball.velocity - velocity, d);
	glm::vec2 v2p = ball.velocity - k * nd;

	velocity = glm::normalize(v1p);
	ball.velocity = glm::normalize(v2p);

	// lose some energy
	float maxEnergy = std::max(velocityEnergyInPercents, ball.velocityEnergyInPercents);
	maxEnergy = std::max(maxEnergy - 0.01f, 0.0f);
	
	velocityEnergyInPercents = maxEnergy;
	ball.velocityEnergyInPercents = maxEnergy;
}

void Ball::ComputeSurfaceCollisionPhysics(glm::vec2 normal)
{
	//normal = -normal;
	//glm::vec3 d = normal * radius;
	//glm::vec3 nd = glm::normalize(d);
	//float k = glm::dot(velocity - normal, d);
	//glm::vec3 v1p = velocity - k * nd;
	velocity = glm::vec2(velocity.x * normal.x, velocity.y * normal.y);
	velocityEnergyInPercents = std::max(velocityEnergyInPercents - 0.01f, 0.0f);
}