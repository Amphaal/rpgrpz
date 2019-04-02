#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QDir>
#include <QJsonObject>

#include "src/helpers/_const.cpp"
#include "AssetsNavigator.h"

class AssetsManager : public QWidget {

    Q_OBJECT
    
    public:
        AssetsManager(QWidget * parent = nullptr);

    private:
        AssetsNavigator* _tree;

        QJsonDocument _getCoordinator();
        void _addAppObjectsToTree();
        void _refreshTree();
};