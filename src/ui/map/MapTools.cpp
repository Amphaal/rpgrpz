#include "MapTools.h"

MapTools::MapTools(QWidget* parent) : QToolBar(parent) {
    
    //self
    this->layout()->setMargin(0);
    this->setIconSize(QSize(16, 16));
    this->setMovable(true);
    this->setFloatable(true);

    QObject::connect(
        this, &QToolBar::actionTriggered,
        this, &MapTools::_onToolSelectionChanged
    );

    //reset tool
    auto resetTool = RPZActions::resetTool();
    resetTool->setData(MapTools::Actions::ResetTool);
    this->addAction(resetTool);
    
    this->addSeparator();

    //reset view
    auto resetV = RPZActions::resetView();
    resetV->setData(MapTools::Actions::ResetView);
    this->addAction(resetV);

}

void MapTools::_onToolSelectionChanged(QAction *action) {
    return emit actionRequested((MapTools::Actions)action->data().toInt());
}