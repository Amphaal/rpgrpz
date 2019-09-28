#pragma once

#include "src/helpers/_appContext.h"
#include "base/JSONDatabase.h"

#include "src/shared/models/RPZCharacter.hpp"

class CharactersDatabase : public JSONDatabase {
    
    public:
        static CharactersDatabase* get() {
            if(!_singleton) _singleton = new CharactersDatabase;
            return _singleton;
        };

        QMap<snowflake_uid, RPZCharacter> characters() {
            QMap<snowflake_uid, RPZCharacter> out;
            for(auto &i : this->_characters().toVariantHash()) {
                auto character = RPZCharacter(i.toHash());
                out.insert(character.id(), character);
            }
            return out;
        }

        RPZCharacter addNewCharacter() {
            
            RPZCharacter character;
            character.shuffleId();

            //update
            return this->updateCharacter(character);

        }

        void removeCharacter(const RPZCharacter &toRemove) {
            //copy
            auto obj = this->_db.object();

                //insert in db
                auto chars = this->_characters();
                chars.remove(toRemove.idAsStr());
                obj["characters"] = chars;
            
            //save
            this->_updateDbFile(obj);
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
        
        QJsonObject _characters() {
            return this->_db["characters"].toObject();
        }

};