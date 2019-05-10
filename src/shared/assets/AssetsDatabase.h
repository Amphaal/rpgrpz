#pragma once

#include <QHash>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QUuid>
#include <QCryptographicHash>
#include <QDir>

#include "JSONDatabase.h"
#include "AssetsDatabaseElement.h"

#include "src/helpers/_const.hpp"

class AssetsDatabase : public JSONDatabase, public AssetsDatabaseElement {
    public:
        AssetsDatabase();
        bool insertAsset(QUrl &url, AssetsDatabaseElement* parent);

    protected:
        const QString defaultJsonDoc() override;
        const QString dbPath() override;
    
    private:
        void _includeStaticComponents();
        void _includeDbComponents();

        QHash<AssetsDatabaseElement::Type, AssetsDatabaseElement*> _staticElements;
};