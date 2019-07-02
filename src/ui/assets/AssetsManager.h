#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QJsonDocument>
#include <QDir>
#include <QFlags>
#include <QJsonObject>

#include "src/helpers/_appContext.h"
#include "base/AssetsTreeView.h"

class AssetsManager : public QWidget {

    public:
        AssetsManager(QWidget * parent = nullptr);
        AssetsTreeView* tree();

    private:
        AssetsTreeView* _tree;
};