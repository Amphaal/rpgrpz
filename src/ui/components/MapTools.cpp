#include "MapTools.h"

MapTools::MapTools(QWidget* parent) : QToolBar(parent) {
    
    //self
    this->layout()->setMargin(0);
    this->setMovable(true);
    this->setIconSize(QSize(16,16));
    this->setFloatable(true);

    //draw
    auto draw = new QAction(this);
    auto draw_icon = QIcon(":/icons/app/pen.png");
    draw->setIcon(draw_icon);
    draw->setData(MapTools::Actions::Draw);
    draw->setIconText("Dessiner");
    draw->setCheckable(true);
    this->addAction(draw);
}

void MapTools::unselectAllTools() {
    for(auto action : this->actions()) {
        action->setChecked(false);
    }
}