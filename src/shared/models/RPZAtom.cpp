#include "RPZAtom.h"

void RPZAtom::updateGraphicsItemFromMetadata(QGraphicsItem* item, const Parameters &param, QVariant &val) {
    
    if(!item) return;
    
    auto _updtTOP = [&]() {
        item->setTransformOriginPoint(
            item->boundingRect().center()
        );
    };

    switch(param) {
                    
        //on moving
        case RPZAtom::Parameters::Position: {
            auto destPos = val.toPointF();
            item->setPos(destPos);  
        }
        break;

        //on scaling
        case RPZAtom::Parameters::Scale: {
            
            auto destScale = val.toDouble();
            item->setScale(destScale);

            // _updtTOP();

        }
        break;

        // on locking change
        case RPZAtom::Parameters::Locked: {
            auto locked = val.toBool();
            auto flags = !locked ? MapViewItemsNotifier::defaultFlags() : 0;
            item->setFlags(flags);
        }
        break;
        
        // on changing visibility
        case RPZAtom::Parameters::Hidden: {
            auto hidden = val.toBool();
            auto opacity = hidden ? .05 : 1;
            item->setOpacity(opacity);
        }
        break;

        //on rotation
        case RPZAtom::Parameters::Rotation: {
            auto destRotation = val.toInt();
            item->setRotation(destRotation);

            // _updtTOP();
        }
        break;

        //on text size change
        case RPZAtom::Parameters::TextSize: {
            if(auto cItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                auto newSize = val.toInt();
                auto font = cItem->font();
                font.setPointSize(newSize);
                cItem->setFont(font);

                // _updtTOP();
            }
        }
        break;

        //on pen width change
        case RPZAtom::Parameters::PenWidth: {
            if(auto cItem = dynamic_cast<QGraphicsPathItem*>(item)) {
                auto newWidth = val.toInt();
                auto pen = cItem->pen();
                pen.setWidth(newWidth);
                cItem->setPen(pen);
            }
        }
        break;

        //on text change
        case RPZAtom::Parameters::Text: {
            if(auto cItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                auto newText = val.toString();
                cItem->setPlainText(newText);

                // _updtTOP();
            }
        }
        break;

        //on layer change
        case RPZAtom::Parameters::Layer: {
            auto newLayer = val.toInt();
            item->setZValue(newLayer);
        }
        break;
    }
};

RPZAtom::RPZAtom() {}
RPZAtom::RPZAtom(const QVariantHash &hash) : Ownable(hash) {}
RPZAtom::RPZAtom(const snowflake_uid &id, const AtomType &type, const RPZUser &owner) : Ownable(id, owner) {
    this->_setType(type);
};
RPZAtom::RPZAtom(const AtomType &type) : Ownable(SnowFlake::get()->nextId()) {
    this->_setType(type);  
};

QGraphicsItem* RPZAtom::graphicsItem() { 
    return this->_graphicsItem; 
};

void RPZAtom::setGraphicsItem(QGraphicsItem* item) { 
    this->_graphicsItem = item; 
};

//overrides descriptor
QString RPZAtom::descriptor() { 

    //displays asset name
    auto asname = this->assetName();
    if(!asname.isEmpty()) {
        return asname + " (" + this->_defaultDescriptor() +")";
    }

    return this->_defaultDescriptor();
};

QSet<RPZAtom::Parameters> RPZAtom::customizableParams() {
    QSet<RPZAtom::Parameters> out { Scale, Rotation };
    
    switch(this->type()) {
        case AtomType::Drawing:
            out.insert(RPZAtom::Parameters::PenWidth);
            break;
        case AtomType::Text:
            out.insert(RPZAtom::Parameters::TextSize);
            break;
    }

    return out;
}

QString RPZAtom::_defaultDescriptor() {
    switch(this->type()) {
        case AtomType::Drawing:
            return "Dessin";
        case AtomType::Text:
            return "Texte";
        case AtomType::Object:
            return "Objet";
        case AtomType::Brush:
            return "Brosse";
        default:
            return "Atome";
    }
}

AtomType RPZAtom::type() {return (AtomType)this->value("t").toInt();}
void RPZAtom::_setType(const AtomType &type) { this->insert("t", (int)type); }
void RPZAtom::changeType(const AtomType &type) { this->_setType(type);}


void RPZAtom::setMetadata(const Parameters &key, const QVariant &value) {
    
    if(value.isNull()) {
        this->remove(_str[key]);
        return;
    }

    switch(key) {

        case Parameters::Position: {
            auto pos = value.toPointF();
            QVariantList a { pos.x(), pos.y() };
            this->insert(_str[Position], a);
        }
        break;

        default:
            this->insert(_str[key], value);
            
    }
    
}

QVariant RPZAtom::metadata(const Parameters &key) {
    switch(key) {

        case Parameters::Position: {
            auto posArr = this->value(_str[Position]).toList();
            return posArr.isEmpty() ? QPointF() : QPointF(posArr[0].toReal(), posArr[1].toReal());
        }
        break;

        default:
            return this->value(_str[key], _defaultVal[key]);

    }
}

QSet<RPZAtom::Parameters> RPZAtom::hasMetadata() {
    QSet<RPZAtom::Parameters> out;

    for (QHash<Parameters, QString>::const_iterator i = _str.constBegin(); i != _str.constEnd(); ++i) {
        if(this->contains(i.value())) out.insert(i.key());
    }
    
    return out;
}

QString RPZAtom::assetId() { return this->metadata(AssetId).toString(); }
QString RPZAtom::assetName() { return this->metadata(AssetName).toString();}
double RPZAtom::scale() { return this->metadata(Scale).toDouble();}
double RPZAtom::rotation() { return this->metadata(Rotation).toDouble(); }
QString RPZAtom::text() { return this->metadata(Text).toString(); }
int RPZAtom::textSize() { return this->metadata(TextSize).toInt(); }
int RPZAtom::layer() { return this->metadata(Layer).toInt(); }
QPointF RPZAtom::pos() { return this->metadata(Position).toPointF();}
int RPZAtom::penWidth() { return this->metadata(PenWidth).toInt(); }
bool RPZAtom::isHidden() { return this->metadata(Hidden).toBool(); }
bool RPZAtom::isLocked() { return this->metadata(Locked).toBool(); }

QPainterPath RPZAtom::shape() {return JSONSerializer::toPainterPath(this->metadata(Shape).toByteArray());}
void RPZAtom::setShape(const QPainterPath &path) { this->setMetadata(Shape, JSONSerializer::asBase64(path)); }
void RPZAtom::setShape(const QRectF &rect) {
    QPainterPath shape;
    shape.addRect(rect);
    this->setShape(shape);
}