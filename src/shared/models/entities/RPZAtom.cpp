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

AtomType RPZAtom::type() {return (AtomType)this->value(_str[Type]).toInt();}
void RPZAtom::_setType(const AtomType &type) { this->insert(_str[Type], (int)type); }
void RPZAtom::changeType(const AtomType &type) { this->_setType(type);}

QString RPZAtom::assetId() { return this->value(_str[AssetId]).toString(); }
void RPZAtom::setAssetId(const QString &id) { 
    if(id.isEmpty()) {
        this->remove(_str[AssetId]); 
        return;
    }
    this->insert(_str[AssetId], id); 
}

QString RPZAtom::assetName() { return this->value(_str[AssetName]).toString();}
void RPZAtom::setAssetName(const QString &name) {
    if(name.isEmpty()) {
        this->remove(_str[AssetName]); 
        return;
    }
    this->insert(_str[AssetName], name); 
}

double RPZAtom::scale() { return this->value(_str[Scale], 1.0).toDouble();}
void RPZAtom::setScale(const double scale) { this->insert(_str[Scale], scale); }

double RPZAtom::rotation() { return this->value(_str[Rotation], 0).toDouble(); }
void RPZAtom::setRotation(const double rotation) { this->insert(_str[Rotation], rotation); }

QString RPZAtom::text() { return this->value(_str[Text], "Saisir du texte").toString(); }
void RPZAtom::setText(const QString &text) {
    if(text.isEmpty()) {
        this->remove(_str[Text]); 
        return;
    }
    this->insert(_str[Text], text); 
}

int RPZAtom::layer() { return this->value(_str[Layer], 0).toInt(); }
void RPZAtom::setLayer(int pos) { this->insert(_str[Layer], pos); }

QPointF RPZAtom::pos() {
    auto arr = this->value(_str[Position]).toList();
    return arr.isEmpty() ? QPointF() : QPointF(arr[0].toReal(), arr[1].toReal());
}
void RPZAtom::setPos(const QPointF &pos) {
    QVariantList a { pos.x(), pos.y() };
    this->insert(_str[Position], a);
}

int RPZAtom::penWidth() { return this->value(_str[PenWidth], 1).toInt(); }
void RPZAtom::setPenWidth(int width) { this->insert(_str[PenWidth], width); }  

QPainterPath RPZAtom::shape() {
    return JSONSerializer::toPainterPath(
        this->value(_str[Shape]).toByteArray()
    );
}
void RPZAtom::setShape(const QPainterPath &path) { this->insert(_str[Shape], JSONSerializer::asBase64(path)); }
void RPZAtom::setShape(const QRectF &rect) {
    QPainterPath shape;
    shape.addRect(rect);
    this->setShape(shape);
}

bool RPZAtom::isHidden() { return this->value(_str[Hidden]).toBool(); }
void RPZAtom::setHidden(bool isHidden) { this->insert(_str[Hidden], isHidden); }

bool RPZAtom::isLocked() { return this->value(_str[Locked]).toBool(); }
void RPZAtom::setLocked(bool isLocked) { this->insert(_str[Locked], isLocked); }