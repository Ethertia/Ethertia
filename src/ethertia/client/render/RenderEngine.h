//
// Created by Dreamtowards on 2022/4/22.
//

#ifndef ETHERTIA_RENDERENGINE_H
#define ETHERTIA_RENDERENGINE_H

#include <ethertia/world/World.h>
#include <ethertia/client/render/renderer/ChunkRenderer.h>

class RenderEngine {

public:
    ChunkRenderer chunkRenderer;


    void renderWorld(World* world)
    {

        for (auto it : *world->getLoadedChunks())
        {
            chunkRenderer.render(it.second);
        }
    }

};

#endif //ETHERTIA_RENDERENGINE_H
