#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QScrollArea>
#include <QtGui/QWindow>
#include <QStyle>

#include "LogScrollView.h"

class ChatWidget : public QWidget {

    public:
        enum LogType { Default, ServerLog, ClientMessage };

        ChatWidget(QWidget *parent = nullptr) : 
                    QWidget(parent),
                    scrollArea(new QScrollArea),
                    msgEdit(new QLineEdit),
                    sendBtn(new QPushButton) {
                
                //UI...
                this->_instUI();

                //bindings...
                QObject::connect(
                    this->scrollArea->verticalScrollBar(), &QScrollBar::rangeChanged,
                    this, &ChatWidget::_scrollUpdate
                );
            
                //initial log
                createNewLog();
        }
     
        void createNewLog() {
            if(!this->lsv) delete this->lsv;
            this->lsv = new LogScrollView;
            this->scrollArea->setWidget(this->lsv);
        };

        void printLog(const std::string &message, ChatWidget::LogType logType = ChatWidget::LogType::Default) {
            
            QPalette* colors = 0;

            switch(logType) {
                case ChatWidget::LogType::ServerLog:
                    colors = new QPalette();
                    colors->setColor(QPalette::Window, Qt::red);
                    colors->setColor(QPalette::WindowText, Qt::black);
                    break;
                case ChatWidget::LogType::ClientMessage:
                    break;
            }

            this->lsv->addMessage(message, colors);
        };

    private:
        QPushButton *sendBtn = 0;
        LogScrollView *lsv = 0;
        QScrollArea *scrollArea = 0;
        QLineEdit* msgEdit = 0;

        void _scrollUpdate() {
            //to perform heavy CPU consuming action
            auto tabScrollBar = this->scrollArea->verticalScrollBar();
            tabScrollBar->setValue(tabScrollBar->maximum());
        }

        void _instUI() {
                //this...
                this->setLayout(new QVBoxLayout);
                this->layout()->setContentsMargins(10, 0, 10, 0);
                 
                //scroll
                this->scrollArea->setWidgetResizable(true);
                this->scrollArea->setAutoFillBackground(true);
                this->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
                this->scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
                auto pal = this->scrollArea->palette();
                pal.setColor(QPalette::Background, Qt::white);
                this->scrollArea->setPalette(pal);
                this->layout()->addWidget(this->scrollArea);

                //messaging
                auto msgWdgt = new QWidget;
                msgWdgt->setLayout(new QHBoxLayout);
                msgWdgt->layout()->setMargin(0);
                this->sendBtn->setText("Envoyer Message");
                this->msgEdit->setPlaceholderText("Message à envoyer");
                msgWdgt->layout()->addWidget(this->msgEdit);
                msgWdgt->layout()->addWidget(this->sendBtn);
                this->layout()->addWidget(msgWdgt);
        }

};