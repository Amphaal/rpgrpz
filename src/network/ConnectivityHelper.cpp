#include "ConnectivityHelper.h"


ConnectivityHelper::ConnectivityHelper(QObject *parent) : 
        QObject(parent), 
        _nam(new QNetworkAccessManager(this)), 
        _ncm(new QNetworkConfigurationManager(this)) { 
    
    qDebug() << "Connectivity : helper starting !";

    this->_pickPreferedConfiguration();

    QObject::connect(
        this->_ncm, &QNetworkConfigurationManager::configurationChanged, 
        this, &ConnectivityHelper::_mustReInit
    );

    QObject::connect(
        this->_nam, &QNetworkAccessManager::finished, 
        this, &ConnectivityHelper::_onExternalAddressRequestResponse
    );

};

void ConnectivityHelper::_mustReInit(QNetworkConfiguration config) {
    
    auto mustReInit = this->_nam->configuration() == config;

    auto oldPrefConfig = this->_nam->configuration();
    this->_pickPreferedConfiguration();
    auto newPrefConfig = this->_nam->configuration();
    auto mustReInit_2 = oldPrefConfig != newPrefConfig;

    qDebug() << "Connectivity : Detected an alteration in network" << config.name() << ", repicking interface " << this->_nam->configuration().name();
    
    if(mustReInit || mustReInit_2) {
        this->init();
    }
}

void ConnectivityHelper::_pickPreferedConfiguration() {
    
    auto filter = QNetworkConfiguration::StateFlags(QNetworkConfiguration::Defined);
    auto filteredConfs = _ncm->allConfigurations(filter);

    for(auto conf : filteredConfs) {

        auto purpose = conf.purpose();
        auto type = conf.type();
        auto name = conf.name();

        if(!type == QNetworkConfiguration::InternetAccessPoint) continue;

        auto unauthorizedInterface = name.contains("npcap", Qt::CaseInsensitive) ||
                             name.contains("virtualbox", Qt::CaseInsensitive) ||
                             name.contains("bluetooth", Qt::CaseInsensitive) ||
                             name.contains("WAN", Qt::CaseSensitive);
        if(unauthorizedInterface) continue;
        
        //define new config
        conf.setConnectTimeout(3000);
        this->_nam->setConfiguration(conf);

        this->_debugNetworkConfig();

        return; 
    }
}

void ConnectivityHelper::init() {
    auto initAccessibilityCheck = this->_nam->networkAccessible();
    this->networkChanged(initAccessibilityCheck);
}

ConnectivityHelper::~ConnectivityHelper()  {
    this->_clearUPnPRequester();
}

void ConnectivityHelper::_clearUPnPRequester() {
    if(this->_upnpThread) {
        this->_upnpThread->exit();
        this->_upnpThread->wait();
        this->_upnpThread->disconnect();
        delete this->_upnpThread;
        this->_upnpThread = 0;
    }
}

void ConnectivityHelper::_tryNegociateUPnPPort() {
    
    this->_clearUPnPRequester();

    this->_requestedUPnPPort = UPNP_DEFAULT_TARGET_PORT.c_str();
    auto descr = UPNP_REQUEST_DESCRIPTION.c_str();

    qDebug() << "Connectivity : trying to open uPnP port " << this->_requestedUPnPPort.c_str() << " as \"" << descr << "\" ";

    this->_upnpThread = new uPnPRequester(this->_requestedUPnPPort.c_str(), descr, this);

    QObject::connect(
        this->_upnpThread, &uPnPThread::uPnPSuccess, 
        this, &ConnectivityHelper::_onUPnPSuccess
    );

    QObject::connect(
        this->_upnpThread, &uPnPThread::uPnPError, 
        this, &ConnectivityHelper::_onUPnPError
    );

    this->_upnpThread->start();
}

void ConnectivityHelper::_onUPnPError(int errorCode) {
    qWarning() << "Connectivity : uPnP failed !";
    emit uPnPStateChanged("Non");
}

void ConnectivityHelper::_onUPnPSuccess(const char * protocol, const char * negociatedPort) {
    std::string out = "OK [port: ";
    out += negociatedPort;
    out += "] ";

    qDebug() << "Connectivity : uPnP "<< protocol << " " << QString::fromStdString(out);
    emit uPnPStateChanged(out.c_str());

}

void ConnectivityHelper::_askExternalAddress() {

    QUrl targetUrl("http://api.ipify.org?format=json");
    QNetworkRequest request(targetUrl);

    qDebug() << "Connectivity : asking ipify.org for external IP...";

    this->_nam->get(request);
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
    auto activeConf = this->_nam->configuration();

    auto _debug = [&](std::string descr, QNetworkConfiguration &config) {
        qDebug() << "Connectivity :" << QString::fromStdString(descr)
                << ">> name:" << config.name() 
                << ", state:" << config.state() 
                << ", type:" << config.type() 
                << ", bearer:" << config.bearerTypeName();
    };

    _debug("active configuration", activeConf);
}
