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
    auto draw_icon = QIcon(":/icons/app/pen.png");
    draw->setIcon(draw_icon);
    draw->setData(MapTools::Actions::Draw);
    draw->setIconText("Dessiner");
    draw->setCheckable(true);
    this->addAction(draw);

    //draw size
    auto sizer = new QSpinBox(this);
    sizer->setMinimum(1);

    QObject::connect(
        sizer, qOverload<int>(&QSpinBox::valueChanged),
        [&](int i) {
            emit penSizeChanged(i);
        }
    );
    this->addWidget(sizer);
    this->addSeparator();
}

void MapTools::unselectAllTools() {
    for(auto action : this->actions()) {
        action->setChecked(false);
    }
}
