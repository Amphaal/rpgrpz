#pragma once

#include <QSettings>
#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>

#include "src/helpers/_const.hpp"
#include "src/network/rpz/client/RPZClient.h"

class ConnectWidget : public QGroupBox {

    Q_OBJECT

    signals:
        void connectionSuccessful(RPZClient* cc);
        void startingConnection(RPZClient* cc);

    public:
        ConnectWidget(QWidget * parent = nullptr);

    private:
        QLineEdit* _portTarget = nullptr;
        QLineEdit* _domainTarget = nullptr;
        QLineEdit* _nameTarget = nullptr;
        
        QPushButton* _connectBtn = nullptr;
        QMetaObject::Connection _connectBtnLink;

        QSettings _settings;

        RPZClient* _cc = nullptr;
        bool _connected = false;
        
        void _tryConnectToServer();
        void _tryDisconnectingFromServer();

        void _setConnectBtnState(const bool readyForConnection = true);
        void _destroyClient();

        void _onRPZClientError(const std::string &errMsg);
        void _onRPZClientConnected();
};  