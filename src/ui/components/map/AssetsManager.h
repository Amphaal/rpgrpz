#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QDir>
#include <QFlags>
#include <QJsonObject>

#include "src/helpers/_const.cpp"
#include "AssetsNavigator.h"
#include "AssetsPreviewer.h"

class AssetsManager : public QWidget {

    Q_OBJECT
    
    public:
        AssetsManager(QWidget * parent = nullptr);

    private:
        AssetsNavigator* _tree;
        AssetsPreviewer* _previewer;

        QJsonDocument _getCoordinator();
        void _addAppObjectsToTree();
        void _refreshTree();
};