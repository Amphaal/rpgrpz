#pragma once

#include <QString>
#include <QVector>
#include "src/shared/models/RPZAtom.h"

#include "src/shared/models/base/RPZMap.hpp"

#include "base/JSONDatabase.h"

class MapDatabase : public JSONDatabase {
    
    public:
        MapDatabase(const QString &filePath);

        void saveIntoFile(RPZMap<RPZAtom> &atoms);

        RPZMap<RPZAtom> toAtoms();

    protected:
        const QString defaultJsonDoc() override;

        const QString dbPath() override;

        const int apiVersion() override;

        const int dbVersion() override;
    
    private:
        QString _filePath;
};