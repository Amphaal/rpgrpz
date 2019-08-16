#pragma once

#include <QWidget>
#include <QtWidgets/QPushButton>
#include <QLineEdit>
#include <QString>
#include <QHBoxLayout>
#include <QEvent>
#include <QCompleter>
#include <QStringListModel>

#include "src/ui/others/ClientBindable.h"

#include "src/shared/models/base/RPZList.hpp"

class ChatEdit : public QWidget, public ClientBindable {

    Q_OBJECT

    public:
        ChatEdit(QWidget * parent = nullptr);
    
    signals:
        void askedToSendMessage(const QString &msg);
    
    protected:
        void changeEvent(QEvent *event) override;
        void onRPZClientConnecting() override;

    private:
        QLineEdit* _msgEdit = nullptr;
        QPushButton* _sendMsgBtn = nullptr;

        void _sendMessage();
        
        void _onUsersChanged(const QVariantList &users);
};