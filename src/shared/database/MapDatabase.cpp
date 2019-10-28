#include "MapDatabase.h"

MapDatabase::MapDatabase(const QString &filePath) : JSONDatabase(QStringLiteral(u"MapDB")) {
    
    this->_initDatabaseFromJSONFile(filePath);
    
    this->log(QStringLiteral(u"read %1 atoms").arg(this->_atomsById.count()));

};
MapDatabase::MapDatabase(const QJsonObject &obj) : JSONDatabase(QStringLiteral(u"MapDB")) {
    this->_setupFromDbCopy(obj);
}
MapDatabase::MapDatabase(): JSONDatabase(QStringLiteral(u"MapDB")) {}

RPZMap<RPZAtom>& MapDatabase::atoms() {
    return this->_atomsById;
}

const RPZMap<RPZAtom>& MapDatabase::atoms() const {
    return this->_atomsById;
}

const QSet<RPZAssetHash>& MapDatabase::usedAssetHashes() const {
    return this->_assetHashes;
}

const RPZAtom MapDatabase::atom(const RPZAtomId &id) const {
    return this->_atomsById.value(id);
}

RPZAtom* MapDatabase::atomPtr(const RPZAtomId &id) {
    if(!this->_atomsById.contains(id)) return nullptr;
    return &this->_atomsById[id];
}

void MapDatabase::_setupLocalData() {

    //atoms
    for(const auto &atomAsJson : this->entityAsArray(QStringLiteral(u"atoms"))) {
        RPZAtom atom(atomAsJson.toObject().toVariantHash());
        this->_atomsById.insert(atom.id(), atom);
    }

    //hashes
    for(const auto &id : this->entityAsArray(QStringLiteral(u"assets"))) {
        auto hash = id.toString();
        this->_assetHashes += hash;
    }

}

void MapDatabase::saveIntoFile() {
    
    auto db = this->db();

    updateFrom(db, QStringLiteral(u"atoms"), this->_atomsById.toVList());
    updateFrom(db, QStringLiteral(u"assets"), this->_assetHashes);

    this->_updateDbFile(db);

};

void MapDatabase::addAtoms(const QList<RPZAtom> &toAdd) {
    for(auto const &atom : toAdd) {
        this->addAtom(atom);
    }
}

void MapDatabase::addAtom(const RPZAtom &toAdd) {
    
    this->_atomsById.insert(toAdd.id(), toAdd);

    auto hash = toAdd.assetHash();
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
    this->_assetHashes.remove(removed.assetHash());
}

void MapDatabase::clear() {
    this->_atomsById.clear();
    this->_assetHashes.clear();
}

JSONDatabase::Model MapDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"atoms"), JSONDatabase::EntityType::Array }, &this->_atomsById },
        { { QStringLiteral(u"assets"), JSONDatabase::EntityType::Array }, &this->_assetHashes }
    };
};

const JSONDatabase::Version MapDatabase::apiVersion() const {
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
                db.atoms().toVList()
            );

        }
    );

    return out;

}