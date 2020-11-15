// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

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

#include "ConnectivityHelper.h"

ConnectivityHelper::ConnectivityHelper(QObject *parent) : QObject(parent) {
    //
    this->_cmThread = new CMThread;
    QObject::connect(
        this->_cmThread, &CMThread::connectivityChanged,
        this, &ConnectivityHelper::_onConnectivityChanged
    );
}

void ConnectivityHelper::startWorking() {
    this->_cmThread->start();
}

ConnectivityHelper::~ConnectivityHelper()  {
    //
    this->_mayClearUPnPThread();

    //
    if(this->_cmThread) {
        this->_cmThread->exit();
        this->_cmThread->wait();
        this->_cmThread->disconnect();
        delete this->_cmThread;
        this->_cmThread = nullptr;
    }
}

void ConnectivityHelper::_mayClearUPnPThread() {
    if (!this->_upnpThread) return;

    this->_upnpThread->exit();
    this->_upnpThread->wait();
    this->_upnpThread->disconnect();
    delete this->_upnpThread;
    this->_upnpThread = nullptr;
}

void ConnectivityHelper::_requestUPnPHandshake() {
    //
    this->_mayClearUPnPThread();

    //
    this->_upnpThread = new uPnPThread(
        AppContext::UPNP_DEFAULT_TARGET_PORT,
        AppContext::UPNP_REQUEST_DESCRIPTION
    );
    QObject::connect(
        this->_upnpThread, &uPnPThread::uPnPSuccess,
        this, &ConnectivityHelper::_onUPnPSuccess
    );
    QObject::connect(
        this->_upnpThread, &uPnPThread::uPnPError,
        this, &ConnectivityHelper::_onUPnPError
    );

    //
    this->_upnpThread->start();
    qDebug() << "Connectivity : Trying to open uPnP port"
             << qUtf8Printable(AppContext::UPNP_DEFAULT_TARGET_PORT)
             << "as"
             << qUtf8Printable(AppContext::UPNP_REQUEST_DESCRIPTION);
}

void ConnectivityHelper::_onUPnPError() {
    qDebug() << "Connectivity : uPnP failed !";
    emit uPnPStateChanged(tr("No"));
    emit remoteAddressStateChanged(tr("<UPnP Failed>"), RPZStatusLabel::State::Error);
}

void ConnectivityHelper::_onUPnPSuccess(const QString &localIP, const QString &extIP, const QString &protocol, const QString &negociatedPort) {
    //
    auto out = QStringLiteral(u"OK [port: %1]");
    out = out.arg(negociatedPort);

    //
    qDebug() << "Connectivity : uPnP"
             << qUtf8Printable(protocol)
             << qUtf8Printable(out);

    //
    emit remoteAddressStateChanged(extIP);
    emit uPnPStateChanged(out);
}

void ConnectivityHelper::_onConnectivityChanged(bool isConnectedToInternet) {
    //
    if (!isConnectedToInternet) {
        qDebug() << "Connectivity : Network not accessible, letting default label";
        emit remoteAddressStateChanged(this->_getErrorText(), RPZStatusLabel::State::Error);
        emit uPnPStateChanged         (this->_getErrorText(), RPZStatusLabel::State::Error);
    } else {
        qDebug() << "Connectivity : Network accessible, trying to get IPs and UPnP...";
        emit remoteAddressStateChanged(this->_getWaitingText(), RPZStatusLabel::State::Processing);
        emit uPnPStateChanged         (this->_getWaitingText(), RPZStatusLabel::State::Processing);
        this->_requestUPnPHandshake();
    }
}

///
///
///

const QString ConnectivityHelper::_getWaitingText() const {
    return tr("<Searching...>");
}

const QString ConnectivityHelper::_getErrorText() const {
    return tr("<Error>");
}
