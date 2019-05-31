#pragma once

#include "base/MultipleTargetsPayload.hpp"

class LayerChangedPayload : public MultipleTargetsPayload {
    public:
        LayerChangedPayload(const QList<QUuid> &changedAtomIds, const int newLayer) : 
        MultipleTargetsPayload(AlterationPayload::Alteration::LayerChanged, changedAtomIds) {
            this->_setLayer(newLayer);
        }

        int layer() {
            return this->value("lyr").toInt();
        }

        QVariantHash alterationByAtomId() override {
            QVariantHash out;
            auto list = this->targetAtomIds();
            auto layer = this->layer();
            for(auto &e : list) {
                out.insert(e.toString(), layer);
            }
            return out;
        }
    
    private:
        void _setLayer(const int layer) {
            (*this)["lyr"] = layer;
        }

};