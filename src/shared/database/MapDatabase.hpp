#pragma once

#include <QString>
#include <QVector>
#include "src/shared/models/RPZAtom.hpp"

#include "base/JSONDatabase.h"

class MapDatabase : public JSONDatabase {
    
    public:
        MapDatabase(QString &filePath) : _filePath(filePath) {
            JSONDatabase::_instanciateDb();
        };

        void saveIntoFile(QVector<RPZAtom> &atoms) {
            
            auto copy = this->_db.object();

            //reseting "atoms" object
            auto db_atoms = QJsonObject();
            for(auto &atom : atoms) {

                auto atom_id = atom.id().toString(QUuid::WithoutBraces);
                auto casted = QJsonObject::fromVariantHash(atom);

                db_atoms[atom_id] = casted;
            }
            copy["atoms"] = db_atoms;

            //saving...
            this->_updateDbFile(copy);
            qDebug() << "Map database : saving " << atoms.count() << " atoms";
        };

        QVector<RPZAtom> toAtoms() {
            QVector<RPZAtom> out;

            auto db_atoms = this->_db["atoms"].toObject();

            for(auto &e : db_atoms) {
              auto atom = RPZAtom(e.toObject().toVariantHash());
              out.append(atom);
            }

            qDebug() << "Map database : " << out.count() << " atoms read";

            return out;
        }

    protected:
        const QString defaultJsonDoc() override {
            return "{\"version\":" + QString::number(this->apiVersion()) + ",\"atoms\":{}}";
        }

        const QString dbPath() override {
            return this->_filePath;
        }

        const int apiVersion() override {
            return 2;
        }

        const int dbVersion() override {
            return this->_db["version"].toInt();
        }
    
    private:
        QString _filePath;
};