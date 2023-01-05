//
// Created by Dreamtowards on 2022/8/26.
//

#ifndef ETHERTIA_CHUNKLOADER_H
#define ETHERTIA_CHUNKLOADER_H

#include <iostream>
#include <fstream>
#include <utility>
#include <string>

#include <ethertia/util/Strings.h>

class ChunkLoader
{
public:

    std::string m_ChunkDir;

    ChunkLoader(std::string savedir) : m_ChunkDir(std::move(savedir)) {
        assert(!Loader::fileExists(savedir));
    }

    Chunk* loadChunk(glm::vec3 chunkpos, World* world) {
        std::fstream file = openfile(chunkpos, std::ios::in);
        if (!file)
            return nullptr;

        Chunk* chunk = new Chunk(chunkpos, world);

        // todo: octrees
        int chunkBlocksSize = sizeof(Cell) * 4096;
        file.read((char*)&chunk->blocks, chunkBlocksSize);




        file.close();
        return chunk;
    }

    void saveChunk(Chunk* chunk) {
        std::fstream file = openfile(chunk->position, std::ios::out);



    }

    // .vst Volume Store
    [[nodiscard]] std::fstream openfile(glm::vec3 chunkpos, std::ios::openmode m) const {
        assert(Chunk::validchunkpos(chunkpos));
        std::fstream file(Strings::fmt("{}/{}.{}.{}.vst", m_ChunkDir, chunkpos.x, chunkpos.y, chunkpos.z), m);
        return file;
    }

};

#endif //ETHERTIA_CHUNKLOADER_H
