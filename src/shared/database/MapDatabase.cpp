#include "MapDatabase.h"

MapDatabase::MapDatabase(const QString &filePath) : _filePath(filePath) {
    JSONDatabase::_instanciateDb();
};

QJsonObject MapDatabase::toObject(const RPZMap<RPZAtom> &atoms, const QJsonDocument &doc) {

    //reseting "atoms" object
    QJsonArray db_atoms;
    QSet<RPZAssetHash> unique_assetIds;
    for(auto &atom : atoms) {
        
        //list asset id
        auto assetId = atom.assetId();
        if(!assetId.isEmpty()) unique_assetIds.insert(assetId);

        //fill db
        auto casted = QJsonObject::fromVariantHash(atom);
        db_atoms.append(casted);

    }

    //fill copy
    QJsonObject copy;
    copy["assets_c"] = unique_assetIds.count();
    copy["atoms_c"] = db_atoms.count();
    copy["assets"] = QJsonArray::fromStringList(unique_assetIds.toList());
    copy["atoms"] = db_atoms;
    copy["version"] = doc["version"];
    
    return copy;

}

void MapDatabase::saveIntoFile(const RPZMap<RPZAtom> &atoms) {
    
    auto copy = MapDatabase::toObject(atoms, this->_db);

    //saving...
    this->_updateDbFile(copy);

    qDebug() << "Map database : saving " << atoms.count() << " atoms";

};

RPZMap<RPZAtom> MapDatabase::toAtoms() {
    RPZMap<RPZAtom> out = MapDatabase::toAtoms(this->_db);
    qDebug() << "MapDB : read" << out.count() << "atoms";
    return out;
}

RPZMap<RPZAtom> MapDatabase::toAtoms(const QJsonDocument &doc) {
    RPZMap<RPZAtom> out;

    auto db_atoms = doc["atoms"].toArray();

    for(const auto &e : db_atoms) {
        auto atom = RPZAtom(e.toObject().toVariantHash());
        out.insert(atom.id(), atom);
    }

    return out;
}

const QString MapDatabase::defaultJsonDoc() {
    return "{\"version\":" + QString::number(this->apiVersion()) + ",\"atoms\":{}}";
}

const QString MapDatabase::dbPath() {
    return this->_filePath;
}

const int MapDatabase::apiVersion() {
    return 7;
}

QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> MapDatabase::_getUpdateHandlers() {
    
    auto out = QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler>();

    //to v7
    out.insert(
        7,
        [&](QJsonDocument &doc) {
            
            //iterate atoms
            auto atoms = MapDatabase::toAtoms(doc);
            for(auto &atom : atoms) {
                
                auto shape = atom.shape();

                //add center
                atom.setMetadata(
                    AtomParameter::ShapeCenter, 
                    shape.boundingRect().center()
                );

            }

            //update doc
            auto duplicated = MapDatabase::toObject(atoms, doc);
            doc.setObject(duplicated);

        }
    );

    return out;

}