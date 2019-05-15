#pragma once

#include <QString>
#include <QLabel>
#include "uPnP/uPnPRequester.cpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>

#include "src/helpers/_const.hpp"

#include "src/ui/others/statusbar/RPZStatusLabel.hpp"

class ConnectivityHelper : public QObject
{
    Q_OBJECT

    public:
        ConnectivityHelper(QObject *parent = nullptr);
        ~ConnectivityHelper();
        void init();

    signals:
        void localAddressStateChanged(const std::string &stateText, int state = 0);
        void remoteAddressStateChanged(const std::string &stateText, int state = 0);
        void uPnPStateChanged(const std::string &stateText, int state = 0);

    private:
        QNetworkAccessManager* _nam = nullptr;
        QNetworkConfigurationManager* _ncm = nullptr;

        std::string _requestedUPnPPort;
        uPnPRequester* _upnpThread = nullptr;

        void _debugNetworkConfig();
        std::string _getWaitingText();
        std::string _getErrorText();

        void _getLocalAddress();
        void _tryNegociateUPnPPort();
        void _clearUPnPRequester();
        void _pickPreferedConfiguration();

        void _onUPnPSuccess(const char * protocol, const char * negociatedPort);
        void _onUPnPError(int errorCode);
        std::string _upnp_extIp;
        void _onUPnPExtIpFound(const std::string &extIp);
        void networkChanged(const QNetworkAccessManager::NetworkAccessibility accessible);

        void _mustReInit(const QNetworkConfiguration &config);

        QList<QNetworkConfiguration> _getDefinedConfiguration();
        

};
