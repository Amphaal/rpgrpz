#pragma once

#include "base/MultipleTargetsPayload.hpp"

class RotatedPayload : public MultipleTargetsPayload {
    public:
        RotatedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        RotatedPayload(const QVector<snowflake_uid> &changedAtomIds, const double degrees) : MultipleTargetsPayload(AlterationPayload::Alteration::Rotated, changedAtomIds) {
            this->_setRotation(degrees);
        }

        double rotation() {
            return this->value("deg").toDouble();
        }

        QVariantHash alterationByAtomId() override {
            
            auto list = this->targetAtomIds();
            auto rotation = this->rotation();

            QVariantHash out;
            for(auto &e : list) {
                out.insert(QString::number(e), rotation);
            }

            return out;
        }
    
    private:
        void _setRotation(const double degrees) {
            (*this)["deg"] = degrees;
        }
};