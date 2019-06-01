#pragma once

#include <QUuid>
#include <QString>

#include "base/AlterationPayload.hpp"

class TextChangedPayload : public AlterationPayload {
    public:
        TextChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        TextChangedPayload(QUuid &targetedAtomId, const QString &text) : AlterationPayload(AlterationPayload::TextChanged) {
            this->_setTargetAtomId(targetedAtomId);
            this->_setText(text);
        }
    
        QUuid targetAtomId() {
            return this->value("id").toUuid();
        }

        QString text() {
            return this->value("txt").toString();
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;

            out.insert(this->targetAtomId().toString(), this->text());
            
            return out;
        }
    
    private:
        void _setTargetAtomId(const QUuid &targetAtomId) {
            (*this)["id"] = targetAtomId;
        }

        void _setText(const QString &text) {
            (*this)["txt"] = text;
        }

};