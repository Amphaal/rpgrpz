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

const RPZFogParams MapDatabase::fogParams() const {
    return this->_fogParams;
}

const QSet<RPZAsset::Hash>& MapDatabase::usedAssetHashes() const {
    return this->_assetHashes;
}

const RPZAtom MapDatabase::atom(const RPZAtom::Id &id) const {
    return this->_atomsById.value(id);
}

RPZAtom* MapDatabase::atomPtr(const RPZAtom::Id &id) {
    if(!this->_atomsById.contains(id)) return nullptr;
    return &this->_atomsById[id];
}

void MapDatabase::_setupLocalData() {

    //map params
    this->_mapParams = RPZMapParameters(this->entityAsObject(QStringLiteral(u"params")).toVariantHash());

    //fog
    this->_fogParams = RPZFogParams(this->entityAsObject(QStringLiteral(u"fog")).toVariantHash());

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

const QJsonObject MapDatabase::_updatedInnerDb() {
    
    auto db = this->db();

    updateFrom(db, QStringLiteral(u"fog"), this->_fogParams);
    updateFrom(db, QStringLiteral(u"atoms"), this->_atomsById.toVList());
    updateFrom(db, QStringLiteral(u"assets"), this->_assetHashes);
    updateFrom(db, QStringLiteral(u"params"), this->_mapParams);

    return db;

}

const QString MapDatabase::snapshotSave(const QString &folderToSaveTo) {
    
    auto filename = QDateTime::currentDateTime().toString(QStringLiteral(u"dd_MM_yyyy_hh_mm_ss_zzz"));
    
    auto fullPath = QStringLiteral(u"%1/%2%3")
                    .arg(folderToSaveTo)
                    .arg(filename)
                    .arg(AppContext::RPZ_MAP_FILE_EXT);
    
    JSONDatabase::saveAsFile(this->_updatedInnerDb(), fullPath);

    return fullPath;

}

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

void MapDatabase::setMapParams(const RPZMapParameters &newParams) {
    this->_mapParams = newParams;
}

void MapDatabase::setFogParams(const RPZFogParams &fogParams) {
    this->_fogParams = fogParams;
}

const RPZMapParameters MapDatabase::mapParams() const {
    return this->_mapParams;
}

void MapDatabase::updateAtom(const RPZAtom &updated) {
    this->_atomsById.insert(updated.id(), updated);
}

void MapDatabase::updateAtom(const RPZAtom::Id &toUpdate, const RPZAtom::Updates &updates) {
    if(!this->_atomsById.contains(toUpdate)) return;
    this->_atomsById[toUpdate].setMetadata(updates);
}

void MapDatabase::removeAtom(const RPZAtom::Id &toRemove) {
    auto removed = this->_atomsById.take(toRemove);
    this->_assetHashes.remove(removed.assetHash());
}

void MapDatabase::clear() {
    this->_atomsById.clear();
    this->_assetHashes.clear();
    this->_mapParams.clear();
}

JSONDatabase::Model MapDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"fog"), JSONDatabase::EntityType::Object }, &this->_fogParams },
        { { QStringLiteral(u"atoms"), JSONDatabase::EntityType::Array }, &this->_atomsById },
        { { QStringLiteral(u"assets"), JSONDatabase::EntityType::Array }, &this->_assetHashes },
        { { QStringLiteral(u"params"), JSONDatabase::EntityType::Object }, &this->_mapParams },
    };
};

const JSONDatabase::Version MapDatabase::apiVersion() const {
    return 9;
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
                    RPZAtom::Parameter::ShapeCenter, 
                    shape.boundingRect().center()
                );

                //increment brushType
                if(atom.type() == RPZAtom::Type::Brush) {
                    atom.setMetadata(
                        RPZAtom::Parameter::BrushStyle, 
                        (int)atom.brushType() + 1
                    );
                }

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

    //to v8
    out.insert(
        8,
        [&](QJsonObject &doc) {}
    );

    //to v9
    out.insert(
        9,
        [&](QJsonObject &doc) {

            MapDatabase db(doc);

            QList<RPZAtom::BrushType> typesToFix { RPZAtom::BrushType::RoundBrush, RPZAtom::BrushType::Scissors };

            //iterate atoms
            for(auto atom : db.atoms()) {
                
                //restrict
                if(atom.type() != RPZAtom::Type::Brush) continue;
                if(!typesToFix.contains(atom.brushType())) continue;

                //update shape
                atom.setShape(
                    VectorSimplifier::reducePath(atom.shape())
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