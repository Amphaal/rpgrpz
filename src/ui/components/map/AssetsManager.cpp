#include "AssetsManager.h"

AssetsManager::AssetsManager(QWidget * parent) : QWidget(parent), _tree(new AssetsNavigator) {
    this->setLayout(new QVBoxLayout);
    this->layout()->setMargin(5);

    this->_addAppObjectsToTree();
    this->_refreshTree();

    this->_tree->expandAll();
    this->layout()->addWidget(this->_tree);

}

QJsonDocument AssetsManager::_getCoordinator() {
    
    //read coordinator file as JSON
    auto coordPath = QString::fromStdString(getAssetsFileCoordinatorLocation());
    QFile jsonFile(coordPath);
    
    //helper for file creation
    auto writeNewCoord = [](QFile &file) {
        file.open(QFile::WriteOnly);
        file.write("{\"version\":1,\"paths\":{},\"assets\":{}}");
        file.close();
    };

    //helper for reading
    auto readNewCoord = [](QFile &file) {
        file.open(QFile::ReadOnly);
        auto coordinator = QJsonDocument::fromJson(file.readAll());
        file.close();
        return coordinator;
    };


    //if file is empty or doesnt exist
    if(!jsonFile.size() || !jsonFile.exists()) {
        writeNewCoord(jsonFile);
    }

    //try to read the file
    auto coordinator = readNewCoord(jsonFile);

    //corrupted file, move it and create a new one
    if(coordinator.isNull()) {
        auto errorPath = coordPath + "_error";
        QDir().remove(errorPath);
        QDir().rename(coordPath, errorPath);
        writeNewCoord(jsonFile);
        return readNewCoord(jsonFile);
    }

    return coordinator;
}

void AssetsManager::_refreshTree() {

    auto coord = this->_getCoordinator();
    
    auto paths = coord["paths"].toObject().keys();

    // auto _twi = [&](const char * name) {
    //     auto twi = new QTreeWidgetItem;
    //     twi->setText(0, name);
    //     return twi;
    // };

    // this->_tree->addTopLevelItem(_twi("PJ"));
    // this->_tree->addTopLevelItem(_twi("PNJ"));
    // this->_tree->addTopLevelItem(_twi("Assets"));
}

void AssetsManager::_addAppObjectsToTree() {
    
    /* /Internal... */
    auto internal = new QTreeWidgetItem(this->_tree);
    internal->setText(0, "Interne");
    internal->setText(1, "2");
    internal->setFlags(QFlags<Qt::ItemFlag>(Qt::ItemFlag::ItemIsEnabled));
    this->_tree->addTopLevelItem(internal);

        /* /Internal/Player... */
        auto player = new QTreeWidgetItem(internal);
        player->setText(0, "Joueur");
        player->setIcon(0, QIcon(":/icons/app/manager/character.png"));
        player->setFlags(QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable));
        player->setData(0, Qt::UserRole, QString("player"));

        /* /Internal/Trigger... */
        auto trigger = new QTreeWidgetItem(internal);
        trigger->setIcon(0, QIcon(":/icons/app/manager/event.png"));
        trigger->setText(0, "Evenement");
        trigger->setFlags(QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable));
        trigger->setData(0, Qt::UserRole, QString("event"));
    
    /* /NPC... */
    auto npc = new QTreeWidgetItem(this->_tree);
    npc->setText(0, "PNJ");
    npc->setText(1, "0");
    npc->setIcon(0, QIcon(":/icons/app/manager/npc.png"));
    npc->setFlags(QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled));
    this->_tree->addTopLevelItem(npc);

    /* /MapAssets... */
    auto mapAssets = new QTreeWidgetItem(this->_tree);
    mapAssets->setIcon(0, QIcon(":/icons/app/manager/asset.png"));
    mapAssets->setText(0, "Assets de carte");
    mapAssets->setText(1, "0");
    mapAssets->setFlags(QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled));
    this->_tree->addTopLevelItem(mapAssets);

}