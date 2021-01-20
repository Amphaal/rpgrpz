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

#include "PlaylistDatabase.h"

PlaylistDatabase* PlaylistDatabase::get() {
    if (!_singleton) {
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
}

void PlaylistDatabase::_setupLocalData() {
    // fill URLs
    for (const auto i : this->entityAsArray(QStringLiteral(u"ids"))) {
        this->_ytIds.insert(i.toString());
    }

    // fill names
    auto names = this->entityAsObject(QStringLiteral(u"names"));
    for (auto i = names.begin(); i != names.end(); i++) {
        this->_trackNameById.insert(i.key(), i.value().toString());
    }
}

QSet<PlaylistDatabase::VideoId> PlaylistDatabase::ytIds() const {
    return this->_ytIds;
}

void PlaylistDatabase::addYoutubeId(const PlaylistDatabase::VideoId &url) {
    this->_ytIds.insert(url);
    this->save();
}

void PlaylistDatabase::removeYoutubeId(const PlaylistDatabase::VideoId &url) {
    this->_ytIds.remove(url);
    this->save();
}

JSONDatabase::Model PlaylistDatabase::_getDatabaseModel() {
    return {
        { { QStringLiteral(u"ids"), JSONDatabase::EntityType::Array }, &this->_ytIds },
        { { QStringLiteral(u"names"), JSONDatabase::EntityType::Object }, &this->_trackNameById },
    };
}

QString PlaylistDatabase::trackName(const PlaylistDatabase::VideoId &ytId) {
    return this->_trackNameById.value(ytId);
}

void PlaylistDatabase::setTrackName(const PlaylistDatabase::VideoId &ytId, const QString &name) {
    this->_trackNameById.insert(ytId, name);
    this->save();
}

JSONDatabase::Version PlaylistDatabase::apiVersion() const {
    return 1;
}

const QJsonObject PlaylistDatabase::_updatedInnerDb() {
    auto db = this->db();

    updateFrom(
        db,
        QStringLiteral(u"ids"),
        this->_ytIds
    );

    updateFrom(
        db,
        QStringLiteral(u"names"),
        this->_trackNameById
    );

    return db;
}
