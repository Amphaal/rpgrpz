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

#include "RPZStatusBar.h"

RPZStatusBar::RPZStatusBar(QWidget * parent) : QStatusBar(parent) {
    this->setContentsMargins(10, 0, 3, 0);
    this->setAutoFillBackground(true);

    // colors
    auto colors = this->palette();
    colors.setColor(QPalette::Window, "#DDD");
    this->setPalette(colors);

    // install...
    this->_installComponents();
    this->_installLayout();

    // update status bar on map file update
    QObject::connect(
        HintThread::hint(), &MapHint::mapStateChanged,
        this, &RPZStatusBar::_updateMapFileLabel
    );
}

void RPZStatusBar::_installComponents() {
    // ext ip
    this->_extIpLabel = new RPZStatusLabel(tr("External IP"));
    this->_extIpLabel->setToolTip(tr("Copy IP"));
    QObject::connect(
        this->_extIpLabel->dataLabel(), &QLabel::linkActivated,
        [](const QString &link) {
            // remove html tags
            auto s = link;
            s.remove(QRegExp("<[^>]*>"));
            QApplication::clipboard()->setText(s);

            // show tooltip
            QToolTip::showText(QCursor::pos(), tr("IP copied !"));
    });

    this->_mapFileLabel = new RPZStatusLabel(tr("Map"));
    this->_upnpStateLabel = new RPZStatusLabel("uPnP");

    this->_serverLogsBtn = new QPushButton("Logs");
    this->_serverLogsBtn->setVisible(false);
    this->_serverLogsBtn->setMaximumWidth(30);
    this->_serverLogsBtn->setMaximumHeight(18);

    this->_serverStateLabel = new RPZStatusLabel(tr("Server"));
    this->_dlStatus = new DownloadStatus;
    this->_activityIndicators = new ClientActivityBar;
}

void RPZStatusBar::_updateMapFileLabel(const QString &mapDescriptor, bool isMapDirty) {
    auto reflectDirtiness = mapDescriptor + (isMapDirty ? "*" : "");
    this->_mapFileLabel->updateState(reflectDirtiness);
}

void RPZStatusBar::connectingToServer() {
    this->_activityIndicators->setVisible(true);
}

void RPZStatusBar::connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) {
    this->_activityIndicators->setVisible(false);
    this->_dlStatus->setVisible(false);
}

void RPZStatusBar::setBoundServer(RPZServer* server) {
    if(!server) return;

    // init server logs modal to setup bindings
    this->_serverLogs = new ServerLogs(server, this);
    QObject::connect(
        this->_serverLogsBtn, &QPushButton::pressed,
        [=]() {
            this->_serverLogs->show();
        }
    );

    QObject::connect(
        server, &RPZServer::listening,
        this, &RPZStatusBar::_updateServerState_Listening
    );

    QObject::connect(
        server, &RPZServer::failed,
        this, &RPZStatusBar::_updateServerState_Failed
    );

    QObject::connect(
        server, &RPZServer::isActive,
        this, &RPZStatusBar::_onServerIsActive
    );

    QObject::connect(
        server, &RPZServer::isInactive,
        this, &RPZStatusBar::_onServerIsInactive
    );
}

void RPZStatusBar::_installLayout() {
    setUpdatesEnabled(false);

    auto leftPart = new QWidget;
    leftPart->setLayout(new QHBoxLayout);
    leftPart->layout()->setMargin(0);
    leftPart->layout()->addWidget(this->_serverLogsBtn);
    leftPart->layout()->addWidget(this->_serverStateLabel);
    leftPart->layout()->addWidget(new QLabel(" | "));
    leftPart->layout()->addWidget(this->_extIpLabel);
    leftPart->layout()->addWidget(new QLabel(" | "));
    leftPart->layout()->addWidget(this->_upnpStateLabel);

    // append components
    this->addPermanentWidget(leftPart);
    this->addPermanentWidget(new QWidget, 1);
    this->addPermanentWidget(this->_mapFileLabel);
    this->addPermanentWidget(this->_dlStatus);
    this->addPermanentWidget(this->_activityIndicators);

    setUpdatesEnabled(true);
}

///
///
///

void RPZStatusBar::_updateServerStateLabel(const QString &stateText, RPZStatusLabel::State state) {
    this->_serverStateLabel->updateState(
        stateText,
        state
    );
}

void RPZStatusBar::updateUPnPLabel(const QString &stateText, RPZStatusLabel::State state) {
    this->_upnpStateLabel->updateState(
        stateText,
        state
    );
}

void RPZStatusBar::updateExtIPLabel(const QString &stateText, RPZStatusLabel::State state) {
    auto asHTMLLink = [stateText]() {
        return "<a href='" + stateText + "'>" + stateText + "</a>";
    };

    this->_extIpLabel->updateState(
        state == RPZStatusLabel::State::Finished ? asHTMLLink() : stateText,
        state
    );
}

void RPZStatusBar::_updateServerState_Listening() {
    this->_updateServerStateLabel(QStringLiteral("OK"), RPZStatusLabel::State::Finished);
    this->_serverLogsBtn->setVisible(true);
}

void RPZStatusBar::_updateServerState_Failed() {
    this->_updateServerStateLabel(tr("Error"), RPZStatusLabel::State::Error);
}

void RPZStatusBar::updateServerState_NoServer() {
    this->_updateServerStateLabel(tr("No"), RPZStatusLabel::State::Finished);
}

void RPZStatusBar::_onServerIsActive() {
    this->_serverStateLabel->dataLabel()->setStyleSheet("color:green");
}

void RPZStatusBar::_onServerIsInactive() {
    this->_serverStateLabel->dataLabel()->setStyleSheet("color:black");
}
