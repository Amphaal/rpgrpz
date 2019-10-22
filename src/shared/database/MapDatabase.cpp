#include "MapDatabase.h"

MapDatabase::MapDatabase(const QString &filePath) : JSONDatabase(filePath) {};
MapDatabase::MapDatabase(const QJsonObject &obj) : JSONDatabase(obj) {}

const QSet<RPZAssetHash>& MapDatabase::getUsedAssetsIds() const {
    return this->_assetHashes;
}

void MapDatabase::_setupLocalData() {

    //atoms
    for(const auto &atom : this->entityAsObject(QStringLiteral(u"atoms"))) {
        RPZAtom atom(atom.toObject().toVariantHash());
        this->_atoms.insert(atom.id(), atom);
    }

    //hashes
    for(const auto &id : this->entityAsArray(QStringLiteral(u"assets"))) {
        auto hash = id.toString();
        this->_assetHashes += hash;
    }

    qDebug() << "MapDB : read" << this->_atoms.count() << "atoms";
}

void MapDatabase::saveIntoFile() {
    
    auto db = this->db();

    updateFrom(db, QStringLiteral(u"atoms"), this->_atoms.toVMap());
    updateFrom(db, QStringLiteral(u"assets"), this->_assetHashes);

    this->_updateDbFile(db);

    qDebug() << "Map database : saving " << this->_atoms.count() << " atoms";

};

void MapDatabase::addAtom(const RPZAtom &toAdd) {
    
    this->_atoms.insert(toAdd.id(), toAdd);

    auto hash = toAdd.assetId();
    if(!hash.isEmpty()) {
        this->_assetHashes += hash;
    }

}

void MapDatabase::updateAtom(const RPZAtom &updated) {
    this->_atoms.insert(updated.id(), updated);
}

void MapDatabase::removeAtom(const RPZAtomId &idToRemove) {
    auto removed = this->_atoms.take(idToRemove);
    this->_assetHashes.remove(removed.assetId());
}


JSONDatabaseModel MapDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"atoms"), ET_Object }, &this->_atoms },
        { { QStringLiteral(u"assets"), ET_Array }, &this->_assetHashes }
    };
};

const int MapDatabase::apiVersion() {
    return 7;
}

QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler> MapDatabase::_getUpdateHandlers() {
    
    auto out = QHash<JSONDatabaseVersion, JSONDatabaseUpdateHandler>();

    //to v7
    out.insert(
        7,
        [&](QJsonObject &doc) {
            
            MapDatabase db(doc);

            //iterate atoms
            for(auto atom : db.atoms()) {
                
                auto shape = atom.shape();

                //add center
                atom.setMetadata(
                    AtomParameter::ShapeCenter, 
                    shape.boundingRect().center()
                );

                db.updateAtom(atom);

            }

            //update json obj
            updateFrom(
                doc,
                QStringLiteral(u"atoms"),
                db.atoms().toVMap()
            );

        }
    );

    return out;

}