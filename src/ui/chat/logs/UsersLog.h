#pragma once

#include <QPixmapCache>

#include <QWidget>
#include <QPixMap>
#include <QLabel>

#include "base/LogContainer.h"

#include "src/shared/models/RPZUser.h"

#include <QFrame>
#include <QColor>

class UserLogColor : public QFrame {
    public:
        UserLogColor(const QColor &color);
        UserLogColor(const RPZUser &user);
};

class UserLogIcon : public QLabel {
    public:
        UserLogIcon(const RPZUser &user);
};

class UsersLog : public LogContainer {
    public:
        UsersLog(QWidget *parent = nullptr);
        void updateUsers(const QVector<RPZUser> &users);

    private:
        void _addUserLog(const RPZUser &user);
};