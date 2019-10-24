#pragma once

#include <QString>
#include <QList>
#include <QHash>
#include <QModelIndex>
#include <QDebug>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/toy/RPZAsset.hpp"

#include "src/shared/database/AssetsDatabase.h"

class AssetsTreeViewItem : public QObject {

    Q_OBJECT

    public:
        enum class Type { 
            T_Unknown = 0,
            Root = 100,
            Folder = 101,
            InternalContainer = 201,
                Event = (int)RPZAtomType::Event,
                FreeDraw = (int)RPZAtomType::Drawing,
                Text = (int)RPZAtomType::Text,
            NPC_Container = (int)AssetsDatabase::StorageContainer::NPC,
                NPC = (int)RPZAtomType::NPC,
            BackgroundContainer = (int)AssetsDatabase::StorageContainer::Background,
                Background = (int)RPZAtomType::Background,
            FloorBrushContainer = (int)AssetsDatabase::StorageContainer::FloorBrush,
                FloorBrush = (int)RPZAtomType::Brush,
            ObjectContainer = (int)AssetsDatabase::StorageContainer::Object,
                Object = (int)RPZAtomType::Object,
            DownloadedContainer = (int)AssetsDatabase::StorageContainer::Downloaded,
                Downloaded = 650
        };
        //TODO DELETE bound types, add more constructors ?
        Q_ENUM(Type)
        
        static QList<AssetsTreeViewItem::Type> staticContainerTypes();
        static QList<AssetsTreeViewItem::Type> movableStaticContainerTypes();

        static AssetsTreeViewItem* fromIndex(const QModelIndex &index);
        static inline const QString listMimeType = QStringLiteral(u"application/x-assets-db-elem-list");

        AssetsTreeViewItem(AssetsTreeViewItem* parent, const QString &folderName);
        AssetsTreeViewItem(AssetsTreeViewItem* parent, const AssetsTreeViewItem::Type &type);
        AssetsTreeViewItem(AssetsTreeViewItem* parent, const RPZAsset* assetMetadata);
        AssetsTreeViewItem();
        ~AssetsTreeViewItem();

        const AssetsTreeViewItem::Type type() const;
        const AssetsTreeViewItem::Type insertType() const;
        const AssetsTreeViewItem::Type rootStaticContainer() const;
        const QString iconPath() const;
        const QString displayName() const;
        const QString path() const;
        const Qt::ItemFlags flags() const;
        const RPZAsset* asset() const;
                
        bool isContainer() const;
        bool isInternal() const;
        bool isRoot() const;
        bool isIdentifiable() const;
        bool isDeletable() const;

        AssetsTreeViewItem* parent();
        int row() const;
        AssetsTreeViewItem* child(int row);
        int childCount() const;
        int itemChildrenCount() const;
        QList<AssetsTreeViewItem*> childrenContainers();
        QList<AssetsTreeViewItem*> childrenItems();
        
        bool isAcceptableNameChange(QString &newName); //sanitize and check if the name change is OK
        void rename(const QString &newName); //prefer using rename() with AssetsDatabase for db interaction
        void appendChild(AssetsTreeViewItem* child); //prefer using insertAsset() with AssetsDatabase for parallel db insertion

        bool contains(AssetsTreeViewItem* toCheck, AssetsTreeViewItem* toBeChecked = nullptr);
        bool containsAny(const QList<AssetsTreeViewItem*> toCheck);

        static const QString typeDescription(const AssetsTreeViewItem::Type &type);
        static const QList<AssetsTreeViewItem::Type> internalItemTypes();
        static AssetsTreeViewItem::Type pathChunktoType(const QString &chunk); //interpret path element as corresponding type
        static QList<QString> pathAsList(const QString &path); //returns elements of a path

    protected:
        AssetsTreeViewItem(AssetsTreeViewItem* parent, const AssetsTreeViewItem::Type &type, const QString &name);

        QList<AssetsTreeViewItem*> _subElements;
        AssetsTreeViewItem* _parentElement = nullptr;

       //sort items by path length (number of slashes)
        static void sortByPathLengthDesc(QList<AssetsTreeViewItem*> &listToSort);

        //returns a list of single elements by node path
        static QSet<AssetsTreeViewItem*> filterTopMostOnly(QList<AssetsTreeViewItem*> elemsToFilter);

        void unrefChild(AssetsTreeViewItem* child);

    private:
        AssetsTreeViewItem::Type _type = AssetsTreeViewItem::Type::T_Unknown;
        AssetsTreeViewItem::Type _insertType = AssetsTreeViewItem::Type::T_Unknown;
        AssetsTreeViewItem::Type _rootStaticContainerType = AssetsTreeViewItem::Type::T_Unknown;
        RPZAtomType _atomType = RPZAtomType::Undefined;
        RPZAssetHash _hash;
        QString _name;
        QString _path;
        QString _fullPath;
        QString _iconPath;
        Qt::ItemFlags _flags = 0;
        int _itemChildrenCount = 0;

        bool _isContainer = false;
        bool _isInternal = false;
        bool _isRoot = false;
        bool _isIdentifiable = false;
        bool _isStaticContainer = false;
        bool _isDeletable = false;

        void _defineParent(AssetsTreeViewItem* parent);
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
        
        void _setType(const AssetsTreeViewItem::Type &type);
        static void _resetSubjacentItemsType(const AssetsTreeViewItem::Type &replacingType, AssetsTreeViewItem* target); //recursive

        ///
        ///
        //

        static const inline QList<AssetsTreeViewItem::Type> _staticContainerTypes = {
            AssetsTreeViewItem::Type::InternalContainer,
            AssetsTreeViewItem::Type::NPC_Container, 
            AssetsTreeViewItem::Type::FloorBrushContainer, 
            AssetsTreeViewItem::Type::BackgroundContainer,
            AssetsTreeViewItem::Type::ObjectContainer,
            AssetsTreeViewItem::Type::DownloadedContainer
        };

        static const inline QList<AssetsTreeViewItem::Type> _movableStaticContainerTypes = {
            AssetsTreeViewItem::Type::FloorBrushContainer, 
            AssetsTreeViewItem::Type::ObjectContainer
        };

        static const inline QList<AssetsTreeViewItem::Type> _itemTypes = {
            AssetsTreeViewItem::Type::NPC, 
            AssetsTreeViewItem::Type::FloorBrush,
            AssetsTreeViewItem::Type::Object,
            AssetsTreeViewItem::Type::Downloaded,
            AssetsTreeViewItem::Type::Background
        };

        static const inline QList<AssetsTreeViewItem::Type> _internalItemsTypes = {
            AssetsTreeViewItem::Type::Event,
            AssetsTreeViewItem::Type::FreeDraw,
            AssetsTreeViewItem::Type::Text
        };

        static const inline QHash<AssetsTreeViewItem::Type, QString> _iconPathByElementType = {
            { AssetsTreeViewItem::Type::Event, ":/icons/app/manager/event.png" },
            { AssetsTreeViewItem::Type::NPC_Container, ":/icons/app/manager/npc.png" },
            { AssetsTreeViewItem::Type::ObjectContainer, ":/icons/app/manager/asset.png" },
            { AssetsTreeViewItem::Type::FloorBrushContainer, ":/icons/app/manager/brushes.png" },
            { AssetsTreeViewItem::Type::FreeDraw, ":/icons/app/tools/pen.png" },
            { AssetsTreeViewItem::Type::Text, ":/icons/app/tools/text.png" },
            { AssetsTreeViewItem::Type::Folder, ":/icons/app/manager/folder.png" }, 
            { AssetsTreeViewItem::Type::InternalContainer, ":/icons/app/manager/internal.png" },
            { AssetsTreeViewItem::Type::DownloadedContainer, ":/icons/app/manager/downloaded.png" },
            { AssetsTreeViewItem::Type::BackgroundContainer, ":/icons/app/manager/background.png" }
        };

        static const inline QHash<AssetsTreeViewItem::Type, QString> _typeDescriptions = {
            { AssetsTreeViewItem::Type::InternalContainer, QT_TR_NOOP("Internal") },
            { AssetsTreeViewItem::Type::Event, QT_TR_NOOP("Event") },
            { AssetsTreeViewItem::Type::FreeDraw, QT_TR_NOOP("Drawing") },
            { AssetsTreeViewItem::Type::Text, QT_TR_NOOP("Text") },
            { AssetsTreeViewItem::Type::NPC_Container, QT_TR_NOOP("NPC / Portraits") },
            { AssetsTreeViewItem::Type::ObjectContainer, QT_TR_NOOP("Objects") },
            { AssetsTreeViewItem::Type::FloorBrushContainer, QT_TR_NOOP("Brush") },
            { AssetsTreeViewItem::Type::DownloadedContainer, QT_TR_NOOP("Downloaded") },
            { AssetsTreeViewItem::Type::BackgroundContainer, QT_TR_NOOP("Landscapes") }
        };

};
inline uint qHash(const AssetsTreeViewItem::Type &key, uint seed = 0) {return uint(key) ^ seed;}
