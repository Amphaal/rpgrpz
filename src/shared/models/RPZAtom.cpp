#include "RPZAtom.h"

RPZAtom::RPZAtom() {}
RPZAtom::RPZAtom(const QVariantHash &hash) : Serializable(hash) {}
RPZAtom::RPZAtom(RPZAtomId id, const RPZAtomType &type) : Serializable(id) {
    this->_setType(type);
};
RPZAtom::RPZAtom(const RPZAtomType &type) : Serializable(SnowFlake::get()->nextId()) {
    this->_setType(type);  
};

bool RPZAtom::isLayoutAtom() {
    return layoutAtom.contains(this->type());
}

const QString RPZAtom::toString(const RPZAtomType &type, const QString &assetName) { 

    auto out = atomTypeToText(type);

    if(!assetName.isEmpty()) {
		
		return QStringLiteral(u"%1 (%2)")
					.arg(assetName)
					.arg(out);
    }

    return out;
    
};

const QString RPZAtom::toString() const {
    return toString(this->type(), this->assetName());
}

const QString RPZAtom::atomTypeToText(const RPZAtomType &type) {
    switch(type) {
        case RPZAtomType::Drawing:
            return QObject::tr("Drawing");
        case RPZAtomType::Text:
            return QObject::tr("Text");
        case RPZAtomType::Object:
            return QObject::tr("Object");
        case RPZAtomType::Brush:
            return QObject::tr("Brush");
        default:
            return QObject::tr("Atom");
    }
}

QVariant RPZAtom::getDefaultValueForParam(const AtomParameter &param) {
    return _defaultVal.value(param);
}

RPZAtomType RPZAtom::type() const {return (RPZAtomType)this->value(QStringLiteral(u"t")).toInt();}
void RPZAtom::_setType(const RPZAtomType &type) { this->insert(QStringLiteral(u"t"), (int)type); }
void RPZAtom::changeType(const RPZAtomType &type) { this->_setType(type);}

void RPZAtom::unsetMetadata(const AtomParameter &key) {
    this->remove(_str.value(key));
}

void RPZAtom::setMetadata(const AtomParameter &key, RPZAtom &base) {
    this->setMetadata(key, base.metadata(key));
}


void RPZAtom::setMetadata(const AtomUpdates &metadata) {
    for(auto i = metadata.constBegin(); i != metadata.constEnd(); i++) {
        this->setMetadata(i.key(), i.value());
    }
}

void RPZAtom::setMetadata(const AtomParameter &key, const QVariant &value) {
    if(value.isNull()) return this->unsetMetadata(key);
    this->insert(
        _str.value(key),
        JSONSerializer::toSerialized(key, value)
    );
}

QVariant RPZAtom::metadata(const AtomParameter &key) const {
    auto serialiedVal = this->value(_str.value(key), getDefaultValueForParam(key));
    return JSONSerializer::fromSerialized(key, serialiedVal);
}

RPZAssetHash RPZAtom::assetHash() const { return this->metadata(AtomParameter::AssetHash).toString(); }
QString RPZAtom::assetName() const { return this->metadata(AtomParameter::AssetName).toString();}
double RPZAtom::scale() const { return this->metadata(AtomParameter::Scale).toDouble();}
double RPZAtom::rotation() const { return this->metadata(AtomParameter::Rotation).toDouble(); }
QString RPZAtom::text() const { return this->metadata(AtomParameter::Text).toString(); }
int RPZAtom::textSize() const { return this->metadata(AtomParameter::TextSize).toInt(); }
int RPZAtom::layer() const { return this->metadata(AtomParameter::Layer).toInt(); }
QPointF RPZAtom::pos() const { return this->metadata(AtomParameter::Position).toPointF();}
int RPZAtom::penWidth() const { return this->metadata(AtomParameter::PenWidth).toInt(); }
bool RPZAtom::isHidden() const { return this->metadata(AtomParameter::Hidden).toBool(); }
bool RPZAtom::isLocked() const { return this->metadata(AtomParameter::Locked).toBool(); }
double RPZAtom::assetScale() const { return this->metadata(AtomParameter::AssetScale).toDouble();}
double RPZAtom::assetRotation() const { return this->metadata(AtomParameter::AssetRotation).toDouble();}
BrushType RPZAtom::brushType() const { return (BrushType)this->metadata(AtomParameter::BrushStyle).toInt(); }
int RPZAtom::brushPenWidth() const { return this->metadata(AtomParameter::BrushPenWidth).toInt(); }
QPointF RPZAtom::shapeCenter() const { return this->metadata(AtomParameter::ShapeCenter).toPointF(); }

QPainterPath RPZAtom::shape() const {
    auto rawShape = this->metadata(AtomParameter::Shape).toByteArray();
    return JSONSerializer::fromByteArray(rawShape);
}
void RPZAtom::setShape(const QPainterPath &path) { 
    this->setMetadata(AtomParameter::Shape, JSONSerializer::asBase64(path));
    //this->setMetadata(AtomParameter::ShapeCenter, path.boundingRect().center());
}
void RPZAtom::setShape(const QRectF &rect) {
    QPainterPath shape;
    shape.addRect(rect);
    this->setShape(shape);
}

//
//
//

QSet<AtomParameter> RPZAtom::customizableParams(const RPZAtomType &type) {
    QSet<AtomParameter> out;
    
    switch(type) {

        case RPZAtomType::Drawing: {
            out.insert(AtomParameter::PenWidth);
        }
        break;

        case RPZAtomType::Object: {
            out.insert(AtomParameter::Rotation);
            out.insert(AtomParameter::Scale);
        }
        break;

        case RPZAtomType::Text: {
            out.insert(AtomParameter::TextSize);
            out.insert(AtomParameter::Text);
            out.insert(AtomParameter::Rotation);
        }
        break;

        case RPZAtomType::Brush: {
            out.insert(AtomParameter::AssetRotation);
            out.insert(AtomParameter::AssetScale);
            out.insert(AtomParameter::BrushStyle);
            out.insert(AtomParameter::BrushPenWidth);
        }
        break;

        default:
            break;

    }

    return out;
}

QSet<AtomParameter> RPZAtom::customizableParams() const {
    return customizableParams(this->type());
}

QSet<AtomParameter> RPZAtom::legalParameters() const {
    
    auto base = this->customizableParams();
    
    base.insert(AtomParameter::Position);
    base.insert(AtomParameter::Layer);
    base.insert(AtomParameter::Hidden);
    base.insert(AtomParameter::Locked);
    base.insert(AtomParameter::Shape);
    base.insert(AtomParameter::ShapeCenter);
    
    switch(this->type()) {

        case RPZAtomType::Text: {
            base.insert(AtomParameter::Text);
        }
        break;

        case RPZAtomType::Object: {
            base.insert(AtomParameter::AssetHash);
            base.insert(AtomParameter::AssetName);
        }
        break;

        case RPZAtomType::Brush: {
            base.insert(AtomParameter::AssetHash);
            base.insert(AtomParameter::AssetName);
            base.insert(AtomParameter::BrushStyle);
            base.insert(AtomParameter::BrushPenWidth);
        }
        break;

        default:
            break;

    }
    
    return base;
}

QSet<AtomParameter> RPZAtom::editedMetadata() const {
    
    //existing metadata
    QSet<AtomParameter> existing;
    for (auto i = _str.constBegin(); i != _str.constEnd(); ++i) {
        if(this->contains(i.value())) existing.insert(i.key());
    }

    return existing;

}

AtomUpdates RPZAtom::editedMetadataWithValues() const {
    AtomUpdates out;

    for (auto m : this->editedMetadata()) {
        out.insert(m, this->metadata(m));
    }

    return out;
}

QSet<AtomParameter> RPZAtom::legalEditedMetadata() const {
    return this->editedMetadata().intersect(
        this->legalParameters()
    );
}