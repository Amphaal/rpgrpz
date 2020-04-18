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

#include "src/helpers/_appContext.h"
#include "src/shared/database/_base/JSONDatabase.h"

#include "src/shared/models/character/RPZCharacter.hpp"
#include "src/shared/models/_base/RPZMap.hpp"

class CharactersDatabase : public QObject, public JSONDatabase {
    
    Q_OBJECT

    signals:
        void databaseChanged();
        void characterAdded(const RPZCharacter &);
        void characterRemoved(const RPZCharacter::Id &);
        void characterUpdated(const RPZCharacter &);

    public:
        static CharactersDatabase* get();

        const RPZMap<RPZCharacter>& characters() const;
        const RPZCharacter character(RPZCharacter::Id characterId) const;
        RPZCharacter addNewCharacter();
        void removeCharacter(const RPZCharacter::Id &toRemove);
        void updateCharacter(const RPZCharacter &updated);

    protected:
        JSONDatabase::Version apiVersion() const override;
        JSONDatabase::Model _getDatabaseModel() override;
        void _setupLocalData() override;
        const QJsonObject _updatedInnerDb() override;

    private:
        static inline CharactersDatabase* _singleton = nullptr;
        
        RPZMap<RPZCharacter> _characters;
        
        CharactersDatabase();

        void _writeCharactersToDb();
        
};