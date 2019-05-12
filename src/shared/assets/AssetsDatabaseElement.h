#pragma once

#include <QString>
#include <QList>
#include <QHash>
#include <QModelIndex>
#include <QDebug>

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
            const AssetsDatabaseElement::Type &type = Folder,
            QString id = "" 
        );
        AssetsDatabaseElement();
        ~AssetsDatabaseElement();

        static AssetsDatabaseElement* fromIndex(QModelIndex index);
        static QString getIconPathForType(const AssetsDatabaseElement::Type &type);
        QString displayName();
        AssetsDatabaseElement::Type type();
        QString iconPath();
        QString path();
        QString fullPath();
        QString id();

        //prefer using rename() with AssetsDatabase
        void rename(QString &newName);

        AssetsDatabaseElement* parent();
        int row() const;
        AssetsDatabaseElement* child(int row);
        int childCount() const;
        int itemChildrenCount() const;

        QList<AssetsDatabaseElement*> childrenContainers();
        QList<AssetsDatabaseElement*> childrenItems();
        
        bool isContainer();
        bool isInternal();
        bool isRoot();
        bool isItem();
        bool isStaticContainer();
        bool isDeletable();

        //prefer using insertAsset() for parallel db insertion
        void appendChild(AssetsDatabaseElement* child);
        void unrefChild(AssetsDatabaseElement* child);
        
        AssetsDatabaseElement::Type defaultTypeOnContainerForInsert();
        
        AssetsDatabaseElement::Type getBoundStaticContainer();
        Qt::ItemFlags flags();

    protected:
        QList<AssetsDatabaseElement*> _subElements;
        AssetsDatabaseElement* _parentElement = nullptr;

    private:
        QString _name;
        AssetsDatabaseElement::Type _type = Unknown;
        QString _id = "";
        int _itemChildrenCount = 0;

        void _defineParent(AssetsDatabaseElement* parent);

        static const inline QList<AssetsDatabaseElement::Type> _containerTypes = {
            InternalContainer,
            NPC_Container, 
            FloorBrushContainer, 
            ObjectContainer,
            Folder
        };

        static const inline QList<AssetsDatabaseElement::Type> _itemTypes = {
            Player, 
            Event, 
            NPC, 
            FloorBrush,
            Object
        };

        static const inline QList<AssetsDatabaseElement::Type> _deletableItemTypes = {
            NPC, 
            FloorBrush,
            Object,
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
            { FloorBrushContainer, ":/icons/app/manager/brushes.png" },
            { Folder, ":/icons/app/manager/folder.png" }
        };
};