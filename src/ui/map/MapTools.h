#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QSpinBox>
#include <QActionEvent>

class MapTools : public QToolBar {

    Q_OBJECT

    public:
        enum Actions { None, Select, Scroll, Draw, Rotate, RotateToNorth, ResetView };
        MapTools(QWidget * parent = nullptr); 
        void unselectAllTools();
    
    signals:
        void penSizeChanged(const int newSize);
        void toolSelectionChanged(const MapTools::Actions &tool);

    private:
        bool _selectableToolSelected = false;
        void _onToolSelectionChanged(QAction *action);
        QVector<QAction*> _selectableTools;
        QAction* _defaultTool;
};