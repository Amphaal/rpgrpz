#pragma once

#include "src/helpers/_appContext.h"
#include "src/shared/database/_base/JSONDatabase.h"

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/shared/models/_base/RPZMap.hpp"

class CharactersDatabase : public QObject, public JSONDatabase {
    
    Q_OBJECT

    signals:
        void databaseChanged();

    public:
        static CharactersDatabase* get() {
            if(!_singleton) _singleton = new CharactersDatabase(AppContext::getCharacterDatabaseLocation());
            return _singleton;
        };

        const RPZMap<RPZCharacter>& characters() const {
            return this->_characters;
        }
        
        const RPZCharacter& character(snowflake_uid characterId) {
            return this->characters()[characterId];
        }

        RPZCharacter addNewCharacter() {
            
            RPZCharacter character;
            character.shuffleId();

            //update
            this->updateCharacter(character);

            return character;

        }

        void removeCharacter(const snowflake_uid &toRemove) {
            
            //remove...
            this->_characters.remove(toRemove);

            this->_writeCharactersToDb();
            
        }

        void updateCharacter(const RPZCharacter &updated) {
            
            //update...
            this->_characters.insert(updated.id(), updated);

            this->_writeCharactersToDb();

        }

    protected:

        const int apiVersion() {
            return 1;
        };

        JSONDatabase::Model _getDatabaseModel() {
            return {
                { { QStringLiteral(u"characters"), JSONDatabase::EntityType::Object }, &this->_characters }
            };
        }

        void _setupLocalData() override {

            //fill characters...
            for(auto &i : this->entityAsObject(QStringLiteral(u"characters"))) {
                auto character = RPZCharacter(i.toVariant().toHash());
                this->_characters.insert(character.id(), character);
            }

        }

    private:
        static inline CharactersDatabase* _singleton = nullptr;
        
        RPZMap<RPZCharacter> _characters;
        
        CharactersDatabase(const QString &dbFilePath) : JSONDatabase(dbFilePath) {}

        void _writeCharactersToDb() {

            auto obj = this->db();

                updateFrom(
                    obj, 
                    QStringLiteral(u"characters"), 
                    this->_characters.toVMap()
                );
            
            this->_updateDbFile(obj);
            emit databaseChanged();

        }
        
};