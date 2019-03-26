#include "ConnectivityHelper.h"

ConnectivityHelper::ConnectivityHelper(QObject *parent) : QObject(parent) {

    this->_manager = new QNetworkAccessManager(this);

    QObject::connect(this->_manager, &QNetworkAccessManager::finished, 
                    this, &ConnectivityHelper::gotReply);

    this->networkChanged(
        this->_manager->networkAccessible()
    );

    QObject::connect(this->_manager, &QNetworkAccessManager::networkAccessibleChanged, 
                    this, &ConnectivityHelper::networkChanged);

};

ConnectivityHelper::~ConnectivityHelper()  {
    if(this->_upnpThread) { 
        delete this->_upnpThread;
        QObject::disconnect(this->_upnpInitialized);
    }
}

void ConnectivityHelper::tryNegociateUPnPPort() {
    
    if(this->_upnpThread) { 
        delete this->_upnpThread;
        QObject::disconnect(this->_upnpInitialized);
    }

    this->_upnpThread = new uPnPWrapper("31137", "RPGRPZ");

    this->_upnpInitialized = QObject::connect(this->_upnpThread, &uPnPThread::initialized, 
                                              this, &ConnectivityHelper::onUPnPInitialized);

    this->_upnpThread->start();
}

void ConnectivityHelper::onUPnPInitialized(int errorCode, const char * negociatedPort) {
    if(errorCode != 0) {
        this->upnpStateLabel->setText("Non");
    } else {
        std::string out = "OK, port: ";
        out += negociatedPort;
        this->upnpStateLabel->setText(QString::fromStdString(out));
    }
}

void ConnectivityHelper::askExternalAddress() {

    this->extIpLabel->setText(this->_getWaitingText());

    QUrl targetUrl("http://api.ipify.org?format=json");
    QNetworkRequest request(targetUrl);
    this->_manager->get(request);
};

void ConnectivityHelper::networkChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    
    if(!this->extIpLabel) this->extIpLabel = new QLabel(this->_getWaitingText());
    if(!this->localIpLabel) this->localIpLabel = new QLabel(this->_getWaitingText());
    if(!this->upnpStateLabel) this->upnpStateLabel = new QLabel(this->_getWaitingText());
    
    if(!accessible) {
        this->extIpLabel->setText(this->_getWaitingText());
        this->localIpLabel->setText(this->_getWaitingText());
        this->upnpStateLabel->setText(this->_getWaitingText());
    } else {
        this->getLocalAddress();
        this->askExternalAddress();
        this->tryNegociateUPnPPort();
    }
};

QString ConnectivityHelper::getLocalAddress() {
    
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
};

void ConnectivityHelper::gotReply(QNetworkReply* networkReply)
{
    networkReply->deleteLater();
    auto ip = QJsonDocument::fromJson(networkReply->readAll()).object().value("ip").toString();
    
    this->extIpLabel->setText(ip);
    
    emit externalAddressReceived(ip);
};

QString ConnectivityHelper::_getWaitingText() {
    return "<Recherche...>";
};