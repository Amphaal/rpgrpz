#pragma once

#include "src/helpers/_appContext.h"
#include "base/JSONDatabase.h"

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/shared/models/base/RPZMap.hpp"

class CharactersDatabase : public QObject, public JSONDatabase {
    
    Q_OBJECT

    signals:
        void databaseChanged();

    public:
        static CharactersDatabase* get() {
            if(!_singleton) {_singleton = new CharactersDatabase;}
            return _singleton;
        };

        RPZMap<RPZCharacter> characters() {
            RPZMap<RPZCharacter> out;
            for(auto &i : this->_characters().toVariantHash()) {
                auto character = RPZCharacter(i.toHash());
                out.insert(character.id(), character);
            }
            return out;
        }

        const QVector<snowflake_uid> characterIds() const {
            QVector<snowflake_uid> out;
            for(auto &key : this->_characters().keys()) out += key.toULongLong();
            return out;
        }

        RPZCharacter character(snowflake_uid characterId) {
            return this->characters().value(characterId);
        }

        RPZCharacter addNewCharacter() {
            
            RPZCharacter character;
            character.shuffleId();

            //update
            return this->updateCharacter(character);

        }

        void removeCharacter(const snowflake_uid &toRemove) {
            
            //copy
            auto obj = this->_db.object();

                //insert in db
                auto chars = this->_characters();
                auto idToRemove = QString::number(toRemove);
                chars.remove(idToRemove);
                obj["characters"] = chars;
            
            //save
            this->_updateDbFile(obj);
            emit databaseChanged();
            
        }

        RPZCharacter updateCharacter(const RPZCharacter &updated) {
            
            //copy
            auto obj = this->_db.object();

                //insert in db
                auto chars = this->_characters();
                chars.insert(
                    QString::number(updated.id()),
                    QJsonObject::fromVariantHash(updated)
                );
                obj["characters"] = chars;
            
            //save
            this->_updateDbFile(obj);
            emit databaseChanged();

            return updated;

        }

    protected:
        const QString dbPath() {
            return AppContext::getCharacterDatabaseLocation();
        };

        const int apiVersion() {
            return 1;
        };

        const QString defaultJsonDoc() { 
            return "{\"version\":" + QString::number(this->apiVersion()) + ",\"characters\":{}}"; 
        };

    private:
        static inline CharactersDatabase* _singleton = nullptr;
        
        CharactersDatabase() {
            this->_instanciateDb();
        }
        
        QJsonObject _characters() const {
            return this->_db["characters"].toObject();
        }

};