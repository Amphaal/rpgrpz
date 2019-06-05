#pragma once

#include "base/MultipleTargetsPayload.hpp"

class ScaledPayload : public MultipleTargetsPayload {
    public:
        ScaledPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        ScaledPayload(const QVector<snowflake_uid> &changedAtomIds, const double scale) : MultipleTargetsPayload(AlterationPayload::Alteration::Scaled, changedAtomIds) {
            this->_setScale(scale);
        }

        double scale() {
            return this->value("scl").toDouble();
        }

        QVariantMap alterationByAtomId() override {
            
            auto list = this->targetAtomIds();
            auto scale = this->scale();

            QVariantMap out;
            for(auto &e : list) {
                out.insert(QString::number(e), scale);
            }

            return out;
        }
    
    private:
        void _setScale(const double scale) {
            (*this)["scl"] = scale;
        }
};