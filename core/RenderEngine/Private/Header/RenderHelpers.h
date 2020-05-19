#pragma once

#define GLM_SWIZZLE_XYZW
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

inline float sphereIntersect(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &p, const float rad)
{
		glm::vec3 op = p - pos; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t;
		double eps = 1e-4;
		double b = glm::dot(op, rot);
		double det = b * b - glm::dot(op, op) + rad * rad;
        if (det < 0)
            return 0;
        else
            det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
}