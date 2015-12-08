#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm\vec2.hpp>
#include <glm\vec3.hpp>
#include <vector>

class Ball
{
public:
	glm::vec3 color;
	glm::vec2 position;
	glm::vec2 velocity;

	float radius;
	int nrTriangles;

	float *points;
	float *colors;
	float *relativePoints;

	float velocityEnergyInPercents;

	int bufferByteSize;

	Ball(float _radius = 10, int _nrTriangles = 20, glm::vec2 _pos = glm::vec2(0.0f, 0.0f), glm::vec3 _color = glm::vec3(1.0f, 1.0f, 1.0f));
	~Ball();

	void Update(int deltaTime);
	void Draw();

	bool collidesWith(Ball & ball);
	void ComputeCollisionPhysics(Ball & ball);
	void ComputeSurfaceCollisionPhysics(glm::vec2 normal);

	void SetPosition(glm::vec2 _position);
	glm::vec2 GetPosition() const;

private:

	GLuint
		VaoId,
		VboId,
		ColorBufferId,
		VertexShaderId,
		FragmentShaderId,
		ProgramId;

	void UpdatePoints();
	
	void BuildCircle();
	void SetPointPos(float *arr, float x, float y);
	void SetPointColor(float *arr, float x, float y, float z);

	void CreateVBO();
	void DestroyVBO();
};

