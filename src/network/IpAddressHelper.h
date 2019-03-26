#include <QJsonDocument>
#include <QJsonObject>
#include <QHostAddress>
#include <QNetworkReply>
#include <QString>
#include <QNetworkInterface>
#include <QLabel>
#include <QNetworkSession>

class IpAddressHelper : public QObject
{
    Q_OBJECT

    public:
        IpAddressHelper();
        QString getLocalAddress();
        void askExternalAddress();

        QLabel* extIpLabel;
        QLabel* localIpLabel;

    private slots:
        void gotReply(QNetworkReply* networkReply);
        void networkChanged(QNetworkAccessManager::NetworkAccessibility accessible);

    signals:
        void externalAddressReceived(QString extAddress);

    private:
        QNetworkAccessManager* _manager;
        QString _getWaitingText();
};
