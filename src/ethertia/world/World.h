//
// Created by Dreamtowards on 2022/4/22.
//

#ifndef ETHERTIA_WORLD_H
#define ETHERTIA_WORLD_H

#include <unordered_map>
#include <mutex>
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <entt/entt.hpp>

#include <ethertia/world/Cell.h>

class Entity;
class Chunk;
class ChunkGenerator;
class ChunkLoader;

#define LOCK_GUARD(x) std::lock_guard<std::mutex> _guard(x);

class World
{
    std::unordered_map<glm::vec3, Chunk*> m_Chunks;

    std::vector<Entity*> m_Entities;

    ChunkGenerator* m_ChunkGenerator = nullptr;

public:
    ChunkLoader* m_ChunkLoader = nullptr;

    btDynamicsWorld* m_DynamicsWorld = nullptr;

    entt::registry m_EnttRegistry;

    std::mutex lock_ChunkList;
    std::mutex m_LockDynamicsWorld;
    std::mutex m_LockEntities;

    uint32_t m_Seed = 0;


    World(const std::string& savedir, uint32_t seed);
    ~World();

    Cell& getCell(glm::vec3 p);
    Cell& getCell(int x, int y, int z) { return getCell({x,y,z}); }

    void setCell(glm::vec3 p, const Cell& c);
    void setCell(int x, int y, int z, const Cell& c) { setCell({x,y,z}, c); }

    void requestRemodel(glm::vec3 p, bool detectNeighbour = true);

    void processEntityCollision();




    Chunk* provideChunk(glm::vec3 p);

    void unloadChunk(glm::vec3 p);

    Chunk* getLoadedChunk(glm::vec3 p);

    const std::unordered_map<glm::vec3, Chunk*>& getLoadedChunks() {
        return m_Chunks;
    }

    void addEntity(Entity* e);

    void removeEntity(Entity* e);

    std::vector<Entity*>& getEntities() {
        return m_Entities;
    }














    bool raycast(glm::vec3 begin, glm::vec3 end, glm::vec3& p, glm::vec3& n, btCollisionObject** obj) const;


    void tick() {
//        float dt = Timer::T_DELTA;
//        dynamicsWorld->stepSimulation(dt);
    }

    static void populate(World* world, glm::vec3 chunkpos);


    static Cell& _GetCell(Chunk* chunk, glm::vec3 rp);
};

#endif //ETHERTIA_WORLD_H
