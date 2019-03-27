#include "ConnectivityHelper.h"

void ConnectivityHelper::_debugNetworkConfig() {
    
    //active...
    auto activeConf = this->_manager->activeConfiguration();

    qDebug() << "Connectivity : active configuration"
             << ">> name:" << activeConf.name() 
             << ", state:" << activeConf.state() 
             << ", type:" << activeConf.type() 
             << ", bearer:" << activeConf.bearerTypeName();

    //default...
    QNetworkConfigurationManager defManager;
    auto defaultConf = defManager.defaultConfiguration();

    qDebug() << "Connectivity : default configuration"
            << ">> name:" << defaultConf.name() 
            << ", state:" << defaultConf.state() 
            << ", type:" << defaultConf.type() 
            << ", bearer:" << defaultConf.bearerTypeName();
}

ConnectivityHelper::ConnectivityHelper(QObject *parent) : QObject(parent) {
    
    qDebug() << "Connectivity : helper starting !";

    this->_manager = new QNetworkAccessManager(this);

    QObject::connect(this->_manager, &QNetworkAccessManager::finished, 
                    this, &ConnectivityHelper::gotReply);

    auto initAccessibilityCheck = this->_manager->networkAccessible();
    this->networkChanged(initAccessibilityCheck);

    QObject::connect(this->_manager, &QNetworkAccessManager::networkAccessibleChanged, 
                    this, &ConnectivityHelper::networkChanged);

    this->_debugNetworkConfig();
    
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

    auto port = UPNP_DEFAULT_TARGET_PORT.c_str();
    auto descr = UPNP_REQUEST_DESCRIPTION.c_str();

    qDebug() << "Connectivity : trying to open uPnP port " << port << " as \"" << descr << "\" ";

    this->_upnpThread = new uPnPWrapper(port, descr);

    this->_upnpInitialized = QObject::connect(this->_upnpThread, &uPnPThread::initialized, 
                                              this, &ConnectivityHelper::onUPnPInitialized);

    this->_upnpThread->start();
}

void ConnectivityHelper::onUPnPInitialized(int errorCode, const char * negociatedPort) {
    if(errorCode != 0) {
        qWarning() << "Connectivity : uPnP failed !";
        this->upnpStateLabel->setText("Non");
    } else {
        std::string out = "OK [port: ";
        out += negociatedPort;
        out += "] ";

        qDebug() << "Connectivity : uPnP " << QString::fromStdString(out);
        this->upnpStateLabel->setText(QString::fromStdString(out));
    }
}

void ConnectivityHelper::askExternalAddress() {

    QUrl targetUrl("http://api.ipify.org?format=json");
    QNetworkRequest request(targetUrl);

    qDebug() << "Connectivity : asking ipify.org for external IP...";

    this->_manager->get(request);
};

void ConnectivityHelper::networkChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    
    if(!this->extIpLabel) {
        qDebug() << "Connectivity : initial default label passing...";
        
        this->extIpLabel = new QLabel(this->_getWaitingText());
        this->localIpLabel = new QLabel(this->_getWaitingText());
        this->upnpStateLabel = new QLabel(this->_getWaitingText());

    } else {

        this->extIpLabel->setText(this->_getWaitingText());
        this->localIpLabel->setText(this->_getWaitingText());
        this->upnpStateLabel->setText(this->_getWaitingText());

    }

    if(!accessible) {

        qDebug() << "Connectivity : network not accessible, letting default label";

    } else {

        qDebug() << "Connectivity : network accessible, trying to get IPs and UPnP...";

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

    if(rtrn.isNull()) {
        qWarning() << "Connectivity : local ip not found !";
        this->localIpLabel->setText(this->_getErrorText());
    } else {
        qDebug() << "Connectivity : local ip" << rtrn;
        this->localIpLabel->setText(rtrn);
    }
    
    return rtrn;
};

void ConnectivityHelper::gotReply(QNetworkReply* networkReply)
{   
    auto err = networkReply->error();
    networkReply->deleteLater();
    
    if(err) {
        qWarning() << "Connectivity : ipify.org cannot be reached !";
        this->extIpLabel->setText(this->_getErrorText());
        return;
    }
    auto ip = QJsonDocument::fromJson(networkReply->readAll()).object().value("ip").toString();
    
    qDebug() << "Connectivity : ipify.org responded our external IP is " << ip;

    this->extIpLabel->setText(ip);
    
    emit externalAddressReceived(ip);
};

///
///
///

QString ConnectivityHelper::_getWaitingText() {
    return "<Recherche...>";
};

QString ConnectivityHelper::_getErrorText() {
    return "<Erreur>";
};