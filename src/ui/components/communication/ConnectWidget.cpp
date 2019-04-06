#include "ConnectWidget.h"

ConnectWidget::ConnectWidget(QWidget * parent) : QGroupBox(parent), 
                                            _nameTarget(new QLineEdit(this)),
                                            _portTarget(new QLineEdit(this)), 
                                            _domainTarget(new QLineEdit(this)),
                                            _connectBtn(new QPushButton(this)) {
                                                    
    this->_settings.beginGroup("ConnectWidget");

    //this
    this->setLayout(new QHBoxLayout);
    this->setTitle("Connexion à une session");
    this->setAlignment(Qt::AlignHCenter);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    //name target
    this->_nameTarget->addAction(QIcon(":/icons/app/connectivity/user.png"), QLineEdit::LeadingPosition);
    this->_nameTarget->setPlaceholderText("Nom de joueur");
    this->_nameTarget->setText(this->_settings.value("name", "").toString());


    //domain target
    this->_domainTarget->addAction(QIcon(":/icons/app/connectivity/server.png"), QLineEdit::LeadingPosition);
    this->_domainTarget->setPlaceholderText("IP ou domaine du serveur");
    this->_domainTarget->setText(this->_settings.value("domain", "localhost").toString());


    //sep
    auto sep = new QLabel(this);
    sep->setText(":");

    //port target
    this->_portTarget->addAction(QIcon(":/icons/app/connectivity/port.png"), QLineEdit::LeadingPosition);
    this->_portTarget->setValidator(new QIntValidator(0, 65535));
    this->_portTarget->setPlaceholderText("Port");
    this->_portTarget->setText(
        this->_settings.value(
            "port", 
            QString::fromStdString(UPNP_DEFAULT_TARGET_PORT)
        ).toString()
    );
    this->_portTarget->setMaximumWidth(70);

    //bind to Return Key press...
    auto bb = [&]() {
        this->_connectBtn->click();
    };
    QObject::connect(this->_nameTarget, &QLineEdit::returnPressed, bb);
    QObject::connect(this->_domainTarget, &QLineEdit::returnPressed, bb);
    QObject::connect(this->_portTarget, &QLineEdit::returnPressed, bb);

    this->_setConnectBtnState();

    //adding widgets
    this->layout()->addWidget(this->_nameTarget);
    this->layout()->addWidget(this->_domainTarget);
    this->layout()->addWidget(sep);
    this->layout()->addWidget(this->_portTarget);
    this->layout()->addWidget(this->_connectBtn);

    this->_settings.endGroup();
}

void ConnectWidget::_tryConnectToServer() {

    this->setEnabled(false);

    //register default values
    this->_settings.beginGroup("ConnectWidget");

    const auto dt_text = this->_domainTarget->text();
    if(!dt_text.isEmpty()) this->_settings.setValue("domain", dt_text);

    const auto pt_text = this->_portTarget->text();
    if(!pt_text.isEmpty()) this->_settings.setValue("port", pt_text);

    const auto nt_text = this->_nameTarget->text();
    if(!nt_text.isEmpty()) this->_settings.setValue("name", nt_text);

    this->_settings.endGroup();

    //connect..
    this->_destroyClient();
    this->_cc = new RPZClient(nt_text, dt_text, pt_text);
    emit startingConnection(this->_cc);
    
    QObject::connect(
        this->_cc, &RPZClient::logHistoryReceived, 
        this, &ConnectWidget::_onRPZClientConnected
    );

    QObject::connect(
        this->_cc, &RPZClient::error, 
        this, &ConnectWidget::_onRPZClientError
    );

    this->_cc->start();
}

void ConnectWidget::_onRPZClientError(const std::string &errMsg) {
    if(!this->_connected) {
        QMessageBox::information(this, 
            QString("Erreur lors de la connexion"), 
            QString::fromStdString(errMsg), 
            QMessageBox::Ok, QMessageBox::Ok);
    }

    this->_setConnectBtnState(true);
}
void ConnectWidget::_onRPZClientConnected() {
    this->_setConnectBtnState(false);
    emit connectionSuccessful(this->_cc);
}

void ConnectWidget::_destroyClient() {
    if(this->_cc) {
        this->_cc->exit();
        this->_cc->wait();
        this->_cc->disconnect();
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
            this, &ConnectWidget::_destroyClient
        );

        this->_domainTarget->setEnabled(false);
        this->_portTarget->setEnabled(false);
        this->_nameTarget->setEnabled(false);
    }

    this->setEnabled(true);

}