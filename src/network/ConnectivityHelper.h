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
#include <QThread>
#include <QNetworkConfigurationManager>

#include "src/helpers/_const.cpp"

class ConnectivityHelper : public QObject
{
    Q_OBJECT

    public:
        ConnectivityHelper(QObject *parent = nullptr);
        ~ConnectivityHelper();
        void init();

    signals:
        void localAddressStateChanged(std::string state);
        void remoteAddressStateChanged(std::string state, bool isOn = false);
        void uPnPStateChanged(std::string state);

    private:
        
        QNetworkAccessManager* _nam = 0;
        QNetworkConfigurationManager* _ncm = 0;

        std::string _requestedUPnPPort;
        uPnPRequester* _upnpThread = 0;

        void _debugNetworkConfig();
        std::string _getWaitingText();
        std::string _getErrorText();

        void _getLocalAddress();
        void _askExternalAddress();
        void _tryNegociateUPnPPort();
        void _clearUPnPRequester();

        void _onExternalAddressRequestResponse(QNetworkReply* networkReply);

        void _pickPreferedConfiguration();

        void _onUPnPSuccess(const char * protocol, const char * negociatedPort);
        void _onUPnPError(int errorCode);
        void networkChanged(QNetworkAccessManager::NetworkAccessibility accessible);

        void _mustReInit(QNetworkConfiguration config);

        QList<QNetworkConfiguration> _getDefinedConfiguration();
        
};
