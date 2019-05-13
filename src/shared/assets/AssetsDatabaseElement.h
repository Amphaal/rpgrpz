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
        
        static AssetsDatabaseElement* fromIndex(QModelIndex index);
        static inline QString listMimeType = "application/x-assets-db-elem-list";    

        AssetsDatabaseElement(const QString &name, AssetsDatabaseElement* parent, const AssetsDatabaseElement::Type &type = Folder, QString id = "");
        AssetsDatabaseElement();
        ~AssetsDatabaseElement();

        AssetsDatabaseElement::Type type();
        AssetsDatabaseElement::Type insertType();
        AssetsDatabaseElement::Type rootStaticContainer();
        QString iconPath();
        QString displayName();
        QString path();
        QString fullPath();
        QString id();
        Qt::ItemFlags flags();
                
        bool isContainer();
        bool isInternal();
        bool isRoot();
        bool isItem();
        bool isStaticContainer();
        bool isDeletable();

        AssetsDatabaseElement* parent();
        int row() const;
        AssetsDatabaseElement* child(int row);
        int childCount() const;
        int itemChildrenCount() const;
        QList<AssetsDatabaseElement*> childrenContainers();
        QList<AssetsDatabaseElement*> childrenItems();

        void rename(const QString &newName); //prefer using rename() with AssetsDatabase for db interaction
        void appendChild(AssetsDatabaseElement* child); //prefer using insertAsset() for parallel db insertion
        void unrefChild(AssetsDatabaseElement* child);

    protected:
        QList<AssetsDatabaseElement*> _subElements;
        AssetsDatabaseElement* _parentElement = nullptr;

    private:
        AssetsDatabaseElement::Type _type = Unknown;
        AssetsDatabaseElement::Type _insertType = Unknown;
        AssetsDatabaseElement::Type _rootStaticContainerType = Unknown;
        QString _id = "";
        QString _name = "";
        QString _path = "";
        QString _fullPath = "";
        QString _iconPath = "";
        Qt::ItemFlags _flags = 0;
        int _itemChildrenCount = 0;

        bool _isContainer = false;
        bool _isInternal = false;
        bool _isRoot = false;
        bool _isItem = false;
        bool _isStaticContainer = false;
        bool _isDeletable = false;

        void _defineParent(AssetsDatabaseElement* parent);
        void _defineFlags();
        void _definePath();
        void _defineFullPath();
        void _defineIconPath();
        void _defineInsertType();
        void _defineRootStaticContainer();
    
        void _defineIsContainer();
        void _defineIsInternal();
        void _defineIsRoot();
        void _defineIsItem();
        void _defineIsStaticContainer();
        void _defineIsDeletable();
        
        ///
        ///
        ///

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