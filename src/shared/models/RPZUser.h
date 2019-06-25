#pragma once
 
#include <QString>
#include <QColor>
#include <QVariantHash>
#include <QHostAddress>

#include "src/network/rpz/_any/JSONSocket.h"

#include "src/helpers/RandomColor.h"
#include "base/Serializable.hpp"

class RPZUser : public Serializable {

    public:
        enum Role { Unknown, Host };
        static const inline QHash<Role, QString> IconsByRoles = { 
            {Unknown, ":/icons/app/connectivity/user.png"},
            {Host, ":/icons/app/connectivity/crown.png"}
        };
        
        RPZUser();
        RPZUser(const QVariantHash &hash);
        RPZUser(JSONSocket* socket);
        RPZUser(snowflake_uid id, const QString name, const Role &role, const QColor &color);

        void setName(const QString &name);
        void setRole(const Role &role);

        JSONSocket* jsonHelper();
        QString name();
        Role role();
        QColor color();
        QString toString();

    private:
        void _setColor(const QColor &color = QColor());

        JSONSocket* _jsonHelper;
        QString _localAddress;

};