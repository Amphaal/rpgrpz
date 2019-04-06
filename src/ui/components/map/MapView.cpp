#include "MapView.h"


MapView::MapView(QWidget *parent) : QGraphicsView(parent), _scene(new QGraphicsScene) {
    
    //default
    this->setAcceptDrops(true);
    this->_changeTool(MapView::_defaultTool);
    QObject::connect(
        this->_scene, &QGraphicsScene::selectionChanged,
        this, &MapView::_onSceneSelectionChanged
    );

    //custom cursors
    this->_rotateCursor = new QCursor(QPixmap(":/icons/app/tools/rotate.png"));
    
    //openGL activation
    this->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::DirectRendering)));
    this->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    this->update();

    //background
    auto background = new QBrush("#EEE", Qt::CrossPattern);
    this->setBackgroundBrush(*background);
    //this->setCacheMode(QGraphicsView::CacheBackground);

    //optimisations
    //this->setOptimizationFlags( QFlags<OptimizationFlag>(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing));

    //define scene
    this->_scene->setSceneRect(35000, 35000, 35000, 35000);
    this->setScene(this->_scene);
}

void MapView::_onSceneSelectionChanged() {
    if(this->_externalInstructionPending) return;

    //emit event
    const auto mapToEvt = this->_fetchAssets(this->_scene->selectedItems());
    emit mapElementsAltered(mapToEvt, MapElementEvtState::Selected);
}

void MapView::keyPressEvent(QKeyEvent * event) {
    
    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->_alterScene(MapElementEvtState::Removed, this->_scene->selectedItems());
            break;
        
        //ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->_changeTool(MapView::_defaultTool);
            emit unselectCurrentToolAsked();
            break;
    }

}

/////////////
/* NETWORK */
/////////////


void MapView::bindToRPZClient(RPZClient * cc) {

    this->_currentCC = cc;

    //on map change
    QObject::connect(
        this->_currentCC, &RPZClient::hostMapChanged,
        this, &MapView::unpackFromNetworkReceived
    );

    //destroy
    QObject::connect(
        this->_currentCC, &QObject::destroyed,
        [&]() {
            this->_currentCC = 0;
        }
    );

}

QVariantList MapView::packageForNetworkSend(QList<Asset> &assets, const MapView::MapElementEvtState &state) {
    
    QVariantList toSend;

    //parse data
    QVariantHash data;
    data.insert("state", state);

    QVariantHash assetsContainer;
    for(auto &asset : assets) {

        QByteArray assetData = NULL;

        switch(asset.type()) {
            case AssetType::Type::Drawing:
                auto casted = (QGraphicsPathItem*)asset.graphicsItem();
                const auto path = casted->path();
                assetData = JSONSerializer::toBase64(path);
                break;
        }

        QVariantHash binder;
        binder.insert("data", assetData);
        binder.insert("type", asset.type());
        binder.insert("owner", asset.ownerId());

        assetsContainer.insert(asset.id().toString(), binder);
    }

    data.insert("assets", assetsContainer);
    toSend.append(data);

    return toSend;
}

void MapView::unpackFromNetworkReceived(const QVariantList &package) {

    //container
    QList<Asset> out; 

    //get data
    const auto data = package[0].toHash();
    const auto state = (MapElementEvtState)data["state"].toInt();

    //iterate through assets
    const auto assetsContainer = data["assets"].toHash();
    for(auto &key : assetsContainer.keys()) {

        //get data
        const auto elemId = QUuid::fromString(key);
        const auto binder = assetsContainer[key].toHash();
        const auto binderType = (AssetType::Type)binder["type"].toInt();
        const auto binderOwner = binder["owner"].toUuid();

        //build asset or fetch it
        Asset newAsset;

        //if new element from network
        if(state == MapElementEvtState::Added) {
            
            //elem already exists, shouldnt rewrite it!
            if(this->_assetsById.contains(elemId)) continue;

            //newly created map elem
            QGraphicsItem * newItem;

            //depending on assetType...
            switch(binderType) {
                
                //drawing...
                case AssetType::Type::Drawing:
                    const QPainterPath path = JSONSerializer::fromBase64(binder["data"]);
                    newItem = this->_scene->addPath(path);
                    break;
            
            }

            //define
            newAsset = Asset(binderType, newItem, key, binderOwner);

        } else {
            //fetch from stock
            newAsset = this->_assetsById[key];
        }
        
        //add it to container
        out.append(newAsset);
    }

    //process new state
    this->_alterScene(state, out);
}

/////////////////
/* END NETWORK */
/////////////////

//////////
/* TOOL */
//////////

//mouse click
void MapView::mousePressEvent(QMouseEvent *event) {

    //register last position
    this->_lastPointMousePressing = event->pos();
    this->_lastPointMousePressed = event->pos();
    this->_isMousePressed = true;

    switch(event->button()) {
        case Qt::MouseButton::MiddleButton:
            this->_isMiddleToolLock = !this->_isMiddleToolLock;
            this->_changeTool(MapTools::Actions::Scroll, true);
            break;
        case Qt::MouseButton::RightButton:
            this->_changeTool(MapTools::Actions::Rotate, true);
            break;
        case Qt::MouseButton::LeftButton:
            this->_toolOnMousePress(this->_getCurrentTool());
            break;
    }

    QGraphicsView::mousePressEvent(event);
}

//on movement
void MapView::mouseMoveEvent(QMouseEvent *event) {

    switch(this->_getCurrentTool()) {
        case MapTools::Actions::Draw:
            this->_drawLineTo(event->pos());
            break;
        case MapTools::Actions::Rotate:
            this->_rotate(event->pos());
            break;
    }

    //register last position
    this->_lastPointMousePressing = event->pos();

    QGraphicsView::mouseMoveEvent(event);
}


//mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {

    switch(event->button()) {
        case Qt::MouseButton::RightButton:
            this->_changeTool(MapTools::Actions::None, true);
            break;
        case Qt::MouseButton::MiddleButton:
            if(this->_isMiddleToolLock) {
                this->_changeTool(MapTools::Actions::None, true);
            }
            break;
        case Qt::MouseButton::LeftButton:
            this->_toolOnMouseRelease(this->_getCurrentTool());
            break;
    }

    this->_isMousePressed = false;

    QGraphicsView::mouseReleaseEvent(event);
}

void MapView::_toolOnMousePress(const MapTools::Actions &tool) {
    switch(tool) {
        case MapTools::Actions::Draw: {
            this->_beginDrawing();
        }
        break;
    }
}

void MapView::_toolOnMouseRelease(const MapTools::Actions &tool) {
    if(this->_tempDrawing)  {
        this->_endDrawing();
    }
}


//returns tool
MapTools::Actions MapView::_getCurrentTool() const {
    return this->_quickTool == MapTools::Actions::None ? this->_selectedTool : this->_quickTool;
}

//change tool
void MapView::_changeTool(MapTools::Actions newTool, const bool quickChange) {

    if(quickChange) {
        this->_quickTool = newTool;
        if(newTool == MapTools::Actions::None) {
            newTool = this->_selectedTool;
        }   
    } else {
        this->_selectedTool = newTool;
        this->_scene->clearSelection();
    }    
    
    switch(newTool) {
        case MapTools::Actions::Draw:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(Qt::CrossCursor);
            break;
        case MapTools::Actions::Rotate:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            this->setCursor(*this->_rotateCursor);
            break;
        case MapTools::Actions::Scroll:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
            this->setCursor(Qt::ClosedHandCursor);
            break;
        case MapTools::Actions::Select:
        default:
            this->setInteractive(true);
            this->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
            this->setCursor(Qt::ArrowCursor);
    }
}

//on received event
void MapView::changeToolFromAction(QAction *action) {
    
    const auto instruction = (MapTools::Actions)action->data().toInt();
    if(instruction == MapTools::Actions::RotateToNorth) {
        this->_rotateBackToNorth();
        return;
    }

    //go by default tool if unchecked
    auto state = action->isChecked();
    if(!state) return this->_changeTool(MapView::_defaultTool);

    //else select the new tool
    return this->_changeTool(instruction);

}

void MapView::changePenSize(const int newSize) {
    this->_penWidth = newSize;
}

//////////////
/* END TOOL */
//////////////

//////////
/* ZOOM */
//////////

void MapView::wheelEvent(QWheelEvent *event) {

    //make sure no button is pressed
    if(this->_isMousePressed) return;

    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15; // see QWheelEvent documentation
    this->_numScheduledScalings += numSteps;
    
    if (this->_numScheduledScalings * numSteps < 0) {
        this->_numScheduledScalings = numSteps; // if user moved the wheel in another direction, we reset previously scheduled scalings
    } 
    
    QTimeLine *anim = new QTimeLine(350, this);
    anim->setUpdateInterval(20);

    QObject::connect(anim, &QTimeLine::valueChanged, this, &MapView::_zoomBy_scalingTime);
    QObject::connect(anim, &QTimeLine::finished, this, &MapView::_zoomBy_animFinished);
    
    anim->start();
};

void MapView::_zoomBy_scalingTime(qreal x) {
    qreal factor = 1.0 + qreal(this->_numScheduledScalings) / 300.0;
    this->scale(factor, factor);
}

void MapView::_zoomBy_animFinished() {
    if (this->_numScheduledScalings > 0) {
        this->_numScheduledScalings--;
    } else {
        this->_numScheduledScalings++;
        this->sender()->deleteLater();
    }
}

//////////////
/* END ZOOM */
//////////////

////////////
/* ROTATE */
////////////

void MapView::_rotate(const QPoint &evtPoint) {
    const auto way = this->_lastPointMousePressing - evtPoint;
    auto pp = ((double)way.y()) / 5;
    this->_degreesFromNorth += pp;
    this->rotate(pp);
}

void MapView::_rotateBackToNorth() {
    auto adjust = fmod(this->_degreesFromNorth, 360);
    this->rotate(-this->_degreesFromNorth);
    this->_degreesFromNorth = 0;
}

////////////////
/* END ROTATE */
////////////////

//////////
/* DROP */
//////////

void MapView::dropEvent(QDropEvent *event) {
    
    // auto source = (AssetsNavigator*)event->source();
    // auto si = source->selectedItems();
    // auto toRepresent = si[0]->data(1, Qt::UserRole);
    

    // QPointF point = this->mapToScene(event->pos());
    // item->setPos(point);
    // auto item = new QGraphicsSvgItem("C:/Users/Amphaal/Desktop/pp.svg");
    // this->_scene->addItem(item);

    // //update latest for auto inserts
    // this->_latestPosDrop->setX(this->_latestPosDrop->x() + 10);
    // this->_latestPosDrop->setY(this->_latestPosDrop->y() + 10);
}

void MapView::dragMoveEvent(QDragMoveEvent * event) {
    event->acceptProposedAction();
}

void MapView::dragEnterEvent(QDragEnterEvent *event) {
    
    //if has a widget attached, OK
    if(event->source()) {
        event->acceptProposedAction();
    }

}

//////////////
/* END DROP */
//////////////

/////////////
/* DRAWING */
/////////////

void MapView::_beginDrawing() {
    this->_tempDrawing = new QPainterPath(this->mapToScene(this->_lastPointMousePressed));
}

void MapView::_endDrawing() {
    //add definitive path
    auto newPath = this->_scene->addPath(*this->_tempDrawing, this->_getPen());
    newPath->setFlags(QFlags<QGraphicsItem::GraphicsItemFlag>(
        QGraphicsItem::GraphicsItemFlag::ItemIsSelectable |
        QGraphicsItem::GraphicsItemFlag::ItemIsMovable
    ));
    this->_alterScene(MapElementEvtState::Added, Asset(AssetType::Type::Drawing, newPath));
    this->_tempDrawing = nullptr;
    
    //destroy temp
    for(auto &i : this->_tempLines) {
        delete i;
    }
    this->_tempLines.clear();
}

void MapView::_drawLineTo(const QPoint &evtPoint) {

    //save
    this->_tempDrawing->lineTo(this->mapToScene(evtPoint));

    //draw temp line
    const auto lineCoord = QLineF(this->mapToScene(this->_lastPointMousePressing), this->mapToScene(evtPoint));
    auto tempLine = this->_scene->addLine(lineCoord, this->_getPen());
    this->_tempLines.append(tempLine);
}

QPen MapView::_getPen() const {
    return QPen(this->_penColor, this->_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

/////////////////
/* END DRAWING */
/////////////////

//////////////
/* ELEMENTS */
//////////////

//register actions
QUuid MapView::_alterSceneInternal(const MapElementEvtState &alteration, Asset &asset) {

    //get the Uuids
    auto elemId = asset.id();
    if(elemId.isNull()) {
        elemId = alteration == MapElementEvtState::Added ? QUuid::createUuid() : this->_idsByGraphicItem[asset.graphicsItem()];
        asset.setId(elemId);
    }
    auto ownerId = asset.ownerId();
    
    switch(alteration) {
        
        //on addition
        case MapElementEvtState::Added:
            
            //bind to owners
            if(!ownerId.isNull()) {
                if(!this->_foreignElementIdsByOwnerId.contains(ownerId)) {
                    this->_foreignElementIdsByOwnerId.insert(ownerId, QSet<QUuid>());
                }
                this->_foreignElementIdsByOwnerId[ownerId].insert(elemId);
            } else {
                this->_selfElements.insert(elemId);
            }

            //bind elem
            if(!this->_assetsById.contains(ownerId)) {
                this->_assetsById.insert(elemId, asset);
                this->_idsByGraphicItem.insert(asset.graphicsItem(), elemId);
            }
            break;
        
        //on focus
        case MapElementEvtState::Focused:
            this->centerOn(asset.graphicsItem());
            break;

        //on selection
        case MapView::MapElementEvtState::Selected:
            asset.graphicsItem()->setSelected(true);
            break;

        //on removal
        case MapElementEvtState::Removed:

            //unbind from owners
            if(!ownerId.isNull()) {
               this->_foreignElementIdsByOwnerId[ownerId].remove(elemId);
            } else {
                this->_selfElements.remove(elemId);
            }

            //remove from map
            delete asset.graphicsItem();

            //update 
            this->_assetsById.remove(elemId);
            this->_idsByGraphicItem.remove(asset.graphicsItem());
            break;

    }

    return elemId;
}

//alter Scene
void MapView::_alterScene(const MapElementEvtState &alteration, QList<Asset> &assets) { 
    
    //make sure to clear selection before selecting new
    if(alteration == MapElementEvtState::Selected) {
        this->_scene->clearSelection();
    }

    //handling
    for(auto &asset : assets) {
        this->_alterSceneInternal(alteration, asset);
    }

    //emit event
    emit mapElementsAltered(assets, alteration);
}

//helper
void MapView::_alterScene(const MapElementEvtState &alteration, Asset &asset) {
    QList<Asset> list;
    list.append(asset);
    return this->_alterScene(alteration, list);
}

//helper
void MapView::_alterScene(const MapElementEvtState &alteration, const QList<QGraphicsItem*> &elements) {
    return this->_alterScene(alteration, this->_fetchAssets(elements));
}

//helper
void MapView::_alterScene(const MapElementEvtState &alteration, const QList<QUuid> &elementIds) {
    return this->_alterScene(alteration, this->_fetchAssets(elementIds));
}


//from external instructions
void MapView::alterScene(const QList<QUuid> &elementIds, const MapView::MapElementEvtState &state) {
    
    this->_externalInstructionPending = true;

    //update internal state
    this->_alterScene(state, elementIds);

    this->_externalInstructionPending = false;
}

//helper
QList<Asset> MapView::_fetchAssets(const QList<QGraphicsItem*> &listToFetch) const {
    QList<Asset> list;

    for(auto &e : listToFetch) {
        list.append(this->_assetsById[this->_idsByGraphicItem[e]]);
    }

    return list;
}

//helper
QList<Asset> MapView::_fetchAssets(const QList<QUuid> &listToFetch) const {
   QList<Asset> list;

    for(auto &e : listToFetch) {
        list.append(this->_assetsById[e]);
    }

   return list; 
}


//////////////////
/* END ELEMENTS */
//////////////////
