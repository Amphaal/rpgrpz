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
UserLogColor::UserLogColor(const RPZUser &user) : UserLogColor(user.color()) {};

UserLogIcon::UserLogIcon(const RPZUser &user) {

    this->setMargin(0);
    this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum); 

    //get cached pixmap
    QPixmap pixAsIcon;
    auto pathToIcon = RPZUser::IconsByRoles[user.role()];
    auto foundCached = QPixmapCache::find(pathToIcon, &pixAsIcon);
    if(!foundCached) {
        pixAsIcon = QPixmap(pathToIcon);
        pixAsIcon = pixAsIcon.scaled(14, 14);
        QPixmapCache::insert(pathToIcon, pixAsIcon);
    }
    this->setPixmap(pixAsIcon);

}

UsersLog::UsersLog(QWidget *parent) : LogContainer(parent) {};

void UsersLog::updateUsers(const QVector<RPZUser> &users) {

    this->clearLines();

    for(auto &user : users) {
        this->_addUserLog(user);
    }

};

void UsersLog::_addUserLog(const RPZUser &user) {
    
    auto line = LogContainer::_addLine(user);
    line->setToolTip(user.whisperTargetName());

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