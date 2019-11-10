#include "RPZAtom.h"

RPZAtom::RPZAtom() {}
RPZAtom::RPZAtom(const QVariantHash &hash) : Serializable(hash) {}
RPZAtom::RPZAtom(RPZAtom::Id id, const RPZAtom::Type &type) : Serializable(id) {
    this->_setType(type);
};
RPZAtom::RPZAtom(const RPZAtom::Type &type) : Serializable(SnowFlake::get()->nextId()) {
    this->_setType(type);  
};


RPZAtom::Category RPZAtom::category() const {
    return category(this->type());
}
RPZAtom::Category RPZAtom::category(const RPZAtom::Type &type) {
    if(_layoutAtom.contains(type)) return Category::Layout;
    return Category::Interactive;
}

int RPZAtom::staticZIndex() const {
    auto zIndex = AppContext::TOP_Z_INDEX; 
    auto type = this->type();
    return _interactiveHoveringAtoms.contains(type) ? zIndex + (int)type : zIndex;
}

bool RPZAtom::isRestrictedAtom() const {
    return _restrictedAtom.contains(this->type());
}

const QString RPZAtom::toString(const RPZAtom::Type &type, const QString &description) { 

    //default if no descriptor
    if(description.isEmpty()) {
        switch(type) {
            
            case RPZAtom::Type::Player:
                return QObject::tr("[Unpaired player token]");
            break;

            default:
                return _atomTypeToText(type);
            break;

        }
    }

    //by category, if descriptor
    switch(RPZAtom::category(type)) {
        
        case RPZAtom::Category::Interactive:
            return description;
        break;

        case RPZAtom::Category::Layout:
            return QStringLiteral(u"%1 (%2)")
                        .arg(description)
                        .arg(_atomTypeToText(type));
        break;

        default:
        break;

    }
    
    return _atomTypeToText(type);

};

const QString RPZAtom::toString() const {
    
    auto type = this->type();
    auto descriptorParam = RPZAtom::descriptorsByAtomType.value(type);
    auto descriptor = descriptorParam != RPZAtom::Parameter::Unknown ? this->metadata(descriptorParam).toString() : QString();

    return toString(type, descriptor);
    
}

const QString RPZAtom::descriptiveIconPath() const {
    return descriptiveIconPath(this->type(), this->NPCAttitude());
}

const QString RPZAtom::descriptiveIconPath(const RPZAtom::Type &type, const RPZAtom::NPCType &npcAttitude) {

    if(type == RPZAtom::Type::NPC) {
        switch(npcAttitude) {

            case RPZAtom::NPCType::Unknown:
                return QStringLiteral(u":/icons/app/attitude/unknown.png");

            case RPZAtom::NPCType::Friendly:
                return QStringLiteral(u":/icons/app/attitude/friendly.png");

            case RPZAtom::NPCType::Hostile:
                return QStringLiteral(u":/icons/app/attitude/hostile.png");
            
            case RPZAtom::NPCType::Neutral:
                return QStringLiteral(u":/icons/app/attitude/neutral.png");

        }
    }

    return RPZAtom::iconPathByAtomType.value(type);
    
}

const QString RPZAtom::_atomTypeToText(const RPZAtom::Type &type) {
    
    auto descr = atomTypeDescr.value(
        type, 
        atomTypeDescr.value(RPZAtom::Type::Undefined)
    );

    return QObject::tr(qUtf8Printable(descr));

}

QVariant RPZAtom::getDefaultValueForParam(const RPZAtom::Parameter &param) {
    return _defaultVal.value(param);
}

RPZAtom::Type RPZAtom::type() const {return (RPZAtom::Type)this->value(QStringLiteral(u"t")).toInt();}
void RPZAtom::_setType(const RPZAtom::Type &type) { this->insert(QStringLiteral(u"t"), (int)type); }
void RPZAtom::changeType(const RPZAtom::Type &type) { this->_setType(type);}

void RPZAtom::unsetMetadata(const RPZAtom::Parameter &key) {
    this->remove(_str.value(key));
}

void RPZAtom::setMetadata(const RPZAtom::Parameter &key, RPZAtom &base) {
    this->setMetadata(key, base.metadata(key));
}


void RPZAtom::setMetadata(const RPZAtom::Updates &metadata) {
    for(auto i = metadata.constBegin(); i != metadata.constEnd(); i++) {
        this->setMetadata(i.key(), i.value());
    }
}

void RPZAtom::setMetadata(const RPZAtom::Parameter &key, const QVariant &value) {
    if(value.isNull()) return this->unsetMetadata(key);
    this->insert(
        _str.value(key),
        toSerialized(key, value)
    );
}

QVariant RPZAtom::metadata(const RPZAtom::Parameter &key) const {
    auto serialiedVal = this->value(_str.value(key), getDefaultValueForParam(key));
    return fromSerialized(key, serialiedVal);
}

RPZAsset::Hash RPZAtom::assetHash() const { return this->metadata(RPZAtom::Parameter::AssetHash).toString(); }
QString RPZAtom::assetName() const { return this->metadata(RPZAtom::Parameter::AssetName).toString();}
double RPZAtom::scale() const { return this->metadata(RPZAtom::Parameter::Scale).toDouble();}
double RPZAtom::rotation() const { return this->metadata(RPZAtom::Parameter::Rotation).toDouble(); }
QString RPZAtom::text() const { return this->metadata(RPZAtom::Parameter::Text).toString(); }
int RPZAtom::textSize() const { return this->metadata(RPZAtom::Parameter::TextSize).toInt(); }
RPZAtom::Layer RPZAtom::layer() const { return this->metadata(RPZAtom::Parameter::Layer).toInt(); }
QPointF RPZAtom::pos() const { return this->metadata(RPZAtom::Parameter::Position).toPointF();}
int RPZAtom::penWidth() const { return this->metadata(RPZAtom::Parameter::PenWidth).toInt(); }
bool RPZAtom::isHidden() const { return this->metadata(RPZAtom::Parameter::Hidden).toBool(); }
bool RPZAtom::isLocked() const { return this->metadata(RPZAtom::Parameter::Locked).toBool(); }
double RPZAtom::assetScale() const { return this->metadata(RPZAtom::Parameter::AssetScale).toDouble();}
double RPZAtom::assetRotation() const { return this->metadata(RPZAtom::Parameter::AssetRotation).toDouble();}
RPZAtom::BrushType RPZAtom::brushType() const { return (RPZAtom::BrushType)this->metadata(RPZAtom::Parameter::BrushStyle).toInt(); }
int RPZAtom::brushPenWidth() const { return this->metadata(RPZAtom::Parameter::BrushPenWidth).toInt(); }
QPointF RPZAtom::shapeCenter() const { return this->metadata(RPZAtom::Parameter::ShapeCenter).toPointF(); }
const QColor RPZAtom::defaultPlayerColor() const { return this->metadata(RPZAtom::Parameter::DefaultPlayerColor).value<QColor>(); }
const RPZCharacter::Id RPZAtom::characterId() const { return this->metadata(RPZAtom::Parameter::CharacterId).toULongLong(); }
const QString RPZAtom::characterName() const { return this->metadata(RPZAtom::Parameter::CharacterName).toString(); }
const QString RPZAtom::NPCShortName() const { return this->metadata(RPZAtom::Parameter::NPCShortName).toString(); }
const RPZAtom::NPCType RPZAtom::NPCAttitude() const { return (RPZAtom::NPCType)this->metadata(RPZAtom::Parameter::NPCAttitude).toInt(); }

QPainterPath RPZAtom::shape() const {
    auto rawShape = this->metadata(RPZAtom::Parameter::Shape).toByteArray();
    return JSONSerializer::fromByteArray(rawShape);
}
void RPZAtom::setShape(const QPainterPath &path) { 
    this->setMetadata(RPZAtom::Parameter::Shape, JSONSerializer::asBase64(path));
}
void RPZAtom::setShape(const QRectF &rect) {
    QPainterPath shape;
    shape.addRect(rect);
    this->setShape(shape);
}

//
//
//

QSet<RPZAtom::Parameter> RPZAtom::customizableParams(const RPZAtom::Type &type) {
    QSet<RPZAtom::Parameter> out;
    
    switch(type) {

        case RPZAtom::Type::Drawing: {
            out.insert(RPZAtom::Parameter::PenWidth);
        }
        break;

        case RPZAtom::Type::Object: {
            out.insert(RPZAtom::Parameter::Rotation);
            out.insert(RPZAtom::Parameter::Scale);
        }
        break;

        case RPZAtom::Type::Text: {
            out.insert(RPZAtom::Parameter::TextSize);
            out.insert(RPZAtom::Parameter::Text);
            out.insert(RPZAtom::Parameter::Rotation);
        }
        break;

        case RPZAtom::Type::POI:
        case RPZAtom::Type::Event: {
            out.insert(RPZAtom::Parameter::EventShortDescription);
            out.insert(RPZAtom::Parameter::EventDescription);   
        }
        break;

        case RPZAtom::Type::Player: {
            out.insert(RPZAtom::Parameter::CharacterId);
        }
        break;

        case RPZAtom::Type::NPC: {
            out.insert(RPZAtom::Parameter::NPCAttitude);
            out.insert(RPZAtom::Parameter::NPCShortName);
            out.insert(RPZAtom::Parameter::NPCHealth);
            out.insert(RPZAtom::Parameter::MaxNPCHealth);
            out.insert(RPZAtom::Parameter::NPCDescription);
        }
        break;

        case RPZAtom::Type::Brush: {
            out.insert(RPZAtom::Parameter::AssetRotation);
            out.insert(RPZAtom::Parameter::AssetScale);
            out.insert(RPZAtom::Parameter::BrushStyle);
            out.insert(RPZAtom::Parameter::BrushPenWidth);
        }
        break;

        default:
            break;

    }

    return out;
}

QSet<RPZAtom::Parameter> RPZAtom::customizableParams() const {
    return customizableParams(this->type());
}

const QColor RPZAtom::NPCAssociatedColor() const {
    return _NPCTypeAssociatedColor.value(this->NPCAttitude());
}

bool RPZAtom::isAssetBased() const {
    return assetBasedAtom.contains(this->type());
}

QSet<RPZAtom::Parameter> RPZAtom::legalParameters() const {
    
    auto base = this->customizableParams();
    
    //basic
    base.insert(RPZAtom::Parameter::Position);
    
    //layout specific
    if(this->category() == RPZAtom::Category::Layout) {
        base.insert(RPZAtom::Parameter::Layer);
        base.insert(RPZAtom::Parameter::Hidden);
        base.insert(RPZAtom::Parameter::Locked);
    }

    if(this->isAssetBased()) {
        base.insert(RPZAtom::Parameter::Shape);
        base.insert(RPZAtom::Parameter::ShapeCenter);
        base.insert(RPZAtom::Parameter::AssetHash);
        base.insert(RPZAtom::Parameter::AssetName);
    }

    return base;

}

QSet<RPZAtom::Parameter> RPZAtom::editedMetadata() const {
    
    //existing metadata
    QSet<RPZAtom::Parameter> existing;
    for (auto i = _str.constBegin(); i != _str.constEnd(); ++i) {
        if(this->contains(i.value())) existing.insert(i.key());
    }

    return existing;

}

RPZAtom::Updates RPZAtom::editedMetadataWithValues() const {
    RPZAtom::Updates out;

    for (auto m : this->editedMetadata()) {
        out.insert(m, this->metadata(m));
    }

    return out;
}

QSet<RPZAtom::Parameter> RPZAtom::legalEditedMetadata() const {
    return this->editedMetadata().intersect(
        this->legalParameters()
    );
}


QVariantHash RPZAtom::serializeUpdates(const RPZAtom::Updates &updates) {
    QVariantHash in;
    for (auto i = updates.constBegin(); i != updates.constEnd(); ++i) {
        
        auto param = i.key();
        auto unserializedVal = i.value();
        
        in.insert(
            QString::number((int)param), 
            toSerialized(param, unserializedVal)
        );
    }
    return in;
}

RPZAtom::Updates RPZAtom::unserializeUpdates(const QVariantHash &serializedUpdates) {
    RPZAtom::Updates out;

    for (auto i = serializedUpdates.begin(); i != serializedUpdates.end(); ++i) {
        auto param = (RPZAtom::Parameter)i.key().toInt();
        auto serializedVal = i.value();

        out.insert(
            param, 
            fromSerialized(param, serializedVal)
        );
    }

    return out;
}

QVariant RPZAtom::toSerialized(const RPZAtom::Parameter &param, const QVariant &unserialized) {
    switch(param) {
        case RPZAtom::Parameter::ShapeCenter:
        case RPZAtom::Parameter::Position: {
            return JSONSerializer::fromPointF(unserialized.toPointF());
        }
        break;

        default:
            return unserialized;
    }
}

QVariant RPZAtom::fromSerialized(const RPZAtom::Parameter &param, const QVariant &serialized) {
    switch(param) {
        case RPZAtom::Parameter::ShapeCenter:
        case RPZAtom::Parameter::Position: {
            return JSONSerializer::toPointF(serialized.toList());
        }
        break;

        default:
            return serialized;
    }
}