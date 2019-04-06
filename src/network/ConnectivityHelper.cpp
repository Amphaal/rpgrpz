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

void ConnectivityHelper::_mustReInit(const QNetworkConfiguration &config) {
    
    auto mustReInit = this->_nam->configuration() == config;

    const auto oldPrefConfig = this->_nam->configuration();
    this->_pickPreferedConfiguration();
    const auto newPrefConfig = this->_nam->configuration();
    auto mustReInit_2 = oldPrefConfig != newPrefConfig;

    qDebug() << "Connectivity : Detected an alteration in network" << config.name() << ", repicking interface " << this->_nam->configuration().name();
    
    if(mustReInit || mustReInit_2) {
        this->init();
    }
}

QList<QNetworkConfiguration> ConnectivityHelper::_getDefinedConfiguration() {
    auto filter = QNetworkConfiguration::StateFlags(QNetworkConfiguration::Defined);
    auto filteredConfs = this->_ncm->allConfigurations(filter);
    return filteredConfs;
}

void ConnectivityHelper::_pickPreferedConfiguration() {
    
    for(auto &conf : this->_getDefinedConfiguration()) {

        const auto purpose = conf.purpose();
        const auto type = conf.type();
        const auto name = conf.name();

        if(!type == QNetworkConfiguration::InternetAccessPoint) continue;

        auto unauthorizedInterface = name.contains("npcap", Qt::CaseInsensitive) ||
                             name.contains("virtualbox", Qt::CaseInsensitive) ||
                             name.contains("bluetooth", Qt::CaseInsensitive) ||
                             name.contains("pseudo", Qt::CaseInsensitive) ||
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
    
    this->_upnp_extIp = "";
    
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
    const auto descr = UPNP_REQUEST_DESCRIPTION.c_str();

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

    QObject::connect(
        this->_upnpThread, &uPnPThread::uPnPExtIpFound, 
        this, &ConnectivityHelper::_onUPnPExtIpFound
    );

    this->_upnpThread->start();
}

void ConnectivityHelper::_onUPnPExtIpFound(const std::string &extIp) {
    this->_upnp_extIp = extIp;
    emit remoteAddressStateChanged(extIp, true);
};

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

void ConnectivityHelper::networkChanged(const QNetworkAccessManager::NetworkAccessibility accessible) {
    
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

    for(auto &address : QNetworkInterface::allAddresses()) {
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

void ConnectivityHelper::_onExternalAddressRequestResponse(QNetworkReply* networkReply) {   
    
    //prepare
    auto err = networkReply->error();
    networkReply->deleteLater();

    //if upnp found it already, do nothing
    if(this->_upnp_extIp.length() > 0) return;
    
    //if network error
    if(err) {
        qWarning() << "Connectivity : ipify.org cannot be reached !";
        emit remoteAddressStateChanged(this->_getErrorText());
        return;
    }

    //else get the ip from the response
    const auto ip = QJsonDocument::fromJson(networkReply->readAll()).object().value("ip").toString();
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
    
    auto _debug = [&](const std::string &descr, const QNetworkConfiguration &config) {
        qDebug() << "Connectivity :" << QString::fromStdString(descr)
                << ">> name:" << config.name() 
                << ", state:" << config.state() 
                << ", type:" << config.type() 
                << ", bearer:" << config.bearerTypeName();
    };

    for (auto &config : this->_getDefinedConfiguration()) {
        _debug("defined configuration", config);
    }

    //active...
    auto activeConf = this->_nam->configuration();
    _debug("active configuration", activeConf);
}
