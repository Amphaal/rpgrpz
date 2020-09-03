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

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <audiotube/PlayerConfig.h>

#include "src/shared/database/_base/JSONDatabase.h"

class PlaylistDatabase : public JSONDatabase {
 public:
    // singleton
    using VideoId = QString;
    static PlaylistDatabase* get();

    void addYoutubeId(const PlaylistDatabase::VideoId &ytId);
    void removeYoutubeId(const PlaylistDatabase::VideoId &ytId);

    QString trackName(const PlaylistDatabase::VideoId &ytId);
    void setTrackName(const PlaylistDatabase::VideoId &ytId, const QString &name);

    QSet<PlaylistDatabase::VideoId> ytIds() const;

 protected:
    void _setupLocalData() override;
    JSONDatabase::Model _getDatabaseModel() override;
    JSONDatabase::Version apiVersion() const override;
    const QJsonObject _updatedInnerDb() override;

 private:
    // singleton
    PlaylistDatabase();
    explicit PlaylistDatabase(const QJsonObject &doc);
    static inline PlaylistDatabase* _singleton = nullptr;

    QSet<PlaylistDatabase::VideoId> _ytIds;
    QHash<PlaylistDatabase::VideoId, QString> _trackNameById;
};
