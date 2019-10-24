#include "MapDatabase.h"

MapDatabase::MapDatabase(const QString &filePath) : JSONDatabase(filePath) {};
MapDatabase::MapDatabase(const QJsonObject &obj) : JSONDatabase(obj) {}
MapDatabase::MapDatabase() {}

const RPZMap<RPZAtom> MapDatabase::safe_atoms() const {
    return this->_atomsById;
}

const QSet<RPZAssetHash> MapDatabase::safe_usedAssetsIds() const {
    return this->_assetHashes;
}

void MapDatabase::_setupLocalData() {

    //atoms
    for(const auto &atomAsJson : this->entityAsObject(QStringLiteral(u"atoms"))) {
        RPZAtom atom(atomAsJson.toObject().toVariantHash());
        this->_atomsById.insert(atom.id(), atom);
    }

    //hashes
    for(const auto &id : this->entityAsArray(QStringLiteral(u"assets"))) {
        auto hash = id.toString();
        this->_assetHashes += hash;
    }

    qDebug() << "MapDB : read" << this->_atomsById.count() << "atoms";
}

void MapDatabase::saveIntoFile() {
    
    auto db = this->db();

    updateFrom(db, QStringLiteral(u"atoms"), this->_atomsById.toVMap());
    updateFrom(db, QStringLiteral(u"assets"), this->_assetHashes);

    this->_updateDbFile(db);

    qDebug() << "Map database : saving " << this->_atomsById.count() << " atoms";

};

void MapDatabase::addAtoms(const QList<RPZAtom> &toAdd) {
    for(auto const &atom : toAdd) {
        this->addAtom(atom);
    }
}

void MapDatabase::addAtom(const RPZAtom &toAdd) {
    
    this->_atomsById.insert(toAdd.id(), toAdd);

    auto hash = toAdd.assetId();
    if(!hash.isEmpty()) {
        this->_assetHashes += hash;
    }

}

void MapDatabase::updateAtom(const RPZAtom &updated) {
    this->_atomsById.insert(updated.id(), updated);
}

void MapDatabase::updateAtom(const RPZAtomId &toUpdate, const AtomUpdates &updates) {
    if(!this->_atomsById.contains(toUpdate)) return;
    this->_atomsById[toUpdate].setMetadata(updates);
}

void MapDatabase::removeAtom(const RPZAtomId &toRemove) {
    auto removed = this->_atomsById.take(toRemove);
    this->_assetHashes.remove(removed.assetId());
}

void MapDatabase::clear() {
    this->_atomsById.clear();
    this->_assetHashes.clear();
}

JSONDatabase::Model MapDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"atoms"), JSONDatabase::EntityType::Object }, &this->_atomsById },
        { { QStringLiteral(u"assets"), JSONDatabase::EntityType::Array }, &this->_assetHashes }
    };
};

const JSONDatabase::Version MapDatabase::apiVersion() {
    return 7;
}

QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler> MapDatabase::_getUpdateHandlers() {
    
    auto out = QHash<JSONDatabase::Version, JSONDatabase::UpdateHandler>();

    //to v7
    out.insert(
        7,
        [&](QJsonObject &doc) {
            
            MapDatabase db(doc);

            //iterate atoms
            for(auto atom : db.safe_atoms()) {
                
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
                db.safe_atoms().toVMap()
            );

        }
    );

    return out;

}