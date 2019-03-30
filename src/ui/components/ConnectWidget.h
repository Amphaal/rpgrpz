#pragma once

#include <QSettings>
#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>
#include <QPushButton>
#include <QMessageBox>

#include "src/helpers/_const.cpp"
#include "src/network/chat/client/ChatClient.h"

class ConnectWidget : public QWidget {

    Q_OBJECT

    signals:
        void connectionSuccessful(ChatClient* cc);
        void startingConnection(ChatClient* cc);

    public:
        ConnectWidget(QWidget * parent = nullptr);

    private:
        QLineEdit* _portTarget = 0;
        QLineEdit* _domainTarget = 0;
        QLineEdit* _nameTarget = 0;
        
        QPushButton* _connectBtn = 0;
        QMetaObject::Connection _connectBtnLink;

        ChatClient* _cc = 0;
        bool _connected = false;
        
        void _tryConnectToServer();
        void _tryDisconnectingFromServer();

        void _setConnectBtnState(bool readyForConnection = true);
        void _destroyClient();

        void _onChatClientError(const std::string errMsg);
        void _onChatClientConnected();
};  