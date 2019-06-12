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
    
    //reset
    auto reset = RPZActions::resetView();
    reset->setData(MapTools::Actions::ResetView);
    this->addAction(reset);
    this->addSeparator();

}

void MapTools::_onToolSelectionChanged(QAction *action) {
    return emit actionRequested((MapTools::Actions)action->data().toInt());
}