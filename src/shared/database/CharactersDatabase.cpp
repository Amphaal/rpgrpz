// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "CharactersDatabase.h"

CharactersDatabase* CharactersDatabase::get() {
    if (!_singleton) _singleton = new CharactersDatabase;
    return _singleton;
}

const RPZMap<RPZCharacter>& CharactersDatabase::characters() const {
    return this->_characters;
}

const RPZCharacter CharactersDatabase::character(RPZCharacter::Id characterId) const {
    return this->_characters.value(characterId);
}

RPZCharacter CharactersDatabase::addNewCharacter() {
    RPZCharacter character;
    character.shuffleId();

    // update
    this->updateCharacter(character);
    emit characterAdded(character);

    return character;
}

void CharactersDatabase::removeCharacter(const RPZCharacter::Id &toRemove) {
    // remove...
    this->_characters.remove(toRemove);

    this->_writeCharactersToDb();
    emit characterRemoved(toRemove);
}

void CharactersDatabase::updateCharacter(const RPZCharacter &updated) {
    // update...
    this->_characters.insert(updated.id(), updated);

    this->_writeCharactersToDb();
    emit characterUpdated(updated);
}

JSONDatabase::Version CharactersDatabase::apiVersion() const {
    return 1;
}

JSONDatabase::Model CharactersDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"characters"), JSONDatabase::EntityType::Object }, &this->_characters }
    };
}

void CharactersDatabase::_setupLocalData() {
    // fill characters...
    for (const auto i : this->entityAsObject(QStringLiteral(u"characters"))) {
        auto character = RPZCharacter(i.toVariant().toHash());
        this->_characters.insert(character.id(), character);
    }
}

CharactersDatabase::CharactersDatabase() : JSONDatabase(QStringLiteral(u"CharacterDB")) {
    this->_initDatabaseFromJSONFile(AppContext::getCharacterDatabaseLocation());
}

void CharactersDatabase::_writeCharactersToDb() {
    this->save();
    emit databaseChanged();
}

const QJsonObject CharactersDatabase::_updatedInnerDb() {
    auto db = this->db();

    updateFrom(
        db,
        QStringLiteral(u"characters"),
        this->_characters.toVMap()
    );

    return db;
}
