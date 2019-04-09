#pragma once 

#include <QString>
#include <QUuid>
#include <QDateTime>

#include "RPZUser.hpp"
#include "Serializable.hpp"

class RPZMessage : public Serializable {
    public:
        RPZMessage() {};
        RPZMessage(const QUuid &id, const QString &message, const QDateTime &dt, const QUuid &senderId, const QString &senderName) : 
            Serializable(id),
            _message(message), 
            _dt(dt), 
            _senderId(senderId),
            _senderName(senderName) { };

        RPZMessage(const QUuid &id, const QString &message, const QDateTime &dt, RPZUser* user = nullptr) : 
            Serializable(id),
            _message(message), 
            _dt(dt){ 

            this->setUser(user);

        };

        RPZMessage(const QString &message) : RPZMessage(
            QUuid::createUuid(), 
            message, 
            QDateTime::currentDateTime(), 
            nullptr
        ) { };

        static RPZMessage fromVariantHash(const QVariantHash &hash) {
            return RPZMessage(
                hash["id"].toUuid(), 
                hash["msg"].toString(), 
                hash["dt"].toDateTime(), 
                hash["sid"].toUuid(),
                hash["sname"].toString()
            );
        };

        void setUser(RPZUser* user) { 
            if(user) {
                this->_senderId = user->id();
                this->_senderName = user->name();
            }
        };

        QString toString() {
            const auto ts = QString("[" + _dt.toString("dd.MM.yyyy-hh:mm:ss") + "] ");
            const auto name = this->_senderName + " a dit : ";
            const auto fullMsg = ts + name + "“" + this->_message + "”";
            return fullMsg;
        };

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("id", this->id());
            out.insert("msg", this->_message);
            out.insert("dt", this->_dt);
            out.insert("sid", this->_senderId);
            out.insert("sname", this->_senderName);

            return out;
        };


    private:
        QString _message;
        QDateTime _dt;
        QUuid _senderId;
        QString _senderName;
};