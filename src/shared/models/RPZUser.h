#pragma once
 
#include <QString>
#include <QColor>
#include <QVariantHash>
#include <QHostAddress>

#include "src/helpers/RandomColor.h"
#include "base/Serializable.hpp"
#include "src/shared/commands/MessageInterpreter.h"

#include "src/shared/models/character/RPZCharacter.hpp"

typedef snowflake_uid RPZUserId;

class RPZUser : public Serializable {

    public:
        enum Role { Observer, Host, Player };
        static const inline QHash<Role, QString> IconsByRoles = { 
            {Observer, ":/icons/app/connectivity/observer.png"},
            {Host, ":/icons/app/connectivity/crown.png"},
            {Player, ":/icons/app/connectivity/cloak.png"}
        };
        
        RPZUser();
        RPZUser(const QVariantHash &hash);
        RPZUser(RPZUserId id, const QString &name, const Role &role, const QColor &color);

        void setName(const QString &name);
        void setRole(const Role &role);
        void setCharacter(const RPZCharacter &character);
        void randomiseColor();

        QString name() const;
        QString whisperTargetName() const;
        Role role() const;
        QColor color() const;
        QString toString() const;
        const RPZCharacter character() const;

    private:
        void _setColor(const QColor &color = QColor());

};