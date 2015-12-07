#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm\glm.hpp>
#include <glm\vec3.hpp>
#include <vector>

class Ball
{
public:
	glm::vec3 color;
	glm::vec3 position;
	glm::vec3 velocity;

	float radius;
	int nrTriangles;

	float *points;
	float *colors;
	float *relativePoints;

	int bufferByteSize;

	//std::vector< glm::vec2 > triangles; // triangle fan structure is here

	Ball(float _radius = 10, int _nrTriangles = 20, glm::vec3 _pos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 _color = glm::vec3(1.0f, 1.0f, 1.0f));
	~Ball();

	void Update();
	void Draw();

	void SetPosition(glm::vec3 _position);
	glm::vec3 GetPosition() const;

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
	void SetPointPos(float *arr, float x, float y, float z = 0.0f, float d = 1.0f);

	void CreateVBO();
	void DestroyVBO();
};

