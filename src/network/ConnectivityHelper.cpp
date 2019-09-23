#include "ConnectivityHelper.h"

ConnectivityHelper::ConnectivityHelper(QObject *parent) : 
        QObject(parent), 
        _nam(new QNetworkAccessManager), 
        _ncm(new QNetworkConfigurationManager) { 

    this->_pickPreferedConfiguration();

    QObject::connect(
        this->_ncm, &QNetworkConfigurationManager::configurationChanged, 
        this, &ConnectivityHelper::_mustReInit
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

        auto isIAP = type == QNetworkConfiguration::InternetAccessPoint;
        if(!isIAP) continue;

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

    qDebug() << "Connectivity : Trying to open uPnP port" << AppContext::UPNP_DEFAULT_TARGET_PORT << "as" << AppContext::UPNP_REQUEST_DESCRIPTION;

    this->_requestedUPnPPort = AppContext::UPNP_DEFAULT_TARGET_PORT.toStdString();
    this->_requestedDescription = AppContext::UPNP_REQUEST_DESCRIPTION.toStdString();

    this->_upnpThread = new uPnPRequester(
        this->_requestedUPnPPort.c_str(), 
        this->_requestedDescription.c_str(), 
        this
    );

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

void ConnectivityHelper::_onUPnPExtIpFound(const QString &extIp) {
    this->_upnp_extIp = extIp;
    emit remoteAddressStateChanged(extIp);
};

void ConnectivityHelper::_onUPnPError(int errorCode) {
    qDebug() << "Connectivity : uPnP failed !";
    emit uPnPStateChanged("Non");
}

void ConnectivityHelper::_onUPnPSuccess(const char * protocol, const char * negociatedPort) {
    QString out = "OK [port: ";
    out += negociatedPort;
    out += "] ";

    qDebug() << "Connectivity : uPnP" << protocol << out.toStdString().c_str();
    emit uPnPStateChanged(out);

}

void ConnectivityHelper::networkChanged(const QNetworkAccessManager::NetworkAccessibility accessible) {
    
    emit localAddressStateChanged(this->_getWaitingText(), SLState::SL_Processing);
    emit remoteAddressStateChanged(this->_getWaitingText(), SLState::SL_Processing);
    emit uPnPStateChanged(this->_getWaitingText(), SLState::SL_Processing);

    if(!accessible) {

        qDebug() << "Connectivity : Network not accessible, letting default label";

    } else {

        qDebug() << "Connectivity : Network accessible, trying to get IPs and UPnP...";

        this->_getLocalAddress();
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
        qDebug() << "Connectivity : Local IP not found !";
        emit localAddressStateChanged(this->_getErrorText(), SLState::SL_Error);
    } else {
        qDebug() << "Connectivity : Local IP" << rtrn;
        emit localAddressStateChanged(rtrn);
    }
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

void ConnectivityHelper::_debugNetworkConfig() {
    
    auto _debug = [&](const QString &descr, const QNetworkConfiguration &config) {
        QString model = R"(Connectivity : %1 >> %2 [state:%3, type:%4, bearer:%5])";
        model = model.arg(descr)
                     .arg(config.name())
                     .arg(config.state())
                     .arg(config.type())
                     .arg(config.bearerTypeName());
        qDebug() << model.toStdString().c_str();
    };

    for (auto &config : this->_getDefinedConfiguration()) {
        _debug("EXISTING", config);
    }

    //active...
    auto activeConf = this->_nam->configuration();
    _debug("ACTIVE", activeConf);
}
