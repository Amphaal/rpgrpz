#include "AssetsDatabase.h"

AssetsDatabase::AssetsDatabase() { 
    this->_instanciateDb();

    /* /Internal... */
    auto interne = new AssetsDatabaseElement("Interne", AssetsDatabaseElement::Type::InternalContainer);
            
        /* /Internal/Player... */
        auto player = new AssetsDatabaseElement("Joueur", AssetsDatabaseElement::Type::Player);
        interne->appendChild(player);

        /* /Internal/Trigger... */
        auto trigger = new AssetsDatabaseElement("Evenement", AssetsDatabaseElement::Type::Event);
        interne->appendChild(trigger);

    /* /NPC... */
    auto npc = new AssetsDatabaseElement("PNJ", AssetsDatabaseElement::Type::NPC_Container);

    /* /MapAssets... */
    auto mapAssets = new AssetsDatabaseElement("Assets de carte", AssetsDatabaseElement::Type::ObjectContainer);

    /* /Brushes... */
    auto brushes = new AssetsDatabaseElement("Terrains", AssetsDatabaseElement::Type::FloorBrushContainer);

    this->appendChild(interne);
    this->appendChild(npc);
    this->appendChild(mapAssets);
    this->appendChild(brushes);
};

const QString AssetsDatabase::defaultJsonDoc() {
    return "{\"version\":1,\"paths\":{},\"assets\":{}}";
};
const QString AssetsDatabase::dbPath() {
    return QString::fromStdString(getAssetsFileCoordinatorLocation());
};