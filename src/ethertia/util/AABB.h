//
// Created by Dreamtowards on 2022/5/24.
//

#ifndef ETHERTIA_AABB_H
#define ETHERTIA_AABB_H

#include <glm/vec3.hpp>

class AABB
{
public:
    glm::vec3 min;
    glm::vec3 max;

    AABB(const glm::vec3 &min, const glm::vec3 &max) : min(min), max(max) {}

    AABB() : min(0), max(0) {}
};

#endif //ETHERTIA_AABB_H