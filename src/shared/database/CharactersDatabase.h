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
        static CharactersDatabase* get();

        const RPZMap<RPZCharacter>& characters() const;
        const RPZCharacter character(SnowFlake::Id characterId) const;
        RPZCharacter addNewCharacter();
        void removeCharacter(const SnowFlake::Id &toRemove);
        void updateCharacter(const RPZCharacter &updated);

    protected:
        const JSONDatabase::Version apiVersion() const override;
        JSONDatabase::Model _getDatabaseModel() override;
        void _setupLocalData() override;

    private:
        static inline CharactersDatabase* _singleton = nullptr;
        
        RPZMap<RPZCharacter> _characters;
        
        CharactersDatabase();

        void _writeCharactersToDb();
        
};