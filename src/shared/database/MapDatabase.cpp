#include "MapDatabase.h"

MapDatabase::MapDatabase(QString &filePath) : _filePath(filePath) {
    JSONDatabase::_instanciateDb();
};

void MapDatabase::saveIntoFile(RPZMap<RPZAtom> &atoms) {
    
    auto copy = this->_db.object();

    //reseting "atoms" object
    auto db_atoms = QJsonArray();
    for(auto &atom : atoms) {

        auto casted = QJsonObject::fromVariantHash(atom);

        db_atoms.append(casted);
    }
    copy["atoms"] = db_atoms;

    //saving...
    this->_updateDbFile(copy);
    qDebug() << "Map database : saving " << atoms.count() << " atoms";
};

RPZMap<RPZAtom> MapDatabase::toAtoms() {
    RPZMap<RPZAtom> out;

    auto db_atoms = this->_db["atoms"].toArray();

    for(auto &e : db_atoms) {
        auto atom = RPZAtom(e.toObject().toVariantHash());
        out.insert(atom.id(), atom);
    }

    qDebug() << "Map database : " << out.count() << " atoms read";

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

const int MapDatabase::dbVersion() {
    return this->_db["version"].toInt();
}