#pragma once

#include <QString>
#include <QVector>
#include "src/shared/models/entities/RPZAtom.hpp"

#include "base/JSONDatabase.h"

class MapDatabase : public JSONDatabase {
    
    public:
        MapDatabase(QString &filePath) : _filePath(filePath) {
            JSONDatabase::_instanciateDb();
        };

        void saveIntoFile(QVector<RPZAtom> &atoms) {
            
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

        QVector<RPZAtom> toAtoms() {
            QVector<RPZAtom> out;

            auto db_atoms = this->_db["atoms"].toArray();

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
            return 3;
        }

        const int dbVersion() override {
            return this->_db["version"].toInt();
        }
    
    private:
        QString _filePath;
};