#pragma once

#include <QString>
#include <QList>
#include <QHash>
#include <QModelIndex>
#include <QDebug>

#include "src/shared/models/RPZAtom.h"
#include "src/shared/models/toy/RPZAsset.hpp"

#include "src/shared/database/AssetsDatabase.h"

class ToysTreeViewItem : private QObject {

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
        Q_ENUM(Type)
        
        static QList<ToysTreeViewItem::Type> staticContainerTypes();
        static QList<ToysTreeViewItem::Type> movableStaticContainerTypes();

        static ToysTreeViewItem* fromIndex(const QModelIndex &index);
        static inline const QString listMimeType = QStringLiteral(u"application/x-assets-db-elem-list");

        ToysTreeViewItem(ToysTreeViewItem* ancestor, const QString &folderName);
        ToysTreeViewItem(ToysTreeViewItem* ancestor, const ToysTreeViewItem::Type &type);
        ToysTreeViewItem(ToysTreeViewItem* ancestor, const RPZAsset* assetMetadata);
        ToysTreeViewItem();
        ~ToysTreeViewItem();

        const ToysTreeViewItem::Type type() const;
        const ToysTreeViewItem::Type insertType() const;
        const ToysTreeViewItem::Type rootStaticContainer() const;
        const QString iconPath() const;
        const QString displayName() const;
        const QString path() const;
        const QString fullPath() const;
        const Qt::ItemFlags flags() const;
        const RPZAsset* asset() const;
                
        bool isContainer() const;
        bool isInternal() const;
        bool isRoot() const;
        bool isIdentifiable() const;
        bool isDeletable() const;

        ToysTreeViewItem* ancestor();
        int row() const;
        ToysTreeViewItem* subItem(int row);
        int subItemCount() const;
        int toySubItemCount() const;
        QList<ToysTreeViewItem*> containerSubItems();
        QList<ToysTreeViewItem*> toySubItems();
        
        bool isAcceptableNameChange(QString &newName); //sanitize and check if the name change is OK
        void rename(const QString &newName); //prefer using rename() with AssetsDatabase for db interaction
        void appendSubItem(ToysTreeViewItem* subItem); //prefer using insertAsset() with AssetsDatabase for parallel db insertion

        bool contains(ToysTreeViewItem* toCheck, ToysTreeViewItem* toBeChecked = nullptr);
        bool containsAny(const QList<ToysTreeViewItem*> toCheck);

        static const QString typeDescription(const ToysTreeViewItem::Type &type);
        static const QList<ToysTreeViewItem::Type> internalItemTypes();
        static ToysTreeViewItem::Type pathChunktoType(const QString &chunk); //interpret path element as corresponding type
        static QList<QString> pathAsList(const QString &path); //returns elements of a path

        friend QDebug operator<<(QDebug debug, const ToysTreeViewItem &c);

    protected:
        ToysTreeViewItem(ToysTreeViewItem* ancestor, const ToysTreeViewItem::Type &type, const QString &name);

        QList<ToysTreeViewItem*> _subItems;
        ToysTreeViewItem* _ancestor = nullptr;

       //sort items by path length (number of slashes)
        static void sortByPathLengthDesc(QList<ToysTreeViewItem*> &listToSort);

        //returns a list of single elements by node path
        static QSet<ToysTreeViewItem*> filterTopMostOnly(QList<ToysTreeViewItem*> elemsToFilter);

        void unrefSubItem(ToysTreeViewItem* subItem);

    private:
        ToysTreeViewItem::Type _type = ToysTreeViewItem::Type::T_Unknown;
        ToysTreeViewItem::Type _insertType = ToysTreeViewItem::Type::T_Unknown;
        ToysTreeViewItem::Type _rootStaticContainerType = ToysTreeViewItem::Type::T_Unknown;
        RPZAtomType _atomType = RPZAtomType::Undefined;
        RPZAssetHash _hash;
        QString _name;
        QString _path;
        QString _fullPath;
        QString _iconPath;
        Qt::ItemFlags _flags = 0;
        int _toySubItemCount = 0;

        bool _isContainer = false;
        bool _isInternal = false;
        bool _isRoot = false;
        bool _isIdentifiable = false;
        bool _isStaticContainer = false;
        bool _isDeletable = false;

        void _defineAncestor(ToysTreeViewItem* ancestor);
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
        
        void _setType(const ToysTreeViewItem::Type &type);
        static void _resetSubjacentItemsType(const ToysTreeViewItem::Type &replacingType, ToysTreeViewItem* target); //recursive

        ///
        ///
        //

        static const inline QList<ToysTreeViewItem::Type> _staticContainerTypes = {
            ToysTreeViewItem::Type::InternalContainer,
            ToysTreeViewItem::Type::NPC_Container, 
            ToysTreeViewItem::Type::FloorBrushContainer, 
            ToysTreeViewItem::Type::BackgroundContainer,
            ToysTreeViewItem::Type::ObjectContainer,
            ToysTreeViewItem::Type::DownloadedContainer
        };

        static const inline QList<ToysTreeViewItem::Type> _movableStaticContainerTypes = {
            ToysTreeViewItem::Type::FloorBrushContainer, 
            ToysTreeViewItem::Type::ObjectContainer
        };

        static const inline QList<ToysTreeViewItem::Type> _itemTypes = {
            ToysTreeViewItem::Type::NPC, 
            ToysTreeViewItem::Type::FloorBrush,
            ToysTreeViewItem::Type::Object,
            ToysTreeViewItem::Type::Downloaded,
            ToysTreeViewItem::Type::Background
        };

        static const inline QList<ToysTreeViewItem::Type> _internalItemsTypes = {
            ToysTreeViewItem::Type::Event,
            ToysTreeViewItem::Type::FreeDraw,
            ToysTreeViewItem::Type::Text
        };

        static const inline QHash<ToysTreeViewItem::Type, QString> _iconPathByElementType = {
            { ToysTreeViewItem::Type::Event, ":/icons/app/manager/event.png" },
            { ToysTreeViewItem::Type::NPC_Container, ":/icons/app/manager/npc.png" },
            { ToysTreeViewItem::Type::ObjectContainer, ":/icons/app/manager/asset.png" },
            { ToysTreeViewItem::Type::FloorBrushContainer, ":/icons/app/manager/brushes.png" },
            { ToysTreeViewItem::Type::FreeDraw, ":/icons/app/tools/pen.png" },
            { ToysTreeViewItem::Type::Text, ":/icons/app/tools/text.png" },
            { ToysTreeViewItem::Type::Folder, ":/icons/app/manager/folder.png" }, 
            { ToysTreeViewItem::Type::InternalContainer, ":/icons/app/manager/internal.png" },
            { ToysTreeViewItem::Type::DownloadedContainer, ":/icons/app/manager/downloaded.png" },
            { ToysTreeViewItem::Type::BackgroundContainer, ":/icons/app/manager/background.png" }
        };

        static const inline QHash<ToysTreeViewItem::Type, QString> _typeDescriptions = {
            { ToysTreeViewItem::Type::InternalContainer, QT_TR_NOOP("Internal") },
            { ToysTreeViewItem::Type::Event, QT_TR_NOOP("Event") },
            { ToysTreeViewItem::Type::FreeDraw, QT_TR_NOOP("Drawing") },
            { ToysTreeViewItem::Type::Text, QT_TR_NOOP("Text") },
            { ToysTreeViewItem::Type::NPC_Container, QT_TR_NOOP("NPC / Portraits") },
            { ToysTreeViewItem::Type::ObjectContainer, QT_TR_NOOP("Objects") },
            { ToysTreeViewItem::Type::FloorBrushContainer, QT_TR_NOOP("Brush") },
            { ToysTreeViewItem::Type::DownloadedContainer, QT_TR_NOOP("Downloaded") },
            { ToysTreeViewItem::Type::BackgroundContainer, QT_TR_NOOP("Landscapes") }
        };

};
inline uint qHash(const ToysTreeViewItem::Type &key, uint seed = 0) {return uint(key) ^ seed;}
