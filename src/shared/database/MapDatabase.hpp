#pragma once

#include <QString>
#include <QVector>
#include "src/shared/network/RPZAsset.hpp"

#include "base/JSONDatabase.h"

class MapDatabase : public JSONDatabase {
    
    public:
        MapDatabase(QString &filePath) : _filePath(filePath) {
            JSONDatabase::_instanciateDb();
        };

        void saveIntoFile(QVector<RPZAsset> &elements) {
            
            auto copy = this->_db.object();

            //reseting "elements" object
            auto db_elems = QJsonObject();
            for(auto &elem : elements) {

                auto asset_id = elem.id().toString(QUuid::WithoutBraces);
                auto casted = QJsonObject::fromVariantHash(
                    elem.toVariantHash()
                );

                db_elems[asset_id] = casted;
            }
            copy["elements"] = db_elems;

            //saving...
            this->_updateDbFile(copy);
            qDebug() << "Map database : saving " << elements.count() << " elements";
        };

        QVector<RPZAsset> toAssets() {
            QVector<RPZAsset> out;

            auto db_elems = this->_db["elements"].toObject();

            for(auto &e : db_elems) {
              auto asset = RPZAsset::fromVariantHash(
                  e.toObject().toVariantHash()
              );
              out.append(asset);
            }

            qDebug() << "Map database : " << out.count() << " elements read";

            return out;
        }

    protected:
        const QString defaultJsonDoc() override {
            return "{\"version\":" + QString::number(this->apiVersion()) + ",\"elements\":{}}";
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