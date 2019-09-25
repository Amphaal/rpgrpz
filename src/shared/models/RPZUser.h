#pragma once
 
#include <QString>
#include <QColor>
#include <QVariantHash>
#include <QHostAddress>

#include "src/helpers/RandomColor.h"
#include "base/Serializable.hpp"
#include "src/shared/commands/MessageInterpreter.h"

typedef snowflake_uid RPZUserId;

class RPZUser : public Serializable {

    public:
        enum Role { R_Unknown, Host };
        static const inline QHash<Role, QString> IconsByRoles = { 
            {R_Unknown, ":/icons/app/connectivity/user.png"},
            {Host, ":/icons/app/connectivity/crown.png"}
        };
        
        RPZUser();
        RPZUser(const QVariantHash &hash);
        RPZUser(RPZUserId id, const QString &name, const Role &role, const QColor &color);

        void setName(const QString &name);
        void setRole(const Role &role);
        void randomiseColor();

        QString name() const;
        QString whisperTargetName() const;
        Role role() const;
        QColor color() const;
        QString toString() const;

    private:
        void _setColor(const QColor &color = QColor());

};