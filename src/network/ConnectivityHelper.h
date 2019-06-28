#pragma once

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

#include "src/ui/others/statusbar/RPZStatusLabel.h"

class ConnectivityHelper : public QObject
{
    Q_OBJECT

    public:
        ConnectivityHelper(QObject *parent = nullptr);
        ~ConnectivityHelper();
        void init();

    signals:
        void localAddressStateChanged(const QString &stateText, int state = 0);
        void remoteAddressStateChanged(const QString &stateText, int state = 0);
        void uPnPStateChanged(const QString &stateText, int state = 0);

    private:
        QNetworkAccessManager* _nam = nullptr;
        QNetworkConfigurationManager* _ncm = nullptr;

        std::string _requestedUPnPPort;
        std::string _requestedDescription;
        uPnPRequester* _upnpThread = nullptr;

        void _debugNetworkConfig();
        QString _getWaitingText();
        QString _getErrorText();

        void _getLocalAddress();
        void _tryNegociateUPnPPort();
        void _clearUPnPRequester();
        void _pickPreferedConfiguration();

        void _onUPnPSuccess(const char * protocol, const char * negociatedPort);
        void _onUPnPError(int errorCode);
        QString _upnp_extIp;
        void _onUPnPExtIpFound(const QString &extIp);
        void networkChanged(const QNetworkAccessManager::NetworkAccessibility accessible);

        void _mustReInit(const QNetworkConfiguration &config);

        QList<QNetworkConfiguration> _getDefinedConfiguration();
        

};
