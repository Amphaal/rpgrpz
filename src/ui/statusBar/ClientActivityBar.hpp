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

#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

#include "src/ui/_others/ConnectivityObserver.h"

class ClientActivityIndicator : public QLabel {
    Q_OBJECT

 public:
    ClientActivityIndicator(const QPixmap &offState, const QPixmap &onState) : _off(offState), _on(onState) {
        this->setContentsMargins(0, 0, 0, 0);
        this->_setOff();

        QObject::connect(
            &_lingerTimer, &QTimer::timeout,
            [=](){
                if (!_activityPending) _setOff();
        });
    }

 public slots:
    void onActivityOn(qint64 bytes) {
        Q_UNUSED(bytes)
        _activityPending = true;
        _lingerTimer.start(_indicatorLingerMs);
        _setOn();
    }

    void onActivityOff() {
        _activityPending = false;
        if (!_lingerTimer.isActive()) _setOff();
    }

 private:
    QPixmap _on;
    QPixmap _off;

    static inline int _indicatorLingerMs = 330;

    QTimer _lingerTimer;
    bool _activityPending = false;

    void _setOff() {
        this->setPixmap(_off);
    }

    void _setOn() {
        this->setPixmap(_on);
    }
};

class ClientActivityBar : public QWidget, public ConnectivityObserver  {
 public:
    ClientActivityBar() {
        this->setVisible(false);
        this->setLayout(new QHBoxLayout);
        this->layout()->setContentsMargins(0, 0, 0, 0);
        this->layout()->setSpacing(2);

        // UPLOAD TRACKER
        _upLbl = new ClientActivityIndicator(
            QPixmap(QStringLiteral(u":/icons/app/connectivity/uploadG.png")),
            QPixmap(QStringLiteral(u":/icons/app/connectivity/upload.png"))
        );

        this->layout()->addWidget(_upLbl);

        // DOWNLOAD TRACKER
        _downLbl = new ClientActivityIndicator(
            QPixmap(QStringLiteral(u":/icons/app/connectivity/downloadG.png")),
            QPixmap(QStringLiteral(u":/icons/app/connectivity/download.png"))
        );

        this->layout()->addWidget(_downLbl);
    }

 protected:
    void connectingToServer() override {
        QObject::connect(
            this->_rpzClient, &JSONSocket::JSONUploading,
            _upLbl, &ClientActivityIndicator::onActivityOn
        );
        QObject::connect(
            this->_rpzClient, &JSONSocket::JSONUploaded,
            _upLbl, &ClientActivityIndicator::onActivityOff
        );

        QObject::connect(
            this->_rpzClient, &JSONSocket::JSONDownloading,
            _downLbl, &ClientActivityIndicator::onActivityOn
        );
        QObject::connect(
            this->_rpzClient, &JSONSocket::JSONDownloaded,
            _downLbl, &ClientActivityIndicator::onActivityOff
        );
    }

    void connectionClosed(bool hasInitialMapLoaded, const QString &errorMessage) override {
        this->_upLbl->onActivityOff();
        this->_downLbl->onActivityOff();
    }

 private:
    ClientActivityIndicator* _upLbl = nullptr;
    ClientActivityIndicator* _downLbl = nullptr;
};
