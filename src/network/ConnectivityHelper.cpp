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
    this->_pickPreferedConfiguration();

    QObject::connect(
        this->_ncm, &QNetworkConfigurationManager::configurationChanged,
        this, &ConnectivityHelper::_mustReInit
    );
}

void ConnectivityHelper::_mustReInit(const QNetworkConfiguration &config) {
    auto mustReInit = this->_nam->configuration() == config;

    const auto oldPrefConfig = this->_nam->configuration();
    this->_pickPreferedConfiguration();
    const auto newPrefConfig = this->_nam->configuration();
    auto mustReInit_2 = oldPrefConfig != newPrefConfig;

    qDebug() << "Connectivity : Detected an alteration in network"
             << config.name()
             << ", repicking interface "
             << this->_nam->configuration().name();

    if (mustReInit || mustReInit_2) {
        this->init();
    }
}

void ConnectivityHelper::_pickPreferedConfiguration() {
    for (auto &conf : this->_getDefinedConfiguration()) {
        // const auto purpose = conf.purpose();
        const auto type = conf.type();
        const auto name = conf.name();

        auto isIAP = type == QNetworkConfiguration::InternetAccessPoint;
        if (!isIAP) continue;

        auto unauthorizedInterface = name.contains(QStringLiteral(u"npcap"), Qt::CaseInsensitive) ||
                                     name.contains(QStringLiteral(u"virtualbox"), Qt::CaseInsensitive) ||
                                     name.contains(QStringLiteral(u"bluetooth"), Qt::CaseInsensitive) ||
                                     name.contains(QStringLiteral(u"pseudo"), Qt::CaseInsensitive) ||
                                     name.contains(QStringLiteral(u"WAN"), Qt::CaseSensitive);
        if (unauthorizedInterface) continue;

        // define new config
        conf.setConnectTimeout(3000);
        this->_nam->setConfiguration(conf);

        this->_debugNetworkConfig();

        return;
    }
}

void ConnectivityHelper::init() {
    auto initAccessibilityCheck = this->_nam->networkAccessible();
    this->networkChanged(initAccessibilityCheck);
}

ConnectivityHelper::~ConnectivityHelper()  {
    this->_clearUPnPRequester();
}

void ConnectivityHelper::_clearUPnPRequester() {
    if (!this->_upnpThread) return;

    this->_upnpThread->exit();
    this->_upnpThread->wait();
    this->_upnpThread->disconnect();
    delete this->_upnpThread;
    this->_upnpThread = 0;
}

void ConnectivityHelper::_tryNegociateUPnPPort() {
    this->_clearUPnPRequester();

    qDebug() << "Connectivity : Trying to open uPnP port"
             << qUtf8Printable(AppContext::UPNP_DEFAULT_TARGET_PORT)
             << "as"
             << qUtf8Printable(AppContext::UPNP_REQUEST_DESCRIPTION);

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

    this->_upnpThread->start();
}

void ConnectivityHelper::_onUPnPError() {
    qDebug() << "Connectivity : uPnP failed !";
    emit uPnPStateChanged(tr("No"));
    emit remoteAddressStateChanged(tr("<UPnP Failed>"), RPZStatusLabel::State::Error);
}

void ConnectivityHelper::_onUPnPSuccess(const QString &extIP, const QString &protocol, const QString &negociatedPort) {
    auto out = QStringLiteral(u"OK [port: %1]");
    out = out.arg(negociatedPort);

    qDebug() << "Connectivity : uPnP"
             << qUtf8Printable(protocol)
             << qUtf8Printable(out);

    emit remoteAddressStateChanged(extIP);
    emit uPnPStateChanged(out);
}

void ConnectivityHelper::networkChanged(const QNetworkAccessManager::NetworkAccessibility accessible) {
    emit localAddressStateChanged(this->_getWaitingText(), RPZStatusLabel::State::Processing);
    emit remoteAddressStateChanged(this->_getWaitingText(), RPZStatusLabel::State::Processing);
    emit uPnPStateChanged(this->_getWaitingText(), RPZStatusLabel::State::Processing);

    if (!accessible) {
        qDebug() << "Connectivity : Network not accessible, letting default label";
    } else {
        qDebug() << "Connectivity : Network accessible, trying to get IPs and UPnP...";

        this->_getLocalAddress();
        this->_tryNegociateUPnPPort();
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
