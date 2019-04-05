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
        void askedToSendMessage(QString msg);
    
    protected:
        void changeEvent(QEvent *event) override;

    private:
        QLineEdit* _msgEdit = 0;
        QPushButton* _sendMsgBtn = 0;

        void _sendMessage();
};