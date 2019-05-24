#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#include <QString>
#include <QMovie>

class RPZStatusLabel : public QWidget {

    Q_OBJECT

    public:
        enum State {Finished = 0, Waiting, Processing, Error };

        RPZStatusLabel(QString descriptor, QWidget* parent = nullptr) : QWidget(parent), 
        _descriptorLbl(new QLabel(descriptor + ":")), 
        _dataLbl(new QLabel),
        _loaderLbl(new QLabel) {
            
            this->setLayout(new QHBoxLayout);
            this->layout()->setContentsMargins(0, 0, 0, 0);
            this->layout()->addWidget(this->_descriptorLbl);
            this->layout()->addWidget(this->_dataLbl);
            
            //default state
            this->updateState("<En attente...>", Waiting);
        }

        void updateState(const QString &stateText, int state = Finished) {
            
            auto mustWait = (state == Processing);
            this->setWaiting(mustWait);

            this->_dataLbl->setText(stateText);
        };

        void setWaiting(bool waiting) {
            
            //unset waiting
            if(!waiting) {

                if(this->_loader) {
                    delete this->_loader;
                    this->_loader = nullptr;
                    delete this->_loaderLbl;
                    this->_loaderLbl = nullptr;
                }

                return;
            }

            else {

                //if loader already displayed, do nothing
                if(this->_loader) return;

                //add load spinner
                this->_loader = new QMovie(":/icons/app/loader.gif");
                this->_loaderLbl = new QLabel();
                this->_loaderLbl->setMovie(this->_loader);
                this->_loader->start();
                ((QHBoxLayout*)this->layout())->insertWidget(0, this->_loaderLbl);

            }

        }

        QLabel* dataLabel() {
            return this->_dataLbl;
        }

    private:
        QLabel* _descriptorLbl;
        QLabel* _dataLbl;
        QLabel* _loaderLbl = nullptr;
        QMovie* _loader = nullptr;
       
};