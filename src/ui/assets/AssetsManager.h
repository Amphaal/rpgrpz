#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QDir>
#include <QFlags>
#include <QJsonObject>

#include "src/helpers/_const.hpp"
#include "base/AssetsTreeView.hpp"
#include "AssetsPreviewer.h"

class AssetsManager : public QWidget {

    Q_OBJECT
    
    public:
        AssetsManager(QWidget * parent = nullptr);

    private:
        AssetsTreeView* _tree;
        AssetsPreviewer* _previewer;
};