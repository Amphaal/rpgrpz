#include "ConnectivityHelper.h"


ConnectivityHelper::ConnectivityHelper(QObject *parent) : QObject(parent) { 
    
    qDebug() << "Connectivity : helper starting !";

    this->_manager = new QNetworkAccessManager(this);

    QObject::connect(this->_manager, &QNetworkAccessManager::finished, 
                    this, &ConnectivityHelper::_onExternalAddressRequestResponse);

    QObject::connect(this->_manager, &QNetworkAccessManager::networkAccessibleChanged, 
                    this, &ConnectivityHelper::networkChanged);

    this->_debugNetworkConfig();

};

void ConnectivityHelper::init() {
    auto initAccessibilityCheck = this->_manager->networkAccessible();
    this->networkChanged(initAccessibilityCheck);
}

ConnectivityHelper::~ConnectivityHelper()  {
    if(this->_upnpThread) { 
        delete this->_upnpThread;
        QObject::disconnect(this->_upnpInitialized);
    }
}

void ConnectivityHelper::_tryNegociateUPnPPort() {
    
    if(this->_upnpThread) { 
        delete this->_upnpThread;
        QObject::disconnect(this->_upnpInitialized);
    }

    auto port = UPNP_DEFAULT_TARGET_PORT.c_str();
    auto descr = UPNP_REQUEST_DESCRIPTION.c_str();

    qDebug() << "Connectivity : trying to open uPnP port " << port << " as \"" << descr << "\" ";

    this->_upnpThread = new uPnPRequester(port, descr, this);

    this->_upnpInitialized = QObject::connect(this->_upnpThread, &uPnPThread::uPnPDone, 
                                              this, &ConnectivityHelper::onUPnPDone);

    this->_upnpThread->start();
}

void ConnectivityHelper::onUPnPDone(int errorCode, const char * negociatedPort) {
    if(errorCode != 0) {
        qWarning() << "Connectivity : uPnP failed !";
        emit uPnPStateChanged("Non");
    } else {
        std::string out = "OK [port: ";
        out += negociatedPort;
        out += "] ";

        qDebug() << "Connectivity : uPnP " << QString::fromStdString(out);
        emit uPnPStateChanged(out.c_str());
    }
}

void ConnectivityHelper::_askExternalAddress() {

    QUrl targetUrl("http://api.ipify.org?format=json");
    QNetworkRequest request(targetUrl);

    qDebug() << "Connectivity : asking ipify.org for external IP...";

    this->_manager->get(request);
};

void ConnectivityHelper::networkChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    
    emit localAddressStateChanged(this->_getWaitingText());
    emit remoteAddressStateChanged(this->_getWaitingText());
    emit uPnPStateChanged(this->_getWaitingText());

    if(!accessible) {

        qDebug() << "Connectivity : network not accessible, letting default label";

    } else {

        qDebug() << "Connectivity : network accessible, trying to get IPs and UPnP...";

        this->_getLocalAddress();
        this->_askExternalAddress();
        this->_tryNegociateUPnPPort();
    }
};

void ConnectivityHelper::_getLocalAddress() {
    
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
        emit localAddressStateChanged(this->_getErrorText());
    } else {
        qDebug() << "Connectivity : local ip" << rtrn;
        emit localAddressStateChanged(rtrn.toStdString());
    }
};

void ConnectivityHelper::_onExternalAddressRequestResponse(QNetworkReply* networkReply)
{   
    auto err = networkReply->error();
    networkReply->deleteLater();
    
    if(err) {
        qWarning() << "Connectivity : ipify.org cannot be reached !";
        emit remoteAddressStateChanged(this->_getErrorText());
        return;
    }
    auto ip = QJsonDocument::fromJson(networkReply->readAll()).object().value("ip").toString();
    
    qDebug() << "Connectivity : ipify.org responded our external IP is " << ip;

    emit remoteAddressStateChanged(ip.toStdString().c_str(), true);
};

///
///
///

std::string ConnectivityHelper::_getWaitingText() {
    return "<Recherche...>";
};

std::string ConnectivityHelper::_getErrorText() {
    return "<Erreur>";
};

void ConnectivityHelper::_debugNetworkConfig() {
    
    //active...
    auto activeConf = this->_manager->activeConfiguration();

    auto _debug = [&](std::string descr, QNetworkConfiguration &config) {
        qDebug() << "Connectivity :" << QString::fromStdString(descr)
                << ">> name:" << config.name() 
                << ", state:" << config.state() 
                << ", type:" << config.type() 
                << ", bearer:" << config.bearerTypeName();
    };

    _debug("active configuration", activeConf);
}
