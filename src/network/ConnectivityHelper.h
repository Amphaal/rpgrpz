#include <QString>
#include <QLabel>
#include "uPnP/uPnPWrapper.cpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QNetworkSession>


class ConnectivityHelper : public QObject
{
    Q_OBJECT

    public:
        ConnectivityHelper();
        QString getLocalAddress();
        void askExternalAddress();
        void tryNegociateUPnPPort();

        QLabel* extIpLabel;
        QLabel* localIpLabel;
        QLabel* upnpStateLabel;

    private slots:
        void gotReply(QNetworkReply* networkReply);
        void onUPnPInitialized(int errorCode);
        void networkChanged(QNetworkAccessManager::NetworkAccessibility accessible);

    signals:
        void externalAddressReceived(QString extAddress);

    private:
        QNetworkAccessManager* _manager = 0;
        uPnPWrapper* _upnpThread = 0;
        QMetaObject::Connection _upnpInitialized;
        QString _getWaitingText();
        
};
