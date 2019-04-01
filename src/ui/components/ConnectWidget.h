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

#include "src/helpers/_const.cpp"
#include "src/network/rpz/client/RPZClient.h"

class ConnectWidget : public QGroupBox {

    Q_OBJECT

    signals:
        void connectionSuccessful(RPZClient* cc);
        void startingConnection(RPZClient* cc);

    public:
        ConnectWidget(QWidget * parent = nullptr);

    private:
        QLineEdit* _portTarget = 0;
        QLineEdit* _domainTarget = 0;
        QLineEdit* _nameTarget = 0;
        
        QPushButton* _connectBtn = 0;
        QMetaObject::Connection _connectBtnLink;

        RPZClient* _cc = 0;
        bool _connected = false;
        
        void _tryConnectToServer();
        void _tryDisconnectingFromServer();

        void _setConnectBtnState(bool readyForConnection = true);
        void _destroyClient();

        void _onRPZClientError(const std::string errMsg);
        void _onRPZClientConnected();
};  