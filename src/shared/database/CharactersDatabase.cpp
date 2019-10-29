#include "CharactersDatabase.h"

CharactersDatabase* CharactersDatabase::get() {
    if(!_singleton) _singleton = new CharactersDatabase;
    return _singleton;
};

const RPZMap<RPZCharacter>& CharactersDatabase::characters() const {
    return this->_characters;
}

const RPZCharacter CharactersDatabase::character(SnowFlake::Id characterId) const {
    return this->_characters.value(characterId);
}

RPZCharacter CharactersDatabase::addNewCharacter() {
    
    RPZCharacter character;
    character.shuffleId();

    //update
    this->updateCharacter(character);

    return character;

}

void CharactersDatabase::removeCharacter(const SnowFlake::Id &toRemove) {
    
    //remove...
    this->_characters.remove(toRemove);

    this->_writeCharactersToDb();
    
}

void CharactersDatabase::updateCharacter(const RPZCharacter &updated) {
    
    //update...
    this->_characters.insert(updated.id(), updated);

    this->_writeCharactersToDb();

}


const JSONDatabase::Version CharactersDatabase::apiVersion() const {
    return 1;
}

JSONDatabase::Model CharactersDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"characters"), JSONDatabase::EntityType::Object }, &this->_characters }
    };
}

void CharactersDatabase::_setupLocalData() {

    //fill characters...
    for(auto i : this->entityAsObject(QStringLiteral(u"characters"))) {
        auto character = RPZCharacter(i.toVariant().toHash());
        this->_characters.insert(character.id(), character);
    }

}

CharactersDatabase::CharactersDatabase() : JSONDatabase(QStringLiteral(u"CharacterDB")) {
    this->_initDatabaseFromJSONFile(AppContext::getCharacterDatabaseLocation());
}

void CharactersDatabase::_writeCharactersToDb() {

    auto obj = this->db();

        updateFrom(
            obj, 
            QStringLiteral(u"characters"), 
            this->_characters.toVMap()
        );
    
    this->_updateDbFile(obj);
    emit databaseChanged();

}

