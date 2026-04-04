#pragma once


#include "glm/glm.hpp"


namespace trace {


	struct Edge2D
	{
		glm::vec2 point0;
		glm::vec2 point1;
	};

	struct Triangle2D
	{
		glm::vec2 vertex0;
		glm::vec2 vertex1;
		glm::vec2 vertex2;
	};

	struct Circle2D
	{
		glm::vec2 position;
		float radius;
	};

	

}
