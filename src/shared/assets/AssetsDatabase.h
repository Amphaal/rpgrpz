#pragma once

#include <QString>

#include "JSONDatabase.h"
#include "AssetsDatabaseElement.hpp"

#include "src/helpers/_const.hpp"

class AssetsDatabase : public JSONDatabase, public AssetsDatabaseElement {
    public:
        AssetsDatabase();

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
};