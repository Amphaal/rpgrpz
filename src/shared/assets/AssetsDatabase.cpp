#include "AssetsDatabase.h"

AssetsDatabase::AssetsDatabase() { 
    this->_instanciateDb();
    this->_includeStaticComponents();
    this->_includeDbComponents();
};

const QString AssetsDatabase::defaultJsonDoc() {
    return "{\"version\":1,\"paths\":{},\"hashes\":[]}";
};
const QString AssetsDatabase::dbPath() {
    return QString::fromStdString(getAssetsFileCoordinatorLocation());
};

///
///
///

void AssetsDatabase::_includeDbComponents() {

    auto db_paths = this->_db["paths"].toObject();

    auto paths = db_paths.keys();
    paths.sort();

    for(auto &paths : paths) {
        
    }
}

void AssetsDatabase::_includeStaticComponents() {

    auto staticModelInsert = [&](QString name, AssetsDatabaseElement::Type type) {
        auto i = new AssetsDatabaseElement(name, type);
        this->_staticElements.insert(type, i);
        return i;
    };

    /* /Internal... */
    auto interne = staticModelInsert("Interne", AssetsDatabaseElement::Type::InternalContainer);
            
        /* /Internal/Player... */
        auto player = new AssetsDatabaseElement("Joueur", AssetsDatabaseElement::Type::Player);
        interne->appendChild(player);

        /* /Internal/Trigger... */
        auto trigger = new AssetsDatabaseElement("Evenement", AssetsDatabaseElement::Type::Event);
        interne->appendChild(trigger);

    /* /NPC... */
    auto npc = staticModelInsert("PNJ", AssetsDatabaseElement::Type::NPC_Container);

    /* /MapAssets... */
    auto mapAssets = staticModelInsert("Assets de carte", AssetsDatabaseElement::Type::ObjectContainer);

    /* /Brushes... */
    auto brushes = staticModelInsert("Terrains", AssetsDatabaseElement::Type::FloorBrushContainer);

    this->appendChild(interne);
    this->appendChild(npc);
    this->appendChild(mapAssets);
    this->appendChild(brushes);
}

bool AssetsDatabase::insertAsset(QUrl &url, AssetsDatabaseElement* parent) {
    
    //check if db file exists, recreate it if necessary
    this->_checkFileExistance();

    //check scheme
    if(url.scheme() != "file") {
        qDebug() << "Assets : cannot insert, uri is not a file !";
        return false;
    }

    //prepare
    auto source = url.toString(QUrl::UrlFormattingOption::RemoveScheme).mid(3);
    auto sourceFile = QFile(source);
    if(!sourceFile.exists()) {
        qDebug() << "Assets : cannot insert, uri file does not exist !";
        return false;
    }

    //read hash..
    sourceFile.open(QFile::ReadOnly);
    auto hash = QString::fromUtf8(
        QCryptographicHash::hash(sourceFile.readAll(), QCryptographicHash::Keccak_224)
    );
    sourceFile.close();

        // comparaison from db
        auto db_hashes = this->_db["hashes"].toArray();
        if(db_hashes.contains(hash)) {
            qDebug() << "Assets : will not insert, file hash has been found in db !";
            return false;
        }

    //checksum and uuid
    auto quuid = QUuid::createUuid().toString();
    quuid = quuid.mid(1, quuid.length() - 2);

    //dest file
    auto destFolder = QString::fromStdString(getAssetsFolderLocation());
    auto fInfo = QFileInfo(url.fileName());
    auto destFileExt = fInfo.suffix();
    auto dest = destFolder + QDir::separator() + quuid + "." + destFileExt;

    //copy
    auto copyResult = QFile::copy(source, dest);
    if(!copyResult) {
        qDebug() << "Assets : cannot insert, issue while copying source file to dest !";
        return false;
    }

    //update db
    auto obj = this->_db.object();
        
        //update hashes list
        db_hashes.append(QJsonValue(hash));
        obj["hashes"] = db_hashes;

        //define new asset item
        auto newAsset = QJsonObject();
        newAsset["hash"] = hash;
        newAsset["ext"] = destFileExt;

            //save new asset
            auto assets = obj["assets"].toObject();
            assets.insert(quuid, newAsset);
            obj["assets"] = assets;
        
        //update path status
        auto folderParentPath = parent->path();
        auto db_paths = obj["paths"].toObject();

            //check path existance, create it if non existant
            QJsonArray objsInPath = db_paths.keys().contains(folderParentPath) ? db_paths[folderParentPath].toArray() : QJsonArray();

            //append id to list
            if(!objsInPath.contains(quuid)) {
                objsInPath.append(quuid);
                db_paths[folderParentPath] = objsInPath;
                obj["paths"] = db_paths;
            }


    //update db file
    this->_updateDbFile(obj);
    qDebug() << "Assets : " << fInfo.filePath() << " inserted !";

    //add element
    auto element = new AssetsDatabaseElement(fInfo.fileName(), parent->defaultTypeOnContainerForInsert());
    parent->appendChild(element);
    return true;
}