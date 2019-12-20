#pragma once

#include "src/shared/payloads/_base/AlterationPayload.hpp"

#include "src/helpers/JSONSerializer.h"

class FogChangedPayload : public AlterationPayload {
    
    public:
        enum class ChangeType {
            Added,
            Removed,
            Reset
        };

        explicit FogChangedPayload(const QVariantHash &hash) : AlterationPayload(hash) {}
        FogChangedPayload(const ChangeType &type, const QList<QPolygonF> &modifyingPolys = QList<QPolygonF>()) : AlterationPayload(Payload::Alteration::FogChanged) {
            
            this->insert(QStringLiteral(u"ct"), (int)type);
            
            if(!modifyingPolys.isEmpty()) {
                this->insert(
                    QStringLiteral(u"mp"), 
                    JSONSerializer::fromPolygons(modifyingPolys)
                );
            }

        }
    
        ChangeType changeType() const {
            return (ChangeType)this->value(QStringLiteral(u"ct")).toInt();
        }

        const QList<QPolygonF> modifyingPolys() const {
            qDebug() << this->value("mp").toList();
            return JSONSerializer::toPolygons(
                this->value("mp").toList()
            );
        }
};