#pragma once

#include <QString>

#include "base/AlterationPayload.hpp"

class TextChangedPayload : public AlterationPayload {
    public:
        TextChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        TextChangedPayload(const snowflake_uid &targetedAtomId, const QString &text) : AlterationPayload(AlterationPayload::TextChanged) {
            this->_setTargetAtomId(targetedAtomId);
            this->_setText(text);
        }
    
        snowflake_uid targetAtomId() {
            return this->value("id").toULongLong();
        }

        QString text() {
            return this->value("txt").toString();
        }

        QVariantMap alterationByAtomId() override {
            QVariantMap out;

            out.insert(QString::number(this->targetAtomId()), this->text());
            
            return out;
        }
    
    private:
        void _setTargetAtomId(const snowflake_uid &targetAtomId) {
            (*this)["id"] = QString::number(targetAtomId);
        }

        void _setText(const QString &text) {
            (*this)["txt"] = text;
        }

};