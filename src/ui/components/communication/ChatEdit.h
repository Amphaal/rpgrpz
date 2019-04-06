#pragma once

#include <QWidget>
#include <QtWidgets/QPushButton>
#include <QLineEdit>
#include <QString>
#include <QHBoxLayout>
#include <QEvent>

class ChatEdit : public QWidget {

    Q_OBJECT

    public:
        ChatEdit(QWidget * parent = nullptr);
    
    signals:
        void askedToSendMessage(const QString &msg);
    
    protected:
        void changeEvent(QEvent *event) override;

    private:
        QLineEdit* _msgEdit = nullptr;
        QPushButton* _sendMsgBtn = nullptr;

        void _sendMessage();
};