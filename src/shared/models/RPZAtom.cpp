#include "RPZAtom.h"

void RPZAtom::updateGraphicsItemFromMetadata(QGraphicsItem* item, const AtomParameter &param, QVariant &val) {
    
    if(!item) return;

    switch(param) {
                    
        //on moving
        case AtomParameter::Position: {
            auto destPos = val.toPointF();
            item->setPos(destPos);  
        }
        break;

        //on scaling
        case AtomParameter::Scale: {
            
            auto destScale = val.toDouble();
            item->setScale(destScale);

        }
        break;

        // on locking change
        case AtomParameter::Locked: {
            auto locked = val.toBool();
            auto flags = !locked ? MapViewItemsNotifier::defaultFlags() : 0;
            item->setFlags(flags);
        }
        break;
        
        // on changing visibility
        case AtomParameter::Hidden: {
            auto hidden = val.toBool();
            auto opacity = hidden ? .05 : 1;
            item->setOpacity(opacity);
        }
        break;

        //on rotation
        case AtomParameter::Rotation: {
            auto destRotation = val.toInt();
            item->setRotation(destRotation);
        }
        break;

        //on text size change
        case AtomParameter::TextSize: {
            if(auto cItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                auto newSize = val.toInt();
                auto font = cItem->font();
                font.setPointSize(newSize);
                cItem->setFont(font);
            }
        }
        break;

        //on pen width change
        case AtomParameter::PenWidth: {
            if(auto cItem = dynamic_cast<QGraphicsPathItem*>(item)) {
                auto newWidth = val.toInt();
                auto pen = cItem->pen();
                pen.setWidth(newWidth);
                cItem->setPen(pen);
            }
        }
        break;

        //on text change
        case AtomParameter::Text: {
            if(auto cItem = dynamic_cast<QGraphicsTextItem*>(item)) {
                auto newText = val.toString();
                cItem->setPlainText(newText);
            }
        }
        break;

        //on layer change
        case AtomParameter::Layer: {
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

QSet<AtomParameter> RPZAtom::customizableParams() {
    QSet<AtomParameter> out { AtomParameter::Scale, AtomParameter::Rotation };
    
    switch(this->type()) {
        case AtomType::Drawing:
            out.insert(AtomParameter::PenWidth);
            break;
        case AtomType::Text:
            out.insert(AtomParameter::TextSize);
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


void RPZAtom::setMetadata(const AtomParameter &key, const QVariant &value) {
    
    if(value.isNull()) {
        this->remove(_str[key]);
        return;
    }

    switch(key) {

        case AtomParameter::Position: {
            auto pos = value.toPointF();
            QVariantList a { pos.x(), pos.y() };
            this->insert(_str[AtomParameter::Position], a);
        }
        break;

        default:
            this->insert(_str[key], value);
            
    }
    
}

QVariant RPZAtom::metadata(const AtomParameter &key) {
    switch(key) {

        case AtomParameter::Position: {
            auto posArr = this->value(_str[AtomParameter::Position]).toList();
            return posArr.isEmpty() ? QPointF() : QPointF(posArr[0].toReal(), posArr[1].toReal());
        }
        break;

        default:
            return this->value(_str[key], _defaultVal[key]);

    }
}

QList<AtomParameter> RPZAtom::orderedEditedMetadata() {
    QList<AtomParameter> out;

    for (auto i = _str.constBegin(); i != _str.constEnd(); ++i) {
        if(this->contains(i.value())) out.append(i.key());
    }
    
    //order
    std::sort(out.begin(), out.end());

    return out;
}

QString RPZAtom::assetId() { return this->metadata(AtomParameter::AssetId).toString(); }
QString RPZAtom::assetName() { return this->metadata(AtomParameter::AssetName).toString();}
double RPZAtom::scale() { return this->metadata(AtomParameter::Scale).toDouble();}
double RPZAtom::rotation() { return this->metadata(AtomParameter::Rotation).toDouble(); }
QString RPZAtom::text() { return this->metadata(AtomParameter::Text).toString(); }
int RPZAtom::textSize() { return this->metadata(AtomParameter::TextSize).toInt(); }
int RPZAtom::layer() { return this->metadata(AtomParameter::Layer).toInt(); }
QPointF RPZAtom::pos() { return this->metadata(AtomParameter::Position).toPointF();}
int RPZAtom::penWidth() { return this->metadata(AtomParameter::PenWidth).toInt(); }
bool RPZAtom::isHidden() { return this->metadata(AtomParameter::Hidden).toBool(); }
bool RPZAtom::isLocked() { return this->metadata(AtomParameter::Locked).toBool(); }

QPainterPath RPZAtom::shape() {return JSONSerializer::toPainterPath(this->metadata(AtomParameter::Shape).toByteArray());}
void RPZAtom::setShape(const QPainterPath &path) { this->setMetadata(AtomParameter::Shape, JSONSerializer::asBase64(path)); }
void RPZAtom::setShape(const QRectF &rect) {
    QPainterPath shape;
    shape.addRect(rect);
    this->setShape(shape);
}