#include <QString>
#include <QLabel>
#include "uPnP/uPnPWrapper.cpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QNetworkSession>

#include "src/helpers/_const.cpp"

class ConnectivityHelper : public QObject
{
    Q_OBJECT

    public:
        ConnectivityHelper(QObject *parent = nullptr);
        ~ConnectivityHelper();
        QString getLocalAddress();
        void askExternalAddress();
        void tryNegociateUPnPPort();

        QLabel* extIpLabel = 0;
        QLabel* localIpLabel = 0;
        QLabel* upnpStateLabel = 0;

    private slots:
        void gotReply(QNetworkReply* networkReply);
        void onUPnPInitialized(int errorCode, const char * negociatedPort);
        void networkChanged(QNetworkAccessManager::NetworkAccessibility accessible);

    signals:
        void externalAddressReceived(QString extAddress);

    private:
        QNetworkAccessManager* _manager = 0;
        uPnPWrapper* _upnpThread = 0;
        QMetaObject::Connection _upnpInitialized;
        QString _getWaitingText();
        QString _getErrorText();
        
};
