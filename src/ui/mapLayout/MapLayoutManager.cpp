// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#include "MapLayoutManager.h"

MapLayoutManager::MapLayoutManager(MapView* associatedMapView, QWidget *parent) : QWidget(parent), _associatedMapView(associatedMapView) {
    // tree
    this->_tree = new MapLayoutTree(this->_associatedMapView, this);

    // param btn
    this->_mapParamBtn = new QPushButton(QIcon(QStringLiteral(u":/icons/app/tools/cog.png")), "");
    this->_mapParamBtn->setToolTip(QObject::tr("Map parameters"));
    QObject::connect(
        this->_mapParamBtn, &QPushButton::pressed,
        this, &MapLayoutManager::_handleMapParametersEdition
    );

    // fow slider
    this->_FoWOpacitySlider = new QSlider(this);
    this->_FoWOpacitySlider->setToolTip(QObject::tr("Fog of war opacity"));
    this->_FoWOpacitySlider->setOrientation(Qt::Orientation::Horizontal);
    this->_FoWOpacitySlider->setMinimum(25);
    this->_FoWOpacitySlider->setMaximum(100);
    this->_FoWOpacitySlider->setValue((int)(AppContext::fogOpacity() * 100));
    QObject::connect(
        this->_FoWOpacitySlider, &QSlider::valueChanged,
        this, &MapLayoutManager::_fogOpacityChange
    );

    // fow reset
    this->_FoWResetBtn = new QPushButton(QObject::tr("Reset", "Reset fog"));
    this->_FoWResetBtn->setToolTip(QObject::tr("Reset fog"));
    QObject::connect(
        this->_FoWResetBtn, &QPushButton::pressed,
        this, &MapLayoutManager::_handleFogReset
    );

    // fow chk
    this->_FoWReversedChk = new QCheckBox(this);
    this->_FoWReversedChk->setText(QObject::tr("Reverse", "Reverse fog"));
    this->_FoWReversedChk->setToolTip(QObject::tr("Reverse fog"));
    QObject::connect(
        this->_FoWReversedChk, &QCheckBox::stateChanged,
        this, &MapLayoutManager::_changeFogMode
    );

    // events from map
    QObject::connect(
        HintThread::hint(), &AtomsStorage::mapSetup,
        this, &MapLayoutManager::_onMapSetup
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
    fowL->addWidget(new QLabel(QObject::tr("Opacity", "FoW opacity")));
    fowL->addWidget(this->_FoWOpacitySlider, 1);
    fowL->addWidget(this->_FoWReversedChk, 0);
    fowL->addWidget(this->_FoWResetBtn, 0);

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
    MapParametersForm form(this->_currentMapParameters, this->_associatedMapView, this->parentWidget());
    if (!form.exec()) return;

    // get payload, update params
    auto payload = HintThread::hint()->generateResetPayload();
    payload.setMapParams(form.getParametersFromWidgets());

    // recommit
    AlterationHandler::get()->queueAlteration(this->_tree->mlModel, payload);
}

void MapLayoutManager::_changeFogMode(int) {
    auto mode = this->_FoWReversedChk->isChecked() ? RPZFogParams::Mode::PathIsButFog : RPZFogParams::Mode::PathIsFog;

    // get payload, update params
    FogModeChangedPayload payload(mode);

    // commit
    AlterationHandler::get()->queueAlteration(this->_tree->mlModel, payload);
}

void MapLayoutManager::_fogOpacityChange(int value) {
    AppContext::defineFogOpacity((double)value / 100);
}

void MapLayoutManager::_onMapSetup(const RPZMapParameters &mParams, const RPZFogParams &fParams) {
    // define current params
    this->_currentMapParameters = mParams;

    // define default state
    QSignalBlocker b2(this->_FoWReversedChk);
    this->_FoWReversedChk->setChecked(fParams.mode() == RPZFogParams::Mode::PathIsButFog);
}

void MapLayoutManager::_handleFogReset() {
    // payload
    FogChangedPayload payload(FogChangedPayload::ChangeType::Reset);

    // commit
    AlterationHandler::get()->queueAlteration(this->_tree->mlModel, payload);
}
