#pragma once

#include <QWidget>
#include <QPixMap>
#include <QLabel>

#include "base/LogScrollView.h"
#include "base/ColorIndicator.hpp"

#include "src/shared/models/entities/RPZUser.hpp"

class UsersLog : public LogScrollView {
    
    Q_OBJECT

    public:
        UsersLog(QWidget *parent = nullptr) : LogScrollView(parent) {};
        
        QHash<QUuid, RPZUser>* users() {
            return &this->_users;
        };

    public slots:
        void updateUsers(const QVariantHash &users) {

            //clear        
            this->newLog();
            this->_users.clear();

            for(auto &var_user : users) {
                
                //add user to list
                RPZUser user(var_user.toHash());
                _users.insert(user.id(), user);

                //write line
                this->writeAtEnd(user);
            }
        };

    private:
        QHash<QUuid, RPZUser> _users;

        void writeAtEnd(RPZUser &user) {
            
            auto line = LogScrollView::writeAtEnd(user.name());
            if(!line) return;

            //logo part
            auto pathToIcon = RPZUser::IconsByRoles[user.role()];
            if(!pathToIcon.isEmpty()) {
                auto icon = new QLabel; 
                icon->setMargin(0); 
                auto pixAsIcon = QPixmap(pathToIcon);
                icon->setPixmap(pixAsIcon.scaled(14, 14));
                icon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
                ((QBoxLayout*)line->layout())->insertWidget(0, icon);
            }

            //color descriptor
            auto colorwidget = new ColorIndicator(user.color());
            line->layout()->addWidget(colorwidget);
        };

};