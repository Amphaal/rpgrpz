#include "ConnectWidget.h"

ConnectWidget::ConnectWidget(QWidget * parent) : QGroupBox(parent), 
                                            _nameTarget(new QLineEdit(this)),
                                            _portTarget(new QLineEdit(this)), 
                                            _domainTarget(new QLineEdit(this)),
                                            _connectBtn(new QPushButton(this)) {
                                                    
    AppContext::settings()->beginGroup("ConnectWidget");

    //this
    this->setLayout(new QHBoxLayout);
    this->setTitle("Connexion à une session");
    this->setAlignment(Qt::AlignHCenter);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    //name target
    this->_nameTarget->addAction(QIcon(":/icons/app/connectivity/user.png"), QLineEdit::LeadingPosition);
    this->_nameTarget->setPlaceholderText("Nom de joueur");
    this->_nameTarget->setText(AppContext::settings()->value("name", "").toString());


    //domain target
    this->_domainTarget->addAction(QIcon(":/icons/app/connectivity/server.png"), QLineEdit::LeadingPosition);
    this->_domainTarget->setPlaceholderText("IP ou domaine du serveur");
    this->_domainTarget->setText(AppContext::settings()->value("domain", "localhost").toString());

    //sep
    auto sep = new QLabel(this);
    sep->setText(":");

    //port target
    this->_portTarget->addAction(QIcon(":/icons/app/connectivity/port.png"), QLineEdit::LeadingPosition);
    this->_portTarget->setValidator(new QIntValidator(0, 65535));
    this->_portTarget->setPlaceholderText("Port");
    this->_portTarget->setText(
        AppContext::settings()->value(
            "port", 
            AppContext::UPNP_DEFAULT_TARGET_PORT
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

    //default ui state
    this->_changeState(this->_state);

    //adding widgets
    this->layout()->addWidget(this->_nameTarget);
    this->layout()->addWidget(this->_domainTarget);
    this->layout()->addWidget(sep);
    this->layout()->addWidget(this->_portTarget);
    this->layout()->addWidget(this->_connectBtn);

    AppContext::settings()->endGroup();
}

void ConnectWidget::_saveValuesAsSettings() {
    
    //register default values
    AppContext::settings()->beginGroup("ConnectWidget");

    const auto dt_text = this->_domainTarget->text();
    if(!dt_text.isEmpty()) AppContext::settings()->setValue("domain", dt_text);

    const auto pt_text = this->_portTarget->text();
    if(!pt_text.isEmpty()) AppContext::settings()->setValue("port", pt_text);

    const auto nt_text = this->_nameTarget->text();
    if(!nt_text.isEmpty()) AppContext::settings()->setValue("name", nt_text);

    AppContext::settings()->endGroup();
}

void ConnectWidget::_tryConnectToServer() {

    this->_saveValuesAsSettings();

    //new connection..
    this->_destroyClient();
    this->_cc = new RPZClient(
        this, 
        this->_nameTarget->text(), 
        this->_domainTarget->text(), 
        this->_portTarget->text()
    );

    emit startingConnection(this->_cc);
    
    QObject::connect(
        this->_cc, &RPZClient::receivedLogHistory, 
        this, &ConnectWidget::_onRPZClientConnecting
    );

    QObject::connect(
        this->_cc, &RPZClient::error, 
        this, &ConnectWidget::_onRPZClientError
    );

    this->_changeState(State::Connecting);
    this->_cc->run();
}

void ConnectWidget::_onRPZClientError(const QString &errMsg) {
    
    if(this->_state = State::Connecting) {
        QMessageBox::information(this, 
            QString("Erreur lors de la connexion"), 
            errMsg, 
            QMessageBox::Ok, QMessageBox::Ok);
    }

    this->_changeState(State::NotConnected);

}
void ConnectWidget::_onRPZClientConnecting() {
    this->_changeState(State::Connected);
    emit connectionSuccessful(this->_cc);
}

void ConnectWidget::_destroyClient() {
    if(this->_cc) {
        delete this->_cc;
        this->_cc = 0;
    }

    this->_changeState(State::NotConnected);
}

void ConnectWidget::_changeState(ConnectWidget::State newState) {
    
    //btn text
    QString btnText; 
    switch(newState) {
        case ConnectWidget::State::NotConnected:
            btnText = "Se connecter";
            break;
        case ConnectWidget::State::Connecting:
            btnText = "Annuler";
            break;
        case ConnectWidget::State::Connected:
            btnText = "Se déconnecter";
            break;
    }
    this->_connectBtn->setText(btnText);

    //btn event
    QObject::disconnect(this->_connectBtnLink);
    switch(newState) {
        case ConnectWidget::State::NotConnected:
            this->_connectBtnLink = QObject::connect(
                this->_connectBtn, &QPushButton::clicked,
                this, &ConnectWidget::_tryConnectToServer
            );
            break;
        case ConnectWidget::State::Connecting:
        case ConnectWidget::State::Connected:
            this->_connectBtnLink = QObject::connect(
                this->_connectBtn, &QPushButton::clicked,
                this, &ConnectWidget::_destroyClient
            );
            break;
    }

    //inputs state
    switch(newState) {
        case ConnectWidget::State::NotConnected:
            this->_domainTarget->setEnabled(true);
            this->_portTarget->setEnabled(true);
            this->_nameTarget->setEnabled(true);
            break;
        case ConnectWidget::State::Connecting:
        case ConnectWidget::State::Connected:
            this->_domainTarget->setEnabled(false);
            this->_portTarget->setEnabled(false);
            this->_nameTarget->setEnabled(false);
            break;
    }

    //define state
    this->_state = newState;

}