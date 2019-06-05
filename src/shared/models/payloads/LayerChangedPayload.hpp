#pragma once

#include "base/MultipleTargetsPayload.hpp"

class LayerChangedPayload : public MultipleTargetsPayload {
    public:
        LayerChangedPayload(const QVariantHash &hash) : MultipleTargetsPayload(hash) {}
        LayerChangedPayload(const QVector<snowflake_uid> &changedAtomIds, const int newLayer) : MultipleTargetsPayload(AlterationPayload::Alteration::LayerChanged, changedAtomIds) {
            this->_setLayer(newLayer);
        }

        int layer() {
            return this->value("lyr").toInt();
        }

        QVariantMap alterationByAtomId() override {
            
            auto list = this->targetAtomIds();
            auto layer = this->layer();

            QVariantMap out;
            for(auto &e : list) {
                out.insert(QString::number(e), layer);
            }

            return out;
        }
    
    private:
        void _setLayer(const int layer) {
            (*this)["lyr"] = layer;
        }

};