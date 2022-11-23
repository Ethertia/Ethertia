//
// Created by Dreamtowards on 2022/4/22.
//

#ifndef ETHERTIA_WORLD_H
#define ETHERTIA_WORLD_H

#include <unordered_map>
#include <mutex>
#include <glm/vec3.hpp>

#include <ethertia/world/Chunk.h>
#include <ethertia/world/ChunkLoader.h>
#include <ethertia/world/gen/ChunkGenerator.h>
#include <ethertia/entity/Entity.h>
#include <ethertia/entity/EntityMesh.h>
#include <ethertia/util/Timer.h>
#include <ethertia/util/Collections.h>
#include <ethertia/util/Mth.h>
#include <ethertia/util/concurrent/Executor.h>
#include <ethertia/Ethertia.h>
#include <ethertia/render/Window.h>
#include <ethertia/world/Octree.h>

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>

class World
{
    std::unordered_map<glm::vec3, Chunk*> chunks;

    std::vector<Entity*> entities;

    ChunkGenerator chunkGenerator;

    ChunkLoader chunkLoader{"saves/dim-1/"};

public:
    btDiscreteDynamicsWorld* dynamicsWorld;

    std::mutex chunklock;

    World() {

        // init Phys
        {
            btCollisionConfiguration* collconf = new btDefaultCollisionConfiguration();
            btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collconf);

            btBroadphaseInterface* broadphase = new btDbvtBroadphase();

            btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();

            dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collconf);
        }
    }

    ~World() {

        delete dynamicsWorld->getConstraintSolver();
        delete dynamicsWorld->getDispatcher();
        delete dynamicsWorld;
        delete dynamicsWorld->getBroadphase();
    }

    MaterialStat& getBlock(glm::vec3 blockpos) {
        Chunk* chunk = getLoadedChunk(blockpos);
        if (!chunk) return Materials::STAT_EMPTY;  // shouldn't
        return chunk->getMaterial(Chunk::rpos(blockpos));
    }
    MaterialStat& getBlock(int x, int y, int z) {
        return getBlock(glm::vec3(x,y,z));
    }

    void setBlock(glm::vec3 p, MaterialStat m) {
        Chunk* chunk = getLoadedChunk(p);
        if (!chunk) return;
        glm::ivec3 bp = Chunk::rpos(p);
        chunk->setMaterial(bp, m);

    }
    void setBlock(int x, int y, int z, MaterialStat m) {
        setBlock(glm::vec3(x,y,z), m);
    }
    void requestRemodel(glm::vec3 p) {
        Chunk* chunk = getLoadedChunk(p);
        if (!chunk) return;
        glm::ivec3 bp = Chunk::rpos(p);

        chunk->requestRemodel();

        Chunk* tmp = nullptr;
        if (bp.x == 0 && (tmp=getLoadedChunk(p - glm::vec3(1, 0, 0)))) tmp->requestRemodel();
        if (bp.x == 15 && (tmp=getLoadedChunk(p + glm::vec3(1, 0, 0)))) tmp->requestRemodel();
        if (bp.y == 0 && (tmp=getLoadedChunk(p - glm::vec3(0, 1, 0)))) tmp->requestRemodel();
        if (bp.y == 15 && (tmp=getLoadedChunk(p + glm::vec3(0, 1, 0)))) tmp->requestRemodel();
        if (bp.z == 0 && (tmp=getLoadedChunk(p - glm::vec3(0, 0, 1)))) tmp->requestRemodel();
        if (bp.z == 15 && (tmp=getLoadedChunk(p + glm::vec3(0, 0, 1)))) tmp->requestRemodel();
    }










    Chunk* provideChunk(glm::vec3 p) {
        Chunk* chunk = getLoadedChunk(p);
        if (chunk) return chunk;
        glm::vec3 chunkpos = Chunk::chunkpos(p);

        chunk = chunkLoader.loadChunk(chunkpos, this);

        if (!chunk) {
            chunk = chunkGenerator.generateChunk(chunkpos, this);
        }

        assert(chunk);

        chunks[chunkpos] = chunk;

        Ethertia::getExecutor()->exec([=]() {

            addEntity(chunk->proxy);
        });

        // check populates
    tryPopulate(this, chunkpos + glm::vec3(0, 0, 0));
    tryPopulate(this, chunkpos + glm::vec3(-16, 0, 0));
    tryPopulate(this, chunkpos + glm::vec3(16, 0, 0));
    tryPopulate(this, chunkpos + glm::vec3(0, -16, 0));
    tryPopulate(this, chunkpos + glm::vec3(0, 16, 0));
    tryPopulate(this, chunkpos + glm::vec3(0, 0, -16));
    tryPopulate(this, chunkpos + glm::vec3(0, 0, 16));

        return chunk;
    }

    bool isNeighbourChunksAllLoaded(World* world, glm::vec3 chunkpos) {
        return world->getLoadedChunk(chunkpos + glm::vec3(-16, 0, 0)) &&
               world->getLoadedChunk(chunkpos + glm::vec3(16, 0, 0)) &&
               world->getLoadedChunk(chunkpos + glm::vec3(0, -16, 0)) &&
               world->getLoadedChunk(chunkpos + glm::vec3(0, 16, 0)) &&
               world->getLoadedChunk(chunkpos + glm::vec3(0, 0, -16)) &&
               world->getLoadedChunk(chunkpos + glm::vec3(0, 0, 16));
    }
    void tryPopulate(World* world, glm::vec3 chunkpos) {
        Chunk* c = world->getLoadedChunk(chunkpos);
        if (c && !c->populated && isNeighbourChunksAllLoaded(world, chunkpos)) {  // && isNeighbourAllLoaded()
            World::populate(world, chunkpos);
            c->populated = true;
        }
    }











    void unloadChunk(glm::vec3 p) {
        auto it = chunks.find(Chunk::chunkpos(p));
        if (it == chunks.end())
            throw std::logic_error("Failed unload chunk. Not exists. "+glm::to_string(p));

        Chunk* chunk = it->second;
        if (chunk) {
            removeEntity(chunk->proxy);
        }

        chunks.erase(it);
        delete chunk;
    }

    Chunk* getLoadedChunk(glm::vec3 p) {
        return chunks[Chunk::chunkpos(p)];
    }

    std::unordered_map<glm::vec3, Chunk*> getLoadedChunks() {
        return chunks;
    }

    void addEntity(Entity* e) {
        assert(Collections::find(entities, e) == -1);  // make sure the entity is not in this world.

        entities.push_back(e);
        e->onLoad(dynamicsWorld);
    }

    void removeEntity(Entity* e) {
        Collections::erase(entities, e);
        e->onUnload(dynamicsWorld);
    }

    std::vector<Entity*>& getEntities() {
        return entities;
    }














    bool raycast(glm::vec3 begin, glm::vec3 end, glm::vec3& p, glm::vec3& n) {
        btVector3 _beg(begin.x, begin.y, begin.z);
        btVector3 _end(end.x, end.y, end.z);
        btCollisionWorld::ClosestRayResultCallback rayCallback(_beg, _end);

        dynamicsWorld->rayTest(_beg, _end, rayCallback);
        if (rayCallback.hasHit()) {
            p = Mth::vec3(rayCallback.m_hitPointWorld);
            n = Mth::vec3(rayCallback.m_hitNormalWorld);
            return true;
        }
        return false;
    }



    // http://www.cse.yorku.ca/~amana/research/grid.pdf
    // Impl of Grid Voxel Raycast.
    bool raycast(glm::vec3 rpos, glm::vec3 rdir, glm::vec3& _pos, u8& _face) {
        using glm::vec3;

        vec3 step = glm::sign(rdir);

        vec3 tMax = glm::abs( (glm::fract(rpos)) - glm::max(step, 0.0f)) / glm::abs(rdir);

        vec3 tDelta = 1.0f / glm::abs(rdir);

        glm::vec3 p = glm::floor(rpos);

        int itr = 0;
        while (++itr < 100) {
            int face;
            if (tMax.x < tMax.y && tMax.x < tMax.z) {
                p.x += step.x;
                tMax.x += tDelta.x;
                face = step.x > 0 ? 0 : 1;
            } else if (tMax.y < tMax.z) {
                p.y += step.y;
                tMax.y += tDelta.y;
                face = step.y > 0 ? 2 : 3;
            } else {
                p.z += step.z;
                tMax.z += tDelta.z;
                face = step.z > 0 ? 4 : 5;
            }

            MaterialStat& b = getBlock(p);
            if (b.id) {
                _pos = p;
                _face = face;
                return true;
            }
        }
        return false;
    }

    static void collideAABB(const AABB& self, glm::vec3& d, const AABB& coll) {


        if (d.y != 0 && AABB::intersectsAxis(self, coll, 0) && AABB::intersectsAxis(self, coll, 2)) {
            if (d.y < 0 && self.min.y >= coll.max.y) {
                d.y = absmin(d.y, coll.max.y - self.min.y);
            } else if (d.y > 0 && self.max.y <= coll.min.y) {
                d.y = absmin(d.y, coll.min.y - self.max.y);
            }
        }
        if (d.x != 0 && AABB::intersectsAxis(self, coll, 1) && AABB::intersectsAxis(self, coll, 2)) {
            if (d.x < 0 && self.min.x >= coll.max.x) {
                d.x = absmin(d.x, coll.max.x - self.min.x);
            } else if (d.x > 0 && self.max.x <= coll.min.x) {
                d.x = absmin(d.x, coll.min.x - self.max.x);
            }
        }
        if (d.z != 0 && AABB::intersectsAxis(self, coll, 0) && AABB::intersectsAxis(self, coll, 1)) {
            if (d.z < 0 && self.min.z >= coll.max.z) {
                d.z = absmin(d.z, coll.max.z - self.min.z);
            } else if (d.z > 0 && self.max.z <= coll.min.z) {
                d.z = absmin(d.z, coll.min.z - self.max.z);
            }
        }

    }

    static float absmin(float a, float b) {
        return Mth::abs(a) < Mth::abs(b) ? a : b;
    }

    void tick() {
        float dt = Timer::T_DELTA;

//        dynamicsWorld->stepSimulation(dt);

//        return;
//        for (Entity* e : entities) {
//            // motion
//            e->prevposition = e->position;
//
//            e->position += e->velocity * dt;
//
//            float vels = glm::length(e->velocity);
//            float linearDamping = 0.001f;
//            e->velocity *= vels < 0.01 ? 0 : Mth::pow(linearDamping, dt);
//
//            // collide
//            // get aabb of blocks that inside (prevpos, currpos)
//            // for each axis xyz, test nearest plane, and clip position & velocity
//
////            for (int dx = Mth::floor(e->position.x); dx < Mth::ceil(e->prevposition.x); ++dx) {
////                for (int dy = Mth::floor(e->position.y); dy < Mth::ceil(e->prevposition.y); ++dy) {
////                    for (int dz = Mth::floor(e->position.z); dz < Mth::ceil(e->prevposition.z); ++dz) {
////
////                    }
////                }
////            }
//
//            {
//                if (false ) {  // || !Ethertia::getWindow()->isAltKeyDown()
//                    glm::vec3& v = e->velocity;
//
//                    glm::vec3 pp = e->prevposition;
//                    AABB self = AABB({pp - glm::vec3(0.5f)},
//                                     {pp + glm::vec3(0.5f)});
//
//                    glm::vec3 d = e->position - e->prevposition;
//                    const glm::vec3 od = d;
//
//                    glm::vec3 bmin = glm::floor(glm::min(e->position, e->prevposition) - glm::vec3(0.5f));
//                    glm::vec3 bmax = glm::ceil(glm::max(e->position, e->prevposition) + glm::vec3(0.5f));
//
//                    for (int dx = bmin.x; dx < bmax.x; ++dx) {
//                        for (int dy = bmin.y; dy < bmax.y; ++dy) {
//                            for (int dz = bmin.z; dz < bmax.z; ++dz) {
//
//                                glm::vec3 min(dx, dy, dz);
//                                glm::vec3 max = min + glm::vec3(1);
//
//                                BlockState b = getBlock(min);
//                                if (!b.id) continue;
//
//                                collideAABB(self, d, AABB(min, max));
//                            }
//                        }
//                    }
//
//
//                    if (d.x != od.x) v.x = 0;
//                    if (d.y != od.y) v.y = 0;
//                    if (d.z != od.z) v.z = 0;
//
//                    e->position = e->prevposition + d;
//
//                }
//            }
//
//
//
//        }

    }


    static void populate(World* world, glm::vec3 chunkpos)
    {
        for (int dx = 0; dx < 16; ++dx) {
            for (int dz = 0; dz < 16; ++dz) {
                int x = chunkpos.x + dx;
                int z = chunkpos.z + dz;
                int nextAir = -1;

                for (int dy = 0; dy < 16; ++dy) {
                    int y = chunkpos.y + dy;
                    MaterialStat tmpbl = world->getBlock(x, y, z);
                    if (tmpbl.id == 0)// || tmpbl.id == Blocks::WATER)
                        continue;
                    if (tmpbl.id != Materials::STONE)
                        continue;

                    if (nextAir < dy) {
                        for (int i = 0;; ++i) {
                            if (world->getBlock(x, y+i, z).id == 0) {
                                nextAir = dy+i;
                                break;
                            }
                        }
                    }

                    int nextToAir = nextAir - dy;

                    u8 replace = Materials::STONE;
                    if (y < 3 && nextToAir < 3 && world->chunkGenerator.noise.noise(x/60.0, y/60.0, z/60.0) > 0.1) {
                        replace = Materials::SAND;
                    } else if (nextToAir == 1) {
                        replace = Materials::GRASS;
                    } else if (nextToAir < 4) {
                        replace = Materials::DIRT;
                    }
                    world->getBlock(x, y, z).id = replace;
                }
            }

        }

        // Grass
//        for (int dx = 0; dx < 16; ++dx) {
//            for (int dz = 0; dz < 16; ++dz) {
//                int x = chunkpos.x + dx;
//                int z = chunkpos.z + dz;
//
//                float f = Mth::hash(x * z * 100);
//                if (f < (60 / 256.0f)) {
//
//                    for (int dy = 0; dy < 16; ++dy) {
//                        int y = chunkpos.y + dy;
//                        if (world->getBlock(x, y, z).id == Materials::GRASS) {
//                            u8 b = Blocks::TALL_GRASS;
//                            if (f < (4/256.0f)) b = Blocks::RED_TULIP;
//                            else if (f < (30/256.0f)) b = Blocks::SHRUB;
//                            else if (f < (40/256.0f)) b = Blocks::FERN;
//
//                            world->setBlock(x, y+1, z, BlockState(b));
//                        }
//                    }
//                }
//            }
//        }

//        // Vines
//        for (int dx = 0; dx < 16; ++dx) {
//            for (int dz = 0; dz < 16; ++dz) {
//                int x = chunkpos.x + dx;
//                int z = chunkpos.z + dz;
//
//                if (Mth::hash(x * z * 2349242) < (6.0f / 256.0f)) {
//                    for (int dy = 0; dy < 16; ++dy) {
//                        int y = chunkpos.y + dy;
//                        if (world->getBlock(x, y, z).id == Blocks::STONE &&
//                            world->getBlock(x, y-1, z).id == 0) {
//
//
//                            for (int i = 0; i < 32 * Mth::hash(x*y*47923); ++i) {
//
//                                world->setBlock(x, y-1-i, z, BlockState(Blocks::VINE));
//                            }
//
//                            break;
//                        }
//                    }
//                }
//            }
//        }

//        // Trees
//        for (int dx = 0; dx < 16; ++dx) {
//            for (int dz = 0; dz < 16; ++dz) {
//                int x = chunkpos.x + dx;
//                int z = chunkpos.z + dz;
//
//                if (Mth::hash(x*z) < (2.5/256.0f)) {
//                    for (int dy = 0; dy < 16; ++dy) {
//                        int y = chunkpos.y + dy;
//                        if (world->getBlock(x, y, z).id == Blocks::GRASS) {
//
//                            float f = Mth::hash(x*z*y);
//                            int h = 2+f*8;
//                            int r = 3;
//
//                            u8 _leaf = Blocks::LEAVES;
//                            if (f > 0.8f) {
//                                h *= 1.6f;
//                                _leaf = Blocks::LEAVES_JUNGLE;
//                            } else if (f < 0.2f) _leaf = Blocks::LEAVES_APPLE;
//
//
//                            for (int lx = -r; lx <= r; ++lx) {
//                                for (int lz = -r; lz <= r; ++lz) {
//                                    for (int ly = -r; ly <= r+f*8; ++ly) {
//                                        if (Mth::sq(Mth::abs(lx)) + Mth::sq(Mth::abs(lz)) + Mth::sq(Mth::abs(ly)) > r*r)
//                                            continue;
//                                        if (_leaf == Blocks::LEAVES && Mth::hash(x*y*z) < 0.2f)
//                                            _leaf = Blocks::LEAVES_APPLE;
//                                        world->setBlock(x+lx, y+ly+h+Mth::hash(y)*4, z+lz, BlockState(_leaf));
//                                    }
//                                }
//                            }
//                            for (int i = 0; i < h; ++i) {
//
//                                world->setBlock(x, y+i+1, z, BlockState(Blocks::LOG));
//                            }
//                            break;
//                        }
//                    }
//                }
//            }
//        }

    }



    static MaterialStat& _GetBlock(Chunk* chunk, glm::vec3 rp) {
        return Chunk::outbound(rp) ? chunk->world->getBlock(chunk->position + rp) : chunk->getMaterial(rp);
    }
};

#endif //ETHERTIA_WORLD_H
