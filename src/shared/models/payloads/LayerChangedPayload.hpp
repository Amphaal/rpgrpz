#pragma once

#include "base/MultipleTargetsPayload.hpp"

class LayerChangedPayload : public MultipleTargetsPayload {
    public:
        LayerChangedPayload(const AlterationPayload::Source &source, const QList<QUuid> &changedAtomIds, const int newLayer) : 
        MultipleTargetsPayload(AlterationPayload::Alteration::LayerChanged, AlterationPayload::Source::Local, changedAtomIds) {
            this->_setLayer(newLayer);
        }

        int layer() {
            return this->value("lyr").toInt();
        }
    
    private:
        void _setLayer(const int layer) {
            (*this)["lyr"] = layer;
        }

};