#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QDir>
#include <QFlags>
#include <QJsonObject>

#include "src/helpers/_appContext.h"
#include "base/AssetsTreeView.h"
#include "AssetsPreviewer.h"

class AssetsManager : public QWidget {

    Q_OBJECT
    
    public:
        AssetsManager(QWidget * parent = nullptr);

    private:
        AssetsTreeView* _tree;
        AssetsPreviewer* _previewer;
};