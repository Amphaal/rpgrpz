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

#include "src/shared/hints/MapHint.h"

#include "src/shared/database/CharactersDatabase.hpp"

class ConnectWidget : public QWidget {

    Q_OBJECT

    public:
        enum State { NotConnected, Connecting, Connected };

        ConnectWidget(MapHint* hintToControlStateOf);

    signals:
        void startingConnection(RPZClient* cc);

    private:
        QLineEdit* _domainTarget = nullptr;
        QLineEdit* _nameTarget = nullptr;
        QComboBox* _characterSheetTarget = nullptr;
        
        QPushButton* _connectBtn = nullptr;

        RPZClient* _cc = nullptr;
        ConnectWidget::State _state = ConnectWidget::State::NotConnected;
        
        void _tryConnectToServer();
        void _tryDisconnectingFromServer();

        void _changeState(ConnectWidget::State newState);
        void _destroyClient();

        void _onConnectButtonPressed();
        void _onRPZClientStatus(const QString &statusMsg, bool isError);
        void _onRPZClientConnecting();

        void _saveValuesAsSettings();

        MapHint* _toControlStateOf = nullptr; 

        void _fillCharacterSheetCombo();
        snowflake_uid _getSelectedCharacterId();
};  