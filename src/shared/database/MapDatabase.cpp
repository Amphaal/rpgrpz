#include "MapDatabase.h"

MapDatabase::MapDatabase(const QString &filePath) : _filePath(filePath) {
    JSONDatabase::_instanciateDb();
};

void MapDatabase::saveIntoFile(RPZMap<RPZAtom> &atoms) {
    
    auto copy = this->_db.object();

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
    copy["assets_c"] = unique_assetIds.count();
    copy["atoms_c"] = db_atoms.count();
    copy["assets"] = QJsonArray::fromStringList(unique_assetIds.toList());
    copy["atoms"] = db_atoms;
    
    //saving...
    this->_updateDbFile(copy);
    qDebug() << "Map database : saving " << atoms.count() << " atoms";
};

RPZMap<RPZAtom> MapDatabase::toAtoms() {
    RPZMap<RPZAtom> out;

    auto db_atoms = this->_db["atoms"].toArray();

    for(const auto &e : db_atoms) {
        auto atom = RPZAtom(e.toObject().toVariantHash());
        out.insert(atom.id(), atom);
    }

    qDebug() << "MapDB : read" << out.count() << "atoms";

    return out;
}


const QString MapDatabase::defaultJsonDoc() {
    return "{\"version\":" + QString::number(this->apiVersion()) + ",\"atoms\":{}}";
}

const QString MapDatabase::dbPath() {
    return this->_filePath;
}

const int MapDatabase::apiVersion() {
    return 6;
}