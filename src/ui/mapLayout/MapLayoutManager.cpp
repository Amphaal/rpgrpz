#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(QGraphicsView* viewToMimic, QWidget *parent) : QWidget(parent) {

    this->_tree = new MapLayoutTree(this);
    this->_FoWOpacitySlider = new QSlider(this);
    this->_FoWReversedChk = new QCheckBox(this);

    //param btn
    this->_mapParamBtn = new QPushButton(QIcon(QStringLiteral(u":/icons/app/tools/cog.png")), "");
    this->_mapParamBtn->setToolTip(QObject::tr("Map parameters"));
    QObject::connect(
        this->_mapParamBtn, &QPushButton::pressed,
        this, &MapLayoutManager::_handleMapParametersEdition
    );

    //fow slider
    this->_FoWOpacitySlider->setToolTip(QObject::tr("Fog of war opacity"));
    this->_FoWOpacitySlider->setOrientation(Qt::Orientation::Horizontal);
    this->_FoWOpacitySlider->setValue(50);
    this->_FoWOpacitySlider->setMinimum(0);
    this->_FoWOpacitySlider->setMaximum(100);
    QObject::connect(
        this->_FoWOpacitySlider, &QSlider::valueChanged,
        [=](int state) {
            //TODO
        }
    );

    //fow chk
    this->_FoWReversedChk->setText(QObject::tr("Reverse fog"));
    QObject::connect(
        this->_FoWReversedChk, &QCheckBox::stateChanged,
        [=](int state) {
            //TODO
        }
    );

    //events from map
    QObject::connect(
        HintThread::hint(), &AtomsStorage::mapParametersChanged,
        [=](const RPZMapParameters &mParams) {
            this->_currentMapParameters = mParams;
            //TODO fill fow chk && slider
        }
    );

    auto layout = new QVBoxLayout;
    this->setLayout(layout);

    auto line = new QHBoxLayout;
    line->addWidget(this->_mapParamBtn, 0);
    line->addStretch(1);

    auto foWGroup = new QGroupBox(QObject::tr("Fog of war"));
    foWGroup->setAlignment(Qt::AlignHCenter);
    auto fowL = new QHBoxLayout;
    foWGroup->setLayout(fowL);
    fowL->addWidget(new QLabel(QObject::tr("FoW opacity")));
    fowL->addWidget(this->_FoWOpacitySlider, 1);
    fowL->addWidget(this->_FoWReversedChk, 0);

    layout->addLayout(line);
    layout->addWidget(this->_tree, 1);
    layout->addWidget(foWGroup, 0);

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