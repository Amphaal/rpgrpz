#pragma once

#include <QWidget>
#include <QPixMap>
#include <QLabel>

#include "base/LogContainer.h"

#include "src/shared/models/RPZUser.hpp"

#include <QFrame>
#include <QColor>

class UserLogColor : public QFrame {
    public:
        UserLogColor(const QColor &color) {
            this->setFrameShape(QFrame::Shape::Box); 
            this->setLineWidth(1); 
            this->setFixedSize(10, 10);
            this->setAutoFillBackground(true);
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

            auto pal = this->palette();
            pal.setColor(QPalette::Background, color);
            this->setPalette(pal);
        };
        UserLogColor(RPZUser &user) : UserLogColor(user.color()) {};

};

class UserLogIcon : public QLabel {
    public:
        UserLogIcon(RPZUser &user) {

            this->setMargin(0);
            this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum); 

            auto pathToIcon = RPZUser::IconsByRoles[user.role()];
            auto pixAsIcon = QPixmap(pathToIcon);
            this->setPixmap(pixAsIcon.scaled(14, 14));

        }
};

class UsersLog : public LogContainer {

    public:
        UsersLog(QWidget *parent = nullptr) : LogContainer(parent) {};

        void updateUsers(const QVariantList &users) {
   
            this->clearLines();

            for(auto &rawUser : users) {
                RPZUser user(rawUser.toHash());
                this->_addUserLog(user);
            }

        };

    private:
        void _addUserLog(RPZUser &user) {
            
            auto line = LogContainer::_addLine(user);

            //logo part
            auto userIcon = new UserLogIcon(user);
            line->layout()->addWidget(userIcon);

            //name
            auto name = new LogText(user.name());
            line->layout()->addWidget(name);
            
            //color descriptor
            auto userColor = new UserLogColor(user);
            line->layout()->addWidget(userColor);
        };

};