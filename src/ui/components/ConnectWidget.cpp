#include "ConnectWidget.h"

ConnectWidget::ConnectWidget(QWidget * parent) : QWidget(parent), 
                                            _nameTarget(new QLineEdit(this)),
                                            _portTarget(new QLineEdit(this)), 
                                            _domainTarget(new QLineEdit(this)),
                                            _connectBtn(new QPushButton(this)) {
                                                    
    QSettings settings;
    settings.beginGroup("ConnectWidget");

    //this
    this->setLayout(new QHBoxLayout);
    this->layout()->setContentsMargins(10, 10, 10, 5);

    //name target
    this->_nameTarget->setPlaceholderText("Nom de joueur");
    this->_domainTarget->setText(settings.value("displayname", "").toString());

    //domain target
    this->_domainTarget->setPlaceholderText("IP ou domaine du serveur");
    this->_domainTarget->setText(settings.value("domain", "localhost").toString());

    //sep
    auto sep = new QLabel(this);
    sep->setText(":");

    //port target
    this->_portTarget->setValidator(new QIntValidator(0, 65535));
    this->_portTarget->setPlaceholderText("Port");
    this->_portTarget->setText(
        settings.value(
            "port", 
            QString::fromStdString(UPNP_DEFAULT_TARGET_PORT)
        ).toString()
    );
    this->_portTarget->setFixedSize(40, 22);

    this->_setConnectBtnState();

    //adding widgets
    this->layout()->addWidget(this->_nameTarget);
    this->layout()->addWidget(this->_domainTarget);
    this->layout()->addWidget(sep);
    this->layout()->addWidget(this->_portTarget);
    this->layout()->addWidget(this->_connectBtn);

    settings.endGroup();
}

void ConnectWidget::_tryConnectToServer() {

    this->setEnabled(false);

    //register default values
    QSettings settings;
    settings.beginGroup("MainWindow");

    auto dt_text = this->_domainTarget->text();
    if(!dt_text.isEmpty()) settings.setValue("domain", dt_text);

    auto pt_text = this->_portTarget->text();
    if(!pt_text.isEmpty()) settings.setValue("port", pt_text);    

    auto nt_text = this->_nameTarget->text();
    if(!nt_text.isEmpty()) settings.setValue("displayname", nt_text);

    settings.endGroup();

    //connect..
    this->_destroyClient();
    this->_cc = new ChatClient(nt_text, dt_text, pt_text);
    
    QObject::connect(
        this->_cc, &ChatClient::connected, 
        this, &ConnectWidget::_onChatClientConnected
    );

    QObject::connect(
        this->_cc, &ChatClient::error, 
        this, &ConnectWidget::_onChatClientError
    );

    this->_cc->tryConnection();
}

void ConnectWidget::_onChatClientError(const std::string errMsg) {
    if(!this->_connected) {
        QMessageBox::information(this, 
            QString("Erreur lors de la connexion"), 
            QString::fromStdString(errMsg), 
            QMessageBox::Ok, QMessageBox::Ok);
    }

    this->_setConnectBtnState(true);
}
void ConnectWidget::_onChatClientConnected() {
    this->_setConnectBtnState(false);
    emit connectionSuccessful(this->_cc);
}

void  ConnectWidget::_destroyClient() {
    if(this->_cc) {
        delete this->_cc;
        this->_cc = 0;
    }
}

void ConnectWidget::_setConnectBtnState(bool readyForConnection) {
    
    this->_connectBtn->setText(readyForConnection ? "Se connecter" : "Se déconnecter");

    QObject::disconnect(this->_connectBtnLink);
    this->_connected = !readyForConnection;

    if(readyForConnection) {
        this->_connectBtnLink = QObject::connect(
            this->_connectBtn, &QPushButton::clicked,
            this, &ConnectWidget::_tryConnectToServer
        );

        this->_domainTarget->setEnabled(true);
        this->_portTarget->setEnabled(true);
        this->_nameTarget->setEnabled(true);

    } else {
        this->_connectBtnLink = QObject::connect(
            this->_connectBtn, &QPushButton::clicked,
            this, &ConnectWidget::_tryDisconnectingFromServer
        );

        this->_domainTarget->setEnabled(false);
        this->_portTarget->setEnabled(false);
        this->_nameTarget->setEnabled(false);
    }

    this->setEnabled(true);

}

void ConnectWidget::_tryDisconnectingFromServer() {
    if(this->_cc) {
        this->_cc->close();
    }
}