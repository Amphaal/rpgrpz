#pragma once

#include <QTreeWidget>
#include <QHash>
#include <QList>
#include <QUuid>
#include <QGraphicsItem>
#include <QIcon>

#include "MapView.h"
#include "src/shared/RPZAsset.hpp"

class MapLayoutManager : public QTreeWidget{

    Q_OBJECT

    public:
        MapLayoutManager(QWidget * parent = nullptr);
        void alterTreeElements(const RPZAsset::Alteration &state, QList<RPZAsset> &elements);
    
    signals:
        void elementsAlterationAsked(const RPZAsset::Alteration &state, const QList<QUuid> &elementIds);

    protected:
        void keyPressEvent(QKeyEvent * event) override;

    private:
        void _onElementSelectionChanged();
        void _onElementDoubleClicked(QTreeWidgetItem * item, int column);

        QHash<QUuid, QTreeWidgetItem*> _treeItemsById;
        QList<QUuid> _extractIdsFromSelection() const;
        bool _externalInstructionPending = false;
        bool _deletionProcessing = false;

        QTreeWidgetItem* _createTreeItem(RPZAsset &asset);
};