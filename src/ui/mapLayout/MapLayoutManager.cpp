#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QGraphicsView* viewToMimic, AtomsStorage* mapMaster, QWidget *parent) : QWidget(parent) {

    this->_tree = new MapLayoutTree(mapMaster, this);
    this->_layerSelector = new LayerSelector(this);
    
    this->_mapParamBtn = new QPushButton(QIcon(QStringLiteral(u":/icons/app/tools/cog.png")), "");
    this->_mapParamBtn->setToolTip(tr("Map parameters"));
    QObject::connect(
        this->_mapParamBtn, &QPushButton::pressed,
        [=]() {
            MapParametersForm form(mapMaster, parent);
            form.exec();
        }
    );
    
    auto layout = new QVBoxLayout;
    this->setLayout(layout);

    auto line = new QHBoxLayout;
    line->addWidget(this->_mapParamBtn, 0);
    line->addStretch(1);
    line->addWidget(this->_layerSelector, 0);

    layout->addLayout(line);
    layout->addWidget(this->_tree, 1);

    layout->setSpacing(2);
    layout->setMargin(5);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
}

MapLayoutTree* MapLayoutManager::tree() {
    return this->_tree;
}

LayerSelector* MapLayoutManager::layerSelector(){
    return this->_layerSelector;
}