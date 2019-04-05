#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>

class MapTools : public QToolBar {

    Q_OBJECT

    public:
        enum Actions { None, Select, Scroll, Draw, Rotate };
        MapTools(QWidget * parent = nullptr); 
        void unselectAllTools();
};