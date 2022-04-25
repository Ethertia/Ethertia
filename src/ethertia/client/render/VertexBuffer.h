//
// Created by Dreamtowards on 2022/4/25.
//

#ifndef ETHERTIA_VERTEXBUFFER_H
#define ETHERTIA_VERTEXBUFFER_H

#include <vector>

// 3d positions.
class VertexBuffer
{
public:
    std::vector<float> positions;
    std::vector<float> textureCoords;
    std::vector<float> normals;

    void addpos(float x, float y, float z) {
        positions.push_back(x);
        positions.push_back(y);
        positions.push_back(z);
    }

    void adduv(float x, float y) {
        textureCoords.push_back(x);
        textureCoords.push_back(y);
    }

    void addnorm(float x, float y, float z) {
        normals.push_back(x);
        normals.push_back(y);
        normals.push_back(z);
    }

    uint vertexCount() {
        return positions.size() / 3;
    }

};

#endif //ETHERTIA_VERTEXBUFFER_H
