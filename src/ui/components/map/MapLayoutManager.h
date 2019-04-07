#pragma once

#include <QTreeWidget>
#include <QHash>
#include <QList>
#include <QUuid>
#include <QGraphicsItem>
#include <QIcon>

#include "MapView.h"
#include "Asset.hpp"

class MapLayoutManager : public QTreeWidget{

    Q_OBJECT

    public:
        MapLayoutManager(QWidget * parent = nullptr);
        void alterTreeElements(QList<Asset> &elements, const MapView::Alteration &state);
    
    signals:
        void elementsAlterationAsked(const QList<QUuid> &elementIds, const MapView::Alteration &state);

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);

        QHash<QUuid, QTreeWidgetItem*> _treeItemsById;
        QList<QUuid> _extractIdsFromSelection() const;
        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QTreeWidgetItem* _createTreeItem(const Asset &asset);
};