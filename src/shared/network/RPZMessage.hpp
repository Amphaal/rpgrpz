#pragma once 

#include <QString>
#include <QUuid>
#include <QDateTime>

#include "RPZUser.hpp"
#include "../Serializable.hpp"
#include "Ownable.hpp"

class RPZMessage : public Serializable, public Ownable {
    public:
        RPZMessage() {};
        RPZMessage(const QUuid &id, const QString &message, const QDateTime &dt, const RPZUser &owner) : 
            Serializable(id),
            Ownable(owner),
            _message(message), 
            _dt(dt) { };

        RPZMessage(const QUuid &id, const QString &message, const QDateTime &dt) : 
            Serializable(id),
            _message(message), 
            _dt(dt){ };

        RPZMessage(const QString &message) : RPZMessage(
            QUuid::createUuid(), 
            message, 
            QDateTime::currentDateTime()
        ) { };

        static RPZMessage fromVariantHash(const QVariantHash &hash) {
            return RPZMessage(
                hash["id"].toUuid(), 
                hash["msg"].toString(), 
                hash["dt"].toDateTime(),
                RPZUser::fromVariantHash(hash["owner"].toHash())
            );
        };

        QString toString() {
            const auto ts = QString("[" + _dt.toString("dd.MM.yyyy-hh:mm:ss") + "] ");
            const auto name = this->owner().name() + " a dit : ";
            const auto fullMsg = ts + name + "“" + this->_message + "”";
            return fullMsg;
        };

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("msg", this->_message);
            out.insert("dt", this->_dt);
            
            this->injectOwnerDataToHash(out);

            return out;
        };


    private:
        QString _message;
        QDateTime _dt;
};