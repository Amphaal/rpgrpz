#pragma once

#include "src/shared/map/MapHint.h"
#include "RPZAsset.hpp"

#include "Serializable.hpp"

class AlterationPayload : public Serializable {
    public:
        AlterationPayload(const RPZAsset::Alteration &alteration, QVector<RPZAsset> &assets) : 
        Serializable(NULL), 
        _alteration(alteration),
        _assets(assets) { };

        QVariantHash toVariantHash() override {
            QVariantHash out;

            out.insert("state", (int)this->_alteration);

            QVariantList assets;
            for(auto &i : this->_assets) {
               assets.append(i.toVariantHashWithData(this->_alteration));
            }

            out.insert("assets", assets);

            return out;
        };

        QVector<RPZAsset>* assets() { return &this->_assets; };
        RPZAsset::Alteration alteration() { return this->_alteration; };

        static AlterationPayload fromVariantHash(const QVariantHash &data) {
            
            auto state = (RPZAsset::Alteration)data["state"].toInt();
            
            QVector<RPZAsset> assets;
            for(auto &i : data["assets"].toList()) {
                assets.append(RPZAsset::fromVariantHash(i.toHash()));
            }
            
            return AlterationPayload(state, assets);
        };


    private:
        RPZAsset::Alteration _alteration;
        QVector<RPZAsset> _assets;
};