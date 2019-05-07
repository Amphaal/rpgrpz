#pragma once

#include <QString>
#include <QList>

class AssetsDatabaseElement {
    public:
        enum Type { 
            InternalContainer,
                Player, 
                Event, 
            NPC_Container, 
                NPC, 
            FloorBrushContainer, 
                FloorBrush,
            ObjectContainer,
                Object, 
            Folder,
            Root
        };
        
        AssetsDatabaseElement(
            const QString &name, 
            const AssetsDatabaseElement::Type &type = AssetsDatabaseElement::Type::Folder
        ) : _name(name), _type(type) { };

        AssetsDatabaseElement() : AssetsDatabaseElement("", AssetsDatabaseElement::Type::Root) { };

        ~AssetsDatabaseElement(){
            qDeleteAll(this->_subElements);
        }

        static QString getIconPathForType(const AssetsDatabaseElement::Type &type) {
            return _iconPathByElementType[type];
        }

        QString displayName() {
            return this->_name;
        }

        AssetsDatabaseElement::Type type() {
            return this->_type;
        }

        QString iconPath() {
            return getIconPathForType(this->type());
        }

        AssetsDatabaseElement* parent() {
            return this->_parentElement;
        }

        int row() const {
            if (this->_parentElement) {
                return this->_parentElement->_subElements.indexOf(const_cast<AssetsDatabaseElement*>(this));
            }

            return 0;
        }

        AssetsDatabaseElement* child(int row) {
            return this->_subElements.value(row);
        }

        int childCount() const {
            return this->_subElements.count();
        }

        bool isRoot() {
            return this->_type == AssetsDatabaseElement::Type::Root;
        }

        bool isContainer() {
            return _containerTypes.contains(this->_type);
        }

        void appendChild(AssetsDatabaseElement* child) {
            child->defineParent(this);
            this->_subElements.append(child);
        };

        void defineParent(AssetsDatabaseElement* parent) {
            this->_parentElement = parent;
        }

        Qt::ItemFlags flags() {
            switch(this->_type) {
                case InternalContainer:
                    return Qt::ItemIsEnabled;
                    break;
                case Player:
                case Event:
                    return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);
                    break;
                default:
                    if(this->isContainer()) return QFlags<Qt::ItemFlag>(Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
                    return 0;
                    break;
            }
        }

    protected:
        QList<AssetsDatabaseElement*> _subElements;
        AssetsDatabaseElement* _parentElement = nullptr;

    private:
        QString _name;
        AssetsDatabaseElement::Type _type;

        static const inline QList<AssetsDatabaseElement::Type> _containerTypes = {
            InternalContainer,
            NPC_Container, 
            FloorBrushContainer, 
            ObjectContainer,
            Folder
        };

        static const inline QHash<AssetsDatabaseElement::Type, QString> _iconPathByElementType = {
            { Player, ":/icons/app/manager/character.png" },
            { Event, ":/icons/app/manager/event.png" },
            { NPC_Container, ":/icons/app/manager/npc.png" },
            { ObjectContainer, ":/icons/app/manager/asset.png" },
            { FloorBrushContainer, ":/icons/app/manager/brushes.png" }
        };
};