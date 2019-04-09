#pragma once

#include <QWidget>

#include "base/LogScrollView.h"
#include "src/shared/RPZUser.hpp"


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
                auto user = RPZUser::fromVariantHash(var_user.toHash());
                _users.insert(user.id(), user);

                //icon
                auto pathToIcon = RPZUser::IconsByRoles[(int)user.role()];
                auto icon = QPixmap(pathToIcon);

                //write line
                this->writeAtEnd(user.name().toStdString(), nullptr, &icon);
            }
        };

    private:
        QHash<QUuid, RPZUser> _users;

};