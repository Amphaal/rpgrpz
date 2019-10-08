#include "ConnectWidget.h"

ConnectWidget::ConnectWidget(MapHint* hintToControlStateOf) : QWidget(nullptr), 
    _toControlStateOf(hintToControlStateOf),
    _nameTarget(new QLineEdit),
    _domainTarget(new QLineEdit),
    _connectBtn(new QPushButton),
    _characterSheetTarget(new QComboBox) {
                                                    
    AppContext::settings()->beginGroup("ConnectWidget");

    //this
    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    auto mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);
    this->layout()->setMargin(0);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    //name target
    this->_nameTarget->addAction(QIcon(":/icons/app/connectivity/user.png"), QLineEdit::LeadingPosition);
    this->_nameTarget->setPlaceholderText("Nom de joueur");
    this->_nameTarget->setToolTip("Nom de joueur");
    this->_nameTarget->setText(AppContext::settings()->value("name", "").toString());

    //domain target
    this->_domainTarget->addAction(QIcon(":/icons/app/connectivity/server.png"), QLineEdit::LeadingPosition);
    this->_domainTarget->setPlaceholderText("IP ou domaine du serveur");
    this->_domainTarget->setPlaceholderText("IP ou domaine du serveur");
    this->_domainTarget->setText(AppContext::settings()->value("domain", "localhost").toString());

    //character sheet target
    this->_characterSheetTarget->setToolTip("Personnage à incarner");
    this->_fillCharacterSheetCombo();
    QObject::connect(
        CharactersDatabase::get(), &CharactersDatabase::databaseChanged,
        this, &ConnectWidget::_fillCharacterSheetCombo
    );

    //bind to Return Key press...
    auto bb = [&]() {
        this->_connectBtn->click();
    };
    QObject::connect(this->_nameTarget, &QLineEdit::returnPressed, bb);
    QObject::connect(this->_domainTarget, &QLineEdit::returnPressed, bb);

    //default ui state
    this->_changeState(this->_state);
    QObject::connect(
        this->_connectBtn, &QPushButton::clicked,
        this, &ConnectWidget::_onConnectButtonPressed
    );

    //adding widgets
    auto layoutSub = new QHBoxLayout;
    layoutSub->addWidget(this->_nameTarget);
    layoutSub->addWidget(this->_domainTarget);
    layoutSub->addWidget(this->_connectBtn);

    mainLayout->addWidget(this->_characterSheetTarget);
    mainLayout->addLayout(layoutSub);

    AppContext::settings()->endGroup();
}

void ConnectWidget::_onConnectButtonPressed() {
    switch(this->_state) {
        case ConnectWidget::State::NotConnected:
            this->_tryConnectToServer();
            break;
        case ConnectWidget::State::Connecting:
        case ConnectWidget::State::Connected:
            this->_destroyClient();
            break;
    }
}

void ConnectWidget::_saveValuesAsSettings() {
    
    //register default values
    AppContext::settings()->beginGroup("ConnectWidget");

    const auto dt_text = this->_domainTarget->text();
    if(!dt_text.isEmpty()) AppContext::settings()->setValue("domain", dt_text);

    const auto nt_text = this->_nameTarget->text();
    if(!nt_text.isEmpty()) AppContext::settings()->setValue("name", nt_text);

    AppContext::settings()->endGroup();
}

void ConnectWidget::_tryConnectToServer() {

    //ask for save the map as it may be erased
    MapHint::mayWantToSavePendingState(this, _toControlStateOf);

    this->_saveValuesAsSettings();

    //new connection..
    this->_destroyClient();
    
    auto selectedCharacter = CharactersDatabase::get()->character(
        this->_getSelectedCharacterId()
    );
    
    this->_cc = new RPZClient(
        this->_domainTarget->text(),
        this->_nameTarget->text(), 
        selectedCharacter
    );

    //create a separate thread to run the client into
    auto clientThread = new QThread;
    clientThread->setObjectName("RPZClient Thread");
    this->_cc->moveToThread(clientThread);
    
    //events...
    QObject::connect(
        clientThread, &QThread::started, 
        this->_cc, &RPZClient::run
    );

    QObject::connect(
        this->_cc, &RPZClient::closed, 
        clientThread, &QThread::quit
    );

    QObject::connect(
        clientThread, &QThread::finished,
        [=]() {
            this->_cc = nullptr;
        }
    );

    QObject::connect(
        clientThread, &QThread::finished,  
        this->_cc, &QObject::deleteLater
    );

    QObject::connect(
        clientThread, &QThread::finished, 
        clientThread, &QObject::deleteLater
    );

    QObject::connect(
        this->_cc, &RPZClient::receivedLogHistory, 
        this, &ConnectWidget::_onRPZClientConnecting
    );

    QObject::connect(
        this->_cc, &RPZClient::connectionStatus, 
        this, &ConnectWidget::_onRPZClientStatus
    );

    this->_changeState(State::Connecting);

    //start
    emit startingConnection(this->_cc);
    clientThread->start();

}

void ConnectWidget::_onRPZClientStatus(const QString &statusMsg, bool isError) {
    if(!isError) return;

    if(this->_state == State::Connecting) {
        QMessageBox::information(this, 
            QString("Erreur lors de la connexion"), 
            statusMsg, 
            QMessageBox::Ok, QMessageBox::Ok);
    }

    this->_changeState(State::NotConnected);

}
void ConnectWidget::_onRPZClientConnecting() {
    this->_changeState(State::Connected);
}

void ConnectWidget::_destroyClient() {
    if(this->_cc) {
        this->_cc->thread()->quit();
        this->_cc->thread()->wait();
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
            btnText = QString("Annuler (Connexion à %1...)").arg(this->_domainTarget->text());
            break;
        case ConnectWidget::State::Connected:
            btnText = QString("Se déconnecter de [%1]").arg(this->_domainTarget->text());
            break;
    }
    this->_connectBtn->setText(btnText);

    //inputs state
    auto mustEnableWidgets = newState == ConnectWidget::State::NotConnected;
    
    this->_domainTarget->setEnabled(mustEnableWidgets);
    this->_domainTarget->setVisible(mustEnableWidgets);

    this->_nameTarget->setEnabled(mustEnableWidgets);
    this->_nameTarget->setVisible(mustEnableWidgets);

    this->_characterSheetTarget->setEnabled(mustEnableWidgets);
    this->_characterSheetTarget->setVisible(mustEnableWidgets);

    //define state
    this->_state = newState;

}


snowflake_uid ConnectWidget::_getSelectedCharacterId() {
    return (snowflake_uid)this->_characterSheetTarget->currentData().toULongLong();
}

void ConnectWidget::_fillCharacterSheetCombo() {
    
    auto previouslySelectedCharacterId = this->_getSelectedCharacterId(); //get previous selection
    this->_characterSheetTarget->clear(); //clear content

    //for each character in db
    for(auto &character : CharactersDatabase::get()->characters()) {
        
        auto id = character.id();
        auto toBeInsertedIndex = this->_characterSheetTarget->count();

        this->_characterSheetTarget->addItem(QIcon(":/icons/app/connectivity/cloak.png"), character.toString(), id);

        if(previouslySelectedCharacterId == id) {
            this->_characterSheetTarget->setCurrentIndex(toBeInsertedIndex);
        }

    }

    //default selection
    this->_characterSheetTarget->insertItem(0, " Se connecter sans personnage", 0);
    if(previouslySelectedCharacterId == 0) {
        this->_characterSheetTarget->setCurrentIndex(0);
    }

}
