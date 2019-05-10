#pragma once

#include <QString>
#include <QList>
#include <QHash>

class AssetsDatabaseElement {
    public:
        enum Type { 
            Unknown = 0,
            Root = 1,
            Folder = 2,
            //
            InternalContainer,
                Player, 
                Event, 
            NPC_Container, 
                NPC, 
            FloorBrushContainer, 
                FloorBrush,
            ObjectContainer,
                Object
        };
        
        static inline QString listMimeType = "application/x-assets-db-elem-list";

        AssetsDatabaseElement(
            const QString &name, 
            const AssetsDatabaseElement::Type &type = AssetsDatabaseElement::Type::Folder
        );
        AssetsDatabaseElement();
        ~AssetsDatabaseElement();

        static QString getIconPathForType(const AssetsDatabaseElement::Type &type);
        QString displayName();
        AssetsDatabaseElement::Type type();
        QString iconPath();
        QString path();

        AssetsDatabaseElement* parent();
        int row() const;
        AssetsDatabaseElement* child(int row);
        int childCount() const;
        
        bool isContainer();
        bool isInternal();
        bool isRoot();

        //prefer using insertAsset() for parallel db insertion
        void appendChild(AssetsDatabaseElement* child);;
        AssetsDatabaseElement::Type defaultTypeOnContainerForInsert();
        void defineParent(AssetsDatabaseElement* parent);
        AssetsDatabaseElement::Type getBoundStaticContainer();
        Qt::ItemFlags flags();

    protected:
        QList<AssetsDatabaseElement*> _subElements;
        AssetsDatabaseElement* _parentElement = nullptr;

    private:
        QString _name;
        AssetsDatabaseElement::Type _type = Unknown;

        static const inline QList<AssetsDatabaseElement::Type> _containerTypes = {
            InternalContainer,
            NPC_Container, 
            FloorBrushContainer, 
            ObjectContainer,
            Folder
        };

        static const inline QList<AssetsDatabaseElement::Type> _staticContainerTypes = {
            InternalContainer,
            NPC_Container, 
            FloorBrushContainer, 
            ObjectContainer
        };

        static const inline QHash<AssetsDatabaseElement::Type, QString> _iconPathByElementType = {
            { Player, ":/icons/app/manager/character.png" },
            { Event, ":/icons/app/manager/event.png" },
            { NPC_Container, ":/icons/app/manager/npc.png" },
            { ObjectContainer, ":/icons/app/manager/asset.png" },
            { FloorBrushContainer, ":/icons/app/manager/brushes.png" }
        };
};