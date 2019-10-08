#pragma once

#include <QListView>

#include "UsersModel.hpp"
#include "UserItemDelegate.hpp"

class UsersListView : public QListView, public ClientBindable {
    public:
        UsersListView() {

            this->setVisible(false);
            this->setMaximumWidth(120);

            this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            this->setLayoutDirection(Qt::LayoutDirection::LeftToRight);

            this->setItemDelegate(new UserItemDelegate);
            
            this->setModel(new UsersModel);

        }
    
    private:
        void onRPZClientDisconnect() override {
            this->setVisible(false);
        }

        void onRPZClientConnecting() override {
            this->setVisible(true);
        }

};