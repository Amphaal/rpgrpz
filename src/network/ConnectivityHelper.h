#pragma once

#include <QDebug>

#include <QString>
#include <QLabel>
#include "uPnP/uPnPRequester.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QNetworkSession>
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>

#include "src/helpers/_appContext.h"

#include "src/ui/statusBar/RPZStatusLabel.h"

class ConnectivityHelper : public QObject
{
    Q_OBJECT

    public:
        ConnectivityHelper(QObject *parent = nullptr);
        ~ConnectivityHelper();
        void init();

    signals:
        void localAddressStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);
        void remoteAddressStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);
        void uPnPStateChanged(const QString &stateText, RPZStatusLabel::State state = RPZStatusLabel::State::Finished);

    private:
        QNetworkAccessManager* _nam = nullptr;
        QNetworkConfigurationManager* _ncm = nullptr;
        uPnPRequester* _upnpThread = nullptr;

        void _debugNetworkConfig();
        QString _getWaitingText();
        QString _getErrorText();

        void _getLocalAddress();
        void _tryNegociateUPnPPort();
        void _clearUPnPRequester();
        void _pickPreferedConfiguration();

        void _onUPnPSuccess(const QString &protocol, const QString &negociatedPort);
        void _onUPnPError(int errorCode);
        QString _upnp_extIp;
        void _onUPnPExtIpFound(const QString &extIp);
        void networkChanged(const QNetworkAccessManager::NetworkAccessibility accessible);

        void _mustReInit(const QNetworkConfiguration &config);

        QList<QNetworkConfiguration> _getDefinedConfiguration();
        

};
