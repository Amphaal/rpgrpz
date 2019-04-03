#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>

class MapTools : public QToolBar {

    Q_OBJECT

    public:
        enum Actions { Navigate, Draw };
        MapTools(QWidget * parent = nullptr); 
    
};