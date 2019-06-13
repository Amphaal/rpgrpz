#include "RPZAtom.h"

QByteArray JSONSerializer::asBase64(const QPainterPath &path) {
    QByteArray bArray;
    QDataStream stream(&bArray, QIODevice::WriteOnly);
    stream << path;
    return bArray.toBase64();
}

QPainterPath JSONSerializer::toPainterPath(const QByteArray &base64) {
    auto b64 = QByteArray::fromBase64(base64);
    QDataStream stream(&b64, QIODevice::ReadOnly);
    auto returned = QPainterPath();
    stream >> returned;
    return returned;
}

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
    if(!asname.isNull()) return asname;

    return this->_defaultDescriptor();
};


QString RPZAtom::_defaultDescriptor() {
    switch(this->type()) {
        case AtomType::Drawing:
            return "Dessin";
            break;
        case AtomType::Text:
            return "Texte";
            break;
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

QList<RPZAtom::Parameters> RPZAtom::hasMetadata() {
    QList<RPZAtom::Parameters> out;

    for (QHash<Parameters, QString>::const_iterator i = _str.constBegin(); i != _str.constEnd(); ++i) {
        if(this->contains(i.value())) out.append(i.key());
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