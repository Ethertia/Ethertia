//
// Created by Dreamtowards on 2022/9/11.
//
// Ethernet
// Etherpedia
// Etheream   Etheream
// Etherald
// Etheria

#ifndef ETHERTIA_MATERIALTEXTURES_H
#define ETHERTIA_MATERIALTEXTURES_H

#include <map>

#include <stb/stb_image_resize.h>

#include <ethertia/render/TextureAtlas.h>
#include <ethertia/init/Materials.h>


class MaterialTextures
{
public:
    // 1024 elaborated
    // 512  pretty realistic
    // 256  pretty
    // 128  non-realistic
    inline static int TEX_RESOLUTION = 256;

    inline static Texture* ATLAS_DIFFUSE = nullptr;
    inline static Texture* ATLAS_NORM    = nullptr;
    inline static Texture* ATLAS_DRAM    = nullptr;  // DRAM: Disp, Rough, AO, Metal


    static void load()
    {
        BenchmarkTimer tm;
        Log::info("Loading {} material texture/atlases...", Material::REGISTRY.size());


        for (int i = 0; i < tex_size(); ++i) {
            Material::REGISTRY.m_Ordered[i]->m_AtlasTexIdx = i;
        }



        ATLAS_DIFFUSE = makeAtlas("diff", TEX_RESOLUTION, "./cache/atlas_diff.png");
        ATLAS_NORM    = makeAtlas("norm", TEX_RESOLUTION, "./cache/atlas_norm.png");
        ATLAS_DRAM    = makeAtlas("dram", TEX_RESOLUTION, "./cache/atlas_dram.png", true);


        Log::info("Material Texture Atlases all loaded/generated.\1");
    }

    static int tex_size() {
        return Material::REGISTRY.size();
    }
    static std::string tex_name(int i) {
        return Material::REGISTRY.m_Ordered[i]->getRegistryId();
    }

    // 我在思考 Mtl 不一定要等于/对应一个 MtlTex
    // 我们可以有很多 Mtl 的轮廓 各种装饰模型 复用MtlTex

    static bool loadMtlTex_ResizeTo(std::string_view mtl, std::string_view textype, BitmapImage& dst_resized) {
        std::string path = Loader::fileAssets(Strings::fmt("material/{}/{}.png", mtl, textype));
        if (Loader::fileExists(path)) {
            BitmapImage src = Loader::loadPNG(Loader::loadFile(path));
            BitmapImage::resizeTo(src, dst_resized);
            return true;
        } else {
            std::cerr << Strings::fmt(" missed '{}'.", path);

            //      if (textype == "diff")  resized->fillPixels(0xFFFFFFFF);  // unit 1
            // else if (textype == "disp")  resized->fillPixels(0x808080FF);  // middle height
            // else if (textype == "norm")  resized->fillPixels(0x0000FFFF);  // z=1
            // else if (textype == "rough") resized->fillPixels(0xFFFFFFFF);  // not specular
            return false;
        }
    }

    /**
     * @param textype could be: diff/disp/norm/ao/rough
     * @param px needed resolution in pixel
     */
    static Texture* makeAtlas(std::string_view textype, int px, const std::string& cache_file, bool composeDRAM = false) {
        BENCHMARK_TIMER;

        if (Loader::fileExists(cache_file))
        {
            Log::info(" *{} load cached atlas from '{}'.\1", textype, cache_file);
            return Loader::loadTexture(cache_file, false);
        }
        else
        {
            Log::info(" *{} generate new atlas to '{}'.\1", textype, cache_file);
            const int n = tex_size();
            BitmapImage atlas(px * n, px);

            if (composeDRAM)
            {
                BitmapImage resized_dram[2] = {  // 0: final resized composed DRAM
                        BitmapImage(px, px),
                        BitmapImage(px, px)
                };

                for (int i = 0; i < n; ++i)
                {
                    std::string id = tex_name(i);

                    loadMtlTex_ResizeTo(id, "disp", resized_dram[0]);
                    loadMtlTex_ResizeTo(id, "rough", resized_dram[1]);
                    // BitmapImage img_ao = loadMtlTex(texName, "ao");
                    // BitmapImage img_metal = loadMtlTex(texName, "metal");

                    BitmapImage& composed = resized_dram[0];

                    composed.setPixels(0,0,resized_dram[1], 1);

                    atlas.setPixels(i*px, 0, composed);
                }
            }
            else
            {
                BitmapImage resized(px, px);  // resized img.

                for (int i = 0; i < n; ++i)
                {

                    if (loadMtlTex_ResizeTo(tex_name(i), textype, resized))
                    {
                        atlas.setPixels(i * px, 0, resized);
                    }
                }
            }

            // save cached atlas.
            Loader::savePNG(atlas, cache_file);

            return Loader::loadTexture(atlas);
        }
    }

    static glm::vec2 uv_to_atlas_region(glm::vec2 uv, int mtlId) {
        float cap = Material::REGISTRY.size();
        return {
            (float)mtlId/cap + uv.x/cap,
            uv.y
        };
    }

};

#endif //ETHERTIA_MATERIALTEXTURES_H
