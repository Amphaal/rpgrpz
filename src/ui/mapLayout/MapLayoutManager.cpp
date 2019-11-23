#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QGraphicsView* viewToMimic, QWidget *parent) : QWidget(parent) {

    this->_tree = new MapLayoutTree(this);

    this->_mapParamBtn = new QPushButton(QIcon(QStringLiteral(u":/icons/app/tools/cog.png")), "");
    this->_mapParamBtn->setToolTip(QObject::tr("Map parameters"));
    QObject::connect(
        this->_mapParamBtn, &QPushButton::pressed,
        this, &MapLayoutManager::_handleMapParametersEdition
    );

    QObject::connect(
        HintThread::hint(), &AtomsStorage::mapParametersChanged,
        [=](const RPZMapParameters &mParams) {
            this->_currentMapParameters = mParams;
        }
    );

    auto layout = new QVBoxLayout;
    this->setLayout(layout);

    auto line = new QHBoxLayout;
    line->addWidget(this->_mapParamBtn, 0);
    line->addStretch(1);

    layout->addLayout(line);
    layout->addWidget(this->_tree, 1);

    layout->setSpacing(2);
    layout->setMargin(5);

    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
}

MapLayoutTree* MapLayoutManager::tree() {
    return this->_tree;
}

void MapLayoutManager::_handleMapParametersEdition() {
    
    MapParametersForm form(this->_currentMapParameters, this->parentWidget());
    if(!form.exec()) return;
    
    //get payload, update params
    auto payload = HintThread::hint()->generateResetPayload();
    payload.setMapParams(form.getParametersFromWidgets());

    //recommit
    AlterationHandler::get()->queueAlteration(Payload::Source::Local_MapLayout, payload);

}