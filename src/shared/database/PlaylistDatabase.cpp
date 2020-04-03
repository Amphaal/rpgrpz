// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical resources available within the source code may 
// use a different license and copyright : please refer to their metadata
// for further details. Graphical resources without explicit references to a
// different license and copyright still refer to this GNU General Public License.

#include "PlaylistDatabase.h"

PlaylistDatabase* PlaylistDatabase::get() {
    if(!_singleton) {
        _singleton = new PlaylistDatabase();
    }
    return _singleton;
}

PlaylistDatabase::PlaylistDatabase(const QJsonObject &doc) : JSONDatabase(QStringLiteral(u"PlaylistDB")) { 
    this->_setupFromDbCopy(doc);
}
PlaylistDatabase::PlaylistDatabase() : JSONDatabase(QStringLiteral(u"PlaylistDB")) {
    this->_initDatabaseFromJSONFile(
        AppContext::getPlaylistFileLocation()
    );
};

void PlaylistDatabase::_setupLocalData() {
    //fill URLs
    for(const auto i : this->entityAsObject(QStringLiteral(u"urls"))) {
        this->_ytUrls.insert(i.toString());
    }
}

JSONDatabase::Model PlaylistDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"urls"), JSONDatabase::EntityType::Array }, &this->_ytUrls }
    };
}

JSONDatabase::Version PlaylistDatabase::apiVersion() const {
    return 1;
}

const QJsonObject PlaylistDatabase::_updatedInnerDb() {
    auto db = this->db();

    updateFrom(
        db, 
        QStringLiteral(u"urls"), 
        this->_ytUrls
    );

    return db;
    
}