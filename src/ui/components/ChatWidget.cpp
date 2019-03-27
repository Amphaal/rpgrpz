#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QScrollArea>
#include <QtGui/QWindow>
#include <QStyle>

#include "LogScrollView.cpp"


class ChatWidget : public QWidget {

    public:
        ChatWidget(QWidget *parent = nullptr) : 
                    QWidget(parent),
                    scrollArea(new QScrollArea),
                    msgEdit(new QLineEdit),
                    sendBtn(new QPushButton) {
                
                //this...
                this->setLayout(new QVBoxLayout);
                this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                this->layout()->setContentsMargins(10, 0, 10, 0);
      
                //scroll
                auto pal = this->scrollArea->palette();
                pal.setColor(QPalette::Background, "#FFFFFF");
                this->scrollArea->setAutoFillBackground(true);
                this->scrollArea->setPalette(pal);
                this->layout()->addWidget(this->scrollArea);
                QObject::connect(
                    this->scrollArea->verticalScrollBar(), &QScrollBar::rangeChanged,
                    this, &ChatWidget::scrollUpdate
                );

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


    protected:
        QPushButton *sendBtn = 0;
        LogScrollView *lsv = 0;
        QScrollArea *scrollArea = 0;
        QLineEdit* msgEdit = 0;
                
        void createNewLog() {
            if(!this->lsv) delete this->lsv;
            this->lsv = new LogScrollView(this);
            this->scrollArea->setWidget(this->lsv);
        };

        void printLog(const std::string &message, const bool replacePreviousLine, const bool isError) {
            
            if(replacePreviousLine) {
                this->lsv->updateLatestMessage(message);
            } else {
                this->lsv->addMessage(message, isError);
            }
        };

        void scrollUpdate(int min, int max) {
            //to perform heavy CPU consuming action
            auto tabScrollBar = this->scrollArea->verticalScrollBar();
            tabScrollBar->setValue(tabScrollBar->maximum());
        }

};