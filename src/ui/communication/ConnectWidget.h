#pragma once

#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QIntValidator>
#include <QPushButton>
#include <QMessageBox>

#include "src/helpers/_appContext.h"
#include "src/network/rpz/client/RPZClient.h"

class ConnectWidget : public QWidget {

    Q_OBJECT

    signals:
        void connectionSuccessful(RPZClient* cc);
        void startingConnection(RPZClient* cc);

    public:
        enum State { NotConnected, Connecting, Connected };

        ConnectWidget(QWidget * parent = nullptr);

    private:
        QLineEdit* _portTarget = nullptr;
        QLineEdit* _domainTarget = nullptr;
        QLineEdit* _nameTarget = nullptr;
        
        QPushButton* _connectBtn = nullptr;
        QMetaObject::Connection _connectBtnLink;

        RPZClient* _cc = nullptr;
        ConnectWidget::State _state = ConnectWidget::State::NotConnected;
        
        void _tryConnectToServer();
        void _tryDisconnectingFromServer();

        void _changeState(ConnectWidget::State newState);
        void _destroyClient();

        void _onRPZClientError(const QString &errMsg);
        void _onRPZClientConnecting();

        void _saveValuesAsSettings();
};  