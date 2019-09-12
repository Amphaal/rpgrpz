#pragma once

#include <QString>
#include <QList>
#include <QHash>
#include <QModelIndex>
#include <QDebug>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/RPZToyMetadata.h"

class AssetsDatabaseElement {
    
    public:
        enum Type { 
            T_Unknown = 0,
            Root = 100,
            Folder = 101,
            InternalContainer = 201,
                Player = 250,
                Event = 251,
                FreeDraw = 252,
                Text = 253,
            NPC_Container = 301,
                NPC = 350,
            FloorBrushContainer = 401,
                FloorBrush = 450,
            ObjectContainer = 501,
                Object = 550,
            DownloadedContainer = 601,
                Downloaded = 650 
        };
        
        static AssetsDatabaseElement* fromIndex(const QModelIndex &index);
        static inline const QString listMimeType = "application/x-assets-db-elem-list";

        static QList<AssetsDatabaseElement::Type> staticContainerTypes();
        static QList<AssetsDatabaseElement::Type> internalItemTypes();
        static QString typeDescription(AssetsDatabaseElement::Type &type);

        AssetsDatabaseElement(const QString &name, AssetsDatabaseElement* parent, const AssetsDatabaseElement::Type &type = Folder);
        AssetsDatabaseElement(const RPZToyMetadata &assetMetadata);
        AssetsDatabaseElement();
        ~AssetsDatabaseElement();

        AssetsDatabaseElement::Type type() const;
        AtomType atomType() const;
        AssetsDatabaseElement::Type insertType() const;
        AssetsDatabaseElement::Type rootStaticContainer() const;
        QString iconPath() const;
        QString displayName() const;
        QString path() const;
        QString fullPath() const;
        RPZAssetHash id() const;
        Qt::ItemFlags flags() const;
        RPZToyMetadata toyMetadata() const;

        //sanitize and check if the name change is OK
        bool isAcceptableNameChange(QString &newName);
                
        bool isContainer() const;
        bool isInternal() const;
        bool isRoot() const;
        bool isIdentifiable() const;
        bool isStaticContainer() const;
        bool isDeletable() const;

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

        
        //returns a list of single elements by node path
        static QSet<AssetsDatabaseElement*> filterTopMostOnly(QList<AssetsDatabaseElement*> elemsToFilter);

        //returns elements of a path
        static QList<QString> pathAsList(const QString &path);

        //interpret path element as corresponding type
        static AssetsDatabaseElement::Type pathChunktoType(const QString &chunk);

        //sort items by path length (number of slashes)
        static void sortByPathLengthDesc(QList<AssetsDatabaseElement*> &listToSort);

    protected:
        QList<AssetsDatabaseElement*> _subElements;
        AssetsDatabaseElement* _parentElement = nullptr;

    private:
        AssetsDatabaseElement::Type _type = T_Unknown;
        AssetsDatabaseElement::Type _insertType = T_Unknown;
        AssetsDatabaseElement::Type _rootStaticContainerType = T_Unknown;
        AtomType _atomType = AtomType::Undefined;
        RPZAssetHash _id = "";
        QString _name = "";
        QString _path = "";
        QString _fullPath = "";
        QString _iconPath = "";
        Qt::ItemFlags _flags = 0;
        int _itemChildrenCount = 0;
        RPZToyMetadata _toyMetadata;

        bool _isContainer = false;
        bool _isInternal = false;
        bool _isRoot = false;
        bool _isIdentifiable = false;
        bool _isStaticContainer = false;
        bool _isDeletable = false;

        void _defineAtomType();
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
        void _defineIsIdentifiable();
        void _defineIsStaticContainer();
        void _defineIsDeletable();
        
        void _setType(const AssetsDatabaseElement::Type &type);
        static void _resetSubjacentItemsType(const AssetsDatabaseElement::Type &replacingType, AssetsDatabaseElement* target); //recursive

        ///
        ///
        //

        static const inline QList<AssetsDatabaseElement::Type> _containerTypes = {
            InternalContainer,
            NPC_Container, 
            FloorBrushContainer, 
            ObjectContainer,
            Folder,
            DownloadedContainer
        };

        static const inline QList<AssetsDatabaseElement::Type> _itemTypes = {
            NPC, 
            FloorBrush,
            Object,
            Downloaded
        };

        static const inline QList<AssetsDatabaseElement::Type> _deletableItemTypes = {
            NPC, 
            FloorBrush,
            Object,
            Folder,
            Downloaded
        };

        static const inline QList<AssetsDatabaseElement::Type> _staticContainerTypes = {
            InternalContainer,
            NPC_Container, 
            FloorBrushContainer, 
            ObjectContainer,
            DownloadedContainer
        };

        static const inline QList<AssetsDatabaseElement::Type> _internalItemsTypes = {
            Player, 
            Event,
            FreeDraw,
            Text
        };

        static const inline QHash<AssetsDatabaseElement::Type, QString> _iconPathByElementType = {
            { Player, ":/icons/app/manager/character.png" },
            { Event, ":/icons/app/manager/event.png" },
            { NPC_Container, ":/icons/app/manager/npc.png" },
            { ObjectContainer, ":/icons/app/manager/asset.png" },
            { FloorBrushContainer, ":/icons/app/manager/brushes.png" },
            { FreeDraw, ":/icons/app/tools/pen.png" },
            { Text, ":/icons/app/tools/text.png" },
            { Folder, ":/icons/app/manager/folder.png" }, 
            { InternalContainer, ":/icons/app/manager/internal.png" },
            { DownloadedContainer, ":/icons/app/manager/downloaded.png" }
        };

        static const inline QHash<AssetsDatabaseElement::Type, QString> _typeDescriptions = {
            { InternalContainer, "Interne" },
            { Player, "Joueur" },
            { Event, "Evenement" },
            { FreeDraw, "Dessin" },
            { Text, "Texte" },
            { NPC_Container, "PNJ" },
            { ObjectContainer, "Objets" },
            { FloorBrushContainer, "Terrains" },
            { DownloadedContainer, "Téléchargés" }
        };

};