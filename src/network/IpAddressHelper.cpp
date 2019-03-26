#include "IpAddressHelper.h"

IpAddressHelper::IpAddressHelper()
{
    this->_manager = new QNetworkAccessManager(this);

    QObject::connect(this->_manager, &QNetworkAccessManager::finished, 
                    this, &IpAddressHelper::gotReply);

    QObject::connect(this->_manager, &QNetworkAccessManager::networkAccessibleChanged, 
                    this, &IpAddressHelper::networkChanged);

    this->extIpLabel = new QLabel(this->_getWaitingText());
    this->localIpLabel = new QLabel(this->_getWaitingText());

    if(this->_manager->networkAccessible()) {
        this->getLocalAddress();
        this->askExternalAddress();
    }
}

void IpAddressHelper::askExternalAddress() {

    this->extIpLabel->setText(this->_getWaitingText());

    QUrl targetUrl("http://api.ipify.org?format=json");
    QNetworkRequest request(targetUrl);
    this->_manager->get(request);
}

void IpAddressHelper::networkChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    if(!accessible) {
        this->extIpLabel->setText(this->_getWaitingText());
        this->localIpLabel->setText(this->_getWaitingText());
    } else {
        this->getLocalAddress();
        this->askExternalAddress();
    }
}

QString IpAddressHelper::getLocalAddress() {
    
    QHostAddress localhost(QHostAddress::LocalHost);
    QString rtrn;

    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
            rtrn = address.toString();
            break;
        }
    }

    this->localIpLabel->setText(rtrn.isNull() ? "<Erreur>" : rtrn);

    return rtrn;
}

void IpAddressHelper::gotReply(QNetworkReply* networkReply)
{
    networkReply->deleteLater();
    auto ip = QJsonDocument::fromJson(networkReply->readAll()).object().value("ip").toString();
    
    this->extIpLabel->setText(ip);
    
    emit externalAddressReceived(ip);
}

QString IpAddressHelper::_getWaitingText() {
    return "<Recherche...>";
}