#include "MapTools.h"

MapTools::MapTools(QWidget* parent) : QToolBar(parent) {
    
    //self
    this->layout()->setMargin(0);
    this->setMovable(true);
    this->setIconSize(QSize(16,16));
    this->setFloatable(true);

    //draw
    this->addSeparator();
    auto draw = new QAction(this);
    draw->setIcon(QIcon(":/icons/app/tools/pen.png"));
    draw->setData(MapTools::Actions::Draw);
    draw->setIconText("Dessiner");
    draw->setCheckable(true);
    this->addAction(draw);

    //draw size
    auto sizer = new QSpinBox(this);
    sizer->setMinimum(1);
    QObject::connect(
        sizer, qOverload<int>(&QSpinBox::valueChanged),
        [&](const int i) {
            emit penSizeChanged(i);
        }
    );
    this->addWidget(sizer);
    this->addSeparator();

    //to north
    auto tonorth = new QAction(this);
    tonorth->setIcon(QIcon(":/icons/app/tools/north.png"));
    tonorth->setData(MapTools::Actions::RotateToNorth);
    tonorth->setIconText("Repositionner vers le nord");
    this->addAction(tonorth);
    this->addSeparator();
}

void MapTools::unselectAllTools() {
    for(auto &action : this->actions()) {
        action->setChecked(false);
    }
}
