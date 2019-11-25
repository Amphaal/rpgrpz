#include "ConnectWidget.h"

ConnectWidget::ConnectWidget(QWidget *parent) : QWidget(parent), 
    _nameTarget(new QLineEdit(this)),
    _domainTarget(new QLineEdit(this)),
    _connectBtn(new QPushButton(this)),
    _characterSheetTarget(new QComboBox(this)) {
                                                    
    AppContext::settings()->beginGroup(QStringLiteral(u"ConnectWidget"));

    //this
    this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    auto mainLayout = new QVBoxLayout;
    this->setLayout(mainLayout);
    this->layout()->setMargin(0);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    //name target
    this->_nameTarget->addAction(QIcon(QStringLiteral(u":/icons/app/connectivity/user.png")), QLineEdit::LeadingPosition);
    this->_nameTarget->setPlaceholderText(tr("Player name"));
    this->_nameTarget->setToolTip(this->_nameTarget->placeholderText());
    this->_nameTarget->setText(AppContext::settings()->value(QStringLiteral(u"name")).toString());

    //domain target
    this->_domainTarget->addAction(QIcon(QStringLiteral(u":/icons/app/connectivity/server.png")), QLineEdit::LeadingPosition);
    this->_domainTarget->setPlaceholderText(tr("IP or server hostname"));
    this->_domainTarget->setPlaceholderText(this->_domainTarget->placeholderText());
    this->_domainTarget->setText(AppContext::settings()->value(QStringLiteral(u"domain"), "localhost").toString());

    //character sheet target
    this->_characterSheetTarget->setToolTip(tr("Character to embody"));
    this->_fillCharacterSheetCombo();
       
        //define preferences
        auto favChar = AppContext::settings()->value(QStringLiteral(u"favChar")).toULongLong();
        for(auto i = 0; i < this->_characterSheetTarget->count(); i++) {
            
            auto charId = this->_characterSheetTarget->itemData(i).toULongLong();
            if(charId != favChar) continue;

            this->_characterSheetTarget->setCurrentIndex(i);
            break;

        }

    QObject::connect(
        CharactersDatabase::get(), &CharactersDatabase::databaseChanged,
        this, &ConnectWidget::_fillCharacterSheetCombo
    );

    //bind to Return Key press...
    QObject::connect(this->_nameTarget, &QLineEdit::returnPressed, this->_connectBtn, &QPushButton::click);
    QObject::connect(this->_domainTarget, &QLineEdit::returnPressed, this->_connectBtn, &QPushButton::click);

    //default ui state
    this->_changeState(this->_state);
    QObject::connect(
        this->_connectBtn, &QPushButton::clicked,
        this, &ConnectWidget::_onConnectButtonPressed
    );

    //adding widgets
    auto layoutSub = new QHBoxLayout;
    layoutSub->addWidget(this->_nameTarget, 1);
    layoutSub->addWidget(this->_domainTarget);
    layoutSub->addWidget(this->_connectBtn);

    mainLayout->addWidget(this->_characterSheetTarget);
    mainLayout->addLayout(layoutSub);

    AppContext::settings()->endGroup();
}

void ConnectWidget::_onConnectButtonPressed() {
    
    this->_connectBtn->setEnabled(false);
    
    switch(this->_state) {
        
        case ConnectWidget::State::NotConnected: {
            this->_tryConnectToServer();
        }
        break;

        case ConnectWidget::State::Connecting:
        case ConnectWidget::State::Connected: {
            ConnectivityObserver::disconnectClient();
        }
        break;

    }

}

void ConnectWidget::_saveValuesAsSettings() {
    
    //register default values
    AppContext::settings()->beginGroup(QStringLiteral(u"ConnectWidget"));

    auto dt_text = this->_domainTarget->text();
    if(!dt_text.isEmpty()) AppContext::settings()->setValue(QStringLiteral(u"domain"), dt_text);

    auto nt_text = this->_nameTarget->text();
    if(!nt_text.isEmpty()) AppContext::settings()->setValue(QStringLiteral(u"name"), nt_text);

    auto favoriteCharacterId = this->_characterSheetTarget->currentData().toULongLong();
    AppContext::settings()->setValue(QStringLiteral(u"favChar"), favoriteCharacterId);

    AppContext::settings()->endGroup();
}

void ConnectWidget::_tryConnectToServer() {

    //ask for save the map as it may be erased
    MapHint::mayWantToSavePendingState(this, HintThread::hint());

    this->_saveValuesAsSettings();
    
    auto selectedCharacter = CharactersDatabase::get()->character(
        this->_getSelectedCharacterId()
    );
    
    auto cli = new RPZClient(
        this->_domainTarget->text(),
        this->_nameTarget->text(), 
        selectedCharacter
    );

    //start
    ConnectivityObserver::connectWithClient(cli);
    
}

void ConnectWidget::connectingToServer() {
    
    this->_changeState(State::Connecting);

    QObject::connect(
        this->_rpzClient, &RPZClient::gameSessionReceived, 
        this, &ConnectWidget::_onGameSessionReceived
    );

    QObject::connect(
        this->_rpzClient, &RPZClient::connectionStatus, 
        this, &ConnectWidget::_onRPZClientStatus
    );

}

void ConnectWidget::connectionClosed(bool hasInitialMapLoaded) {
    this->_changeState(State::NotConnected);
}

void ConnectWidget::_onRPZClientStatus(const QString &statusMsg, bool isError) {
    
    if(!isError) return;

    if(this->_state == State::Connecting) {
        QMessageBox::information(this, 
            tr("Error while connecting to server"), 
            statusMsg, 
            QMessageBox::Ok, QMessageBox::Ok);
    }

}
void ConnectWidget::_onGameSessionReceived(const RPZGameSession &gameSession) {
    
    Q_UNUSED(gameSession)

    this->_changeState(State::Connected);

}

void ConnectWidget::_changeState(ConnectWidget::State newState) {
    
    //btn text
    QString btnText; 
    switch(newState) {
        case ConnectWidget::State::NotConnected:
            btnText = tr("Connect");
            break;
        case ConnectWidget::State::Connecting:
            btnText = tr("Cancel (Connecting to %1...)").arg(this->_domainTarget->text());
            break;
        case ConnectWidget::State::Connected:
            btnText = tr("Disconnect from [%1]").arg(this->_domainTarget->text());
            break;
    }

    this->_connectBtn->setText(btnText);
    this->_connectBtn->setEnabled(true);

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


RPZCharacter::Id ConnectWidget::_getSelectedCharacterId() {
    return (RPZCharacter::Id)this->_characterSheetTarget->currentData().toULongLong();
}

void ConnectWidget::_fillCharacterSheetCombo() {
    
    auto previouslySelectedCharacterId = this->_getSelectedCharacterId(); //get previous selection
    this->_characterSheetTarget->clear(); //clear content

    //for each character in db
    for(const auto &character : CharactersDatabase::get()->characters()) {
        
        auto id = character.id();
        auto toBeInsertedIndex = this->_characterSheetTarget->count();

        this->_characterSheetTarget->addItem(
            QIcon(QStringLiteral(u":/icons/app/connectivity/self_cloak.png")), 
            character.toString(), 
            id
        );

        if(previouslySelectedCharacterId == id) {
            this->_characterSheetTarget->setCurrentIndex(toBeInsertedIndex);
        }

    }

    //default selection
    this->_characterSheetTarget->insertItem(0, tr(" Log in without a character"), 0);
    if(previouslySelectedCharacterId == 0) {
        this->_characterSheetTarget->setCurrentIndex(0);
    }

}
