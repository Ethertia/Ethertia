//
// Created by Dreamtowards on 2022/4/25.
//

#ifndef ETHERTIA_TEXTUREATLAS_H
#define ETHERTIA_TEXTUREATLAS_H

#include <vector>
#include <glm/vec2.hpp>
#include <ethertia/client/Loader.h>
#include <ethertia/client/render/Texture.h>
#include <ethertia/util/BitmapImage.h>
#include <ethertia/util/Mth.h>


class TextureAtlas
{
public:
    class Region {
    public:
        BitmapImage* image;
        glm::vec2 offset;
        glm::vec2 scale;

        ~Region() {
            delete image;
        }
    };

    std::vector<Region*> atlas;
    Texture* atlasTexture;

    ~TextureAtlas() {
        for (auto* r : atlas) {
            delete r;
        }
        delete atlasTexture;
    }

public:
    Region* addAtlas(BitmapImage* image)
    {
        Region* frag = new Region();
        frag->image = image;

        atlas.push_back(frag);
        return frag;
    }

    void buildAtlas() {
        uint totalWidth = 0;
        uint maxHeight = 0;
        for (auto* frag : atlas) {
            totalWidth += frag->image->getWidth();
            maxHeight = Mth::max(maxHeight, frag->image->getHeight());
        }

        BitmapImage atlasImage(totalWidth, maxHeight);

        uint dx = 0;
        for (Region* frag : atlas) {
            BitmapImage* img = frag->image;
            float wid = img->getWidth();
            float hei = img->getHeight();
            atlasImage.setPixels(dx, maxHeight - hei, img);

            frag->offset = glm::vec2((float)dx / totalWidth, 0);
            frag->scale = glm::vec2(wid / totalWidth, hei / maxHeight);

            dx += wid;
        }

        atlasTexture = Loader::loadTexture(&atlasImage);
    }

};

#endif //ETHERTIA_TEXTUREATLAS_H
