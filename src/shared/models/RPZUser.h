#pragma once
 
#include <QString>
#include <QColor>
#include <QVariantHash>
#include <QHostAddress>

#include "src/shared/models/_base/RPZMap.hpp"
#include "src/helpers/RandomColor.h"
#include "src/shared/models/_base/Serializable.hpp"
#include "src/shared/commands/MessageInterpreter.h"

#include "src/shared/models/character/RPZCharacter.hpp"

class RPZUser : public Serializable {

    public:
        using Id = SnowFlake::Id;

        enum class Role { Observer, Host, Player };

        static const inline QHash<RPZUser::Role, QString> IconsByRoles { 
            { Role::Observer, QStringLiteral(u":/icons/app/connectivity/observer.png") },
            { Role::Host, QStringLiteral(u":/icons/app/connectivity/crown.png") },
            { Role::Player, QStringLiteral(u":/icons/app/connectivity/cloak.png") }
        };
        
        RPZUser();
        explicit RPZUser(const QVariantHash &hash);
        RPZUser(RPZUser::Id id, const QString &name, const Role &role, const QColor &color);

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

inline uint qHash(const RPZUser::Role &key, uint seed = 0) {return uint(key) ^ seed;}