#include "UsersLog.h"

UserLogColor::UserLogColor(const QColor &color) {
    this->setFrameShape(QFrame::Shape::Box); 
    this->setLineWidth(1); 
    this->setFixedSize(10, 10);
    this->setAutoFillBackground(true);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    auto pal = this->palette();
    pal.setColor(QPalette::Window, color);
    this->setPalette(pal);
};
UserLogColor::UserLogColor(RPZUser &user) : UserLogColor(user.color()) {};

UserLogIcon::UserLogIcon(RPZUser &user) {

    this->setMargin(0);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum); 

    auto pathToIcon = RPZUser::IconsByRoles[user.role()];
    auto pixAsIcon = QPixmap(pathToIcon);
    this->setPixmap(pixAsIcon.scaled(14, 14));

}

UsersLog::UsersLog(QWidget *parent) : LogContainer(parent) {};

void UsersLog::updateUsers(const QVariantList &users) {

    this->clearLines();

    for(auto &rawUser : users) {
        RPZUser user(rawUser.toHash());
        this->_addUserLog(user);
    }

};

void UsersLog::_addUserLog(RPZUser &user) {
    
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