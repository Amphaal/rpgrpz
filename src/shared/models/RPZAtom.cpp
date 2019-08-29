#include "RPZAtom.h"

RPZAtom::RPZAtom() {}
RPZAtom::RPZAtom(const QVariantHash &hash) : Ownable(hash) {}
RPZAtom::RPZAtom(RPZAtomId id, const AtomType &type, const RPZUser &owner) : Ownable(id, owner) {
    this->_setType(type);
};
RPZAtom::RPZAtom(const AtomType &type) : Ownable(SnowFlake::get()->nextId()) {
    this->_setType(type);  
};

//overrides descriptor
QString RPZAtom::descriptor() const { 

    //displays asset name
    auto asname = this->assetName();
    if(!asname.isEmpty()) {
		auto descriptor = this->_defaultDescriptor();
		return QString("%1 (%2)")
					.arg(asname)
					.arg(descriptor);
    }

    return this->_defaultDescriptor();
};

QString RPZAtom::_defaultDescriptor() const {
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

AtomType RPZAtom::type() const {return (AtomType)this->value("t").toInt();}
void RPZAtom::_setType(const AtomType &type) { this->insert("t", (int)type); }
void RPZAtom::changeType(const AtomType &type) { this->_setType(type);}

void RPZAtom::unsetMetadata(const AtomParameter &key) {
    this->remove(_str[key]);
}

void RPZAtom::setMetadata(const AtomParameter &key, RPZAtom &base, bool autoRemove) {
    this->setMetadata(key, base.metadata(key), autoRemove);
}


void RPZAtom::setMetadata(const AtomUpdates &metadata, bool autoRemove) {
    for(auto i = metadata.constBegin(); i != metadata.constEnd(); i++) {
        this->setMetadata(i.key(), i.value(), autoRemove);
    }
}

void RPZAtom::setMetadata(const AtomParameter &key, const QVariant &value, bool autoRemove) {

    if(value.isNull() && autoRemove) return this->unsetMetadata(key);

    switch(key) {

        case AtomParameter::ShapeCenter:
        case AtomParameter::Position: {
            auto pos = value.toPointF();
            QVariantList a { pos.x(), pos.y() };
            this->insert(_str[key], a);
        }
        break;

        default:
            this->insert(_str[key], value);
            
    }
    
}

QVariant RPZAtom::metadata(const AtomParameter &key) const {
    
    switch(key) {

        case AtomParameter::ShapeCenter:
        case AtomParameter::Position: {
            auto posArr = this->value(_str[key]).toList();
            return posArr.isEmpty() ? QPointF() : QPointF(posArr[0].toReal(), posArr[1].toReal());
        }
        break;

        default:
            return this->value(_str[key], _defaultVal[key]);

    }
}

RPZAssetHash RPZAtom::assetId() const { return this->metadata(AtomParameter::AssetId).toString(); }
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
void RPZAtom::setShape(const QPainterPath &path) { this->setMetadata(AtomParameter::Shape, JSONSerializer::asBase64(path)); }
void RPZAtom::setShape(const QRectF &rect) {
    QPainterPath shape;
    shape.addRect(rect);
    this->setShape(shape);
}

//
//
//

QSet<AtomParameter> RPZAtom::customizableParams() const {
    
    QSet<AtomParameter> out;
    
    switch(this->type()) {

        case AtomType::Drawing: {
            out.insert(AtomParameter::PenWidth);
        }
        break;

        case AtomType::Object: {
            out.insert(AtomParameter::Rotation);
            out.insert(AtomParameter::Scale);
        }
        break;

        case AtomType::Text: {
            out.insert(AtomParameter::TextSize);
            out.insert(AtomParameter::Text);
            out.insert(AtomParameter::Rotation);
        }
        break;

        case AtomType::Brush: {
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

QSet<AtomParameter> RPZAtom::legalParameters() const {
    
    auto base = this->customizableParams();
    
    base.insert(AtomParameter::Position);
    base.insert(AtomParameter::Layer);
    base.insert(AtomParameter::Hidden);
    base.insert(AtomParameter::Locked);
    base.insert(AtomParameter::Shape);
    base.insert(AtomParameter::ShapeCenter);
    
    switch(this->type()) {

        case AtomType::Text: {
            base.insert(AtomParameter::Text);
        }
        break;

        case AtomType::Object: {
            base.insert(AtomParameter::AssetId);
            base.insert(AtomParameter::AssetName);
        }
        break;

        case AtomType::Brush: {
            base.insert(AtomParameter::AssetId);
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

Q_DECLARE_METATYPE(RPZAtom*)