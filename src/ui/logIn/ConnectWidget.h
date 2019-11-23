#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>
#include <QPushButton>
#include <QMessageBox>
#include <QComboBox>

#include "src/helpers/_appContext.h"
#include "src/network/rpz/client/RPZClient.h"

#include "src/shared/hints/HintThread.hpp"

#include "src/shared/database/CharactersDatabase.h"
#include "src/ui/_others/ConnectivityObserver.h"

class ConnectWidget : public QWidget, public ConnectivityObserver {

    Q_OBJECT

    public:
        enum class State { NotConnected, Connecting, Connected };
        Q_ENUM(State)

        ConnectWidget(QWidget *parent = nullptr);

    protected:
        void connectingToServer() override;
        void connectionClosed(bool hasInitialMapLoaded) override;

    private:
        QLineEdit* _domainTarget = nullptr;
        QLineEdit* _nameTarget = nullptr;
        QComboBox* _characterSheetTarget = nullptr;
        
        QPushButton* _connectBtn = nullptr;

        ConnectWidget::State _state = ConnectWidget::State::NotConnected;
        
        void _tryConnectToServer();
        void _tryDisconnectingFromServer();

        void _changeState(ConnectWidget::State newState);

        void _onConnectButtonPressed();
        void _onRPZClientStatus(const QString &statusMsg, bool isError);
        void _onLogHistoryReceived();

        void _saveValuesAsSettings();

        void _fillCharacterSheetCombo();
        SnowFlake::Id _getSelectedCharacterId();
};  