//
// Created by Dreamtowards on 2023/1/17.
//

#ifndef ETHERTIA_ITEMS_H
#define ETHERTIA_ITEMS_H


class ItemComponentMaterial : public Item::Component {
public:

    Material* m_Material;

    ItemComponentMaterial(Material* mtl) : m_Material(mtl) {}

    void onUse() override;

};

class ItemComponentGrapple : public Item::ComponentTool {
public:

    void onUse() override;

};

class Items
{
public:



#define REGISTER_ITEM(VAR, id, comps) inline static const Item* VAR = new Item(id, comps);

    REGISTER_ITEM(APPLE, "apple", {new Item::ComponentFood(1.5)});
    REGISTER_ITEM(LIME, "lime", {new Item::ComponentFood(0.5)});

    REGISTER_ITEM(PICKAXE, "pickaxe", {new Item::ComponentTool()});
    REGISTER_ITEM(GRAPPLE, "grapple", {new ItemComponentGrapple()});

//    inline static const Item* MELON = new Item("melon", {new Item::ComponentFood(1.0)});

};



//enum Id_ {
//    // Tools
//    PICKAXE,
//    SWORD,
//    AXE,
//    HOE,
//    SHEAR,
//    BACKPACK,
//
//    // Food, Vegetable, Meat
//    MELON,
//    PUMPKIN,
//    CARROT,
//    POTATO,
//    POTATO_BAKED,
//    APPLE,
//    BREAD,
//    PORKCHOP,
//    PORKCHOP_COOKED,
//    BEEF,
//    BEEF_COOCKED,
//    VINES,
//    LILY_PAD,
//    // Mushroom, Flower
//
//    COAL,
//    IRON_INGOT,
//    GOLD_INGOT,
//
//    BRICKS,
//    FLOWER_POT,
//    PAINTING,
//    SIGN,
//    RAIL,
//    TORCH,
//    LADDER,
//    MINECART,
//    SPAWN_EGG,
//};

#endif //ETHERTIA_ITEMS_H