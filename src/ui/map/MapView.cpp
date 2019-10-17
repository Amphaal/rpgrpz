#include "MapView.h"

MapView::MapView(QWidget *parent) : QGraphicsView(parent), MV_Manipulation(this), MV_HUDLayout(this),
    _heavyLoadImage(QPixmap(":/icons/app_64.png")),
    _hints(new MapHint),
    _stdTileSize(AppContext::standardTileSize(this)),
    _menuHandler(new AtomsContextualMenuHandler(_hints, this)) {

    //OpenGL backend activation
    QGLFormat format;
    format.setSampleBuffers(true);
    format.setDirectRendering(true);
    format.setAlpha(true);
    this->setViewport(new QGLWidget(format));

    //hide scrollbars
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    this->setRubberBandSelectionMode(Qt::ItemSelectionMode::ContainsItemBoundingRect); //rubberband UC optimization
    this->setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::SmartViewportUpdate); //force viewport update mode
    this->setMouseTracking(true); //activate mouse tracking for ghost 

    //thread
    this->_hints->moveToThread(new QThread);
    this->_hints->thread()->setObjectName("MapThread");
    this->_hints->thread()->start();

    //default
    auto scene = new QGraphicsScene(
        this->_defaultSceneSize, 
        this->_defaultSceneSize, 
        this->_defaultSceneSize, 
        this->_defaultSceneSize
    );
    this->setScene(scene);
    this->_resetTool();

    this->_handleHintsSignalsAndSlots();

}

MapView::~MapView() {
    if(this->_hints) {
        this->_hints->thread()->quit();
        this->_hints->thread()->wait();
    }
}


void MapView::_updateItemValue(QGraphicsItem* item, const AtomUpdates &updates) {
    
    for(auto i = updates.constBegin(); i != updates.constEnd(); ++i) {
    
        auto param = i.key();

        //update GI
        AtomConverter::updateGraphicsItemFromMetadata(
            item,
            param,
            i.value()
        );

    }
}

void MapView::_handleHintsSignalsAndSlots() {

    //on map loading, set placeholder...
    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::heavyAlterationProcessing,
        this, &MapView::_displayHeavyLoadPlaceholder
    );

    //define selection debouncer
    this->_debounceSelection.setInterval(100);
    this->_debounceSelection.setSingleShot(true);
    this->_debounceSelection.callOnTimeout([=]() {
        this->_hints->notifySelectedItems(
            this->scene()->selectedItems()
        );
    });

    //call debouncer on selection
    QObject::connect(
        this->scene(), &QGraphicsScene::selectionChanged,
        [=]() {this->_debounceSelection.start();}
    );

    QObject::connect(
        this->_hints, &ViewMapHint::requestingUIAlteration,
        this, &MapView::_onUIAlterationRequest
    );
    
    QObject::connect(
        this->_hints, QOverload<const QHash<QGraphicsItem*, AtomUpdates>&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QHash<QGraphicsItem*, AtomUpdates>&>::of(&MapView::_onUIUpdateRequest)
    );
    
    QObject::connect(
        this->_hints, QOverload<const QList<QGraphicsItem*>&, const AtomUpdates&>::of(&ViewMapHint::requestingUIUpdate),
        this, QOverload<const QList<QGraphicsItem*>&, const AtomUpdates&>::of(&MapView::_onUIUpdateRequest)
    );

    QObject::connect(
        this->_hints, &ViewMapHint::requestingUIUserChange,
        this, &MapView::_onUIUserChangeRequest
    );

}

//////////////////
// UI rendering //
//////////////////


void MapView::drawForeground(QPainter *painter, const QRectF &rect) {
    this->_mayUpdateHeavyLoadPlaceholder(painter);
    this->_mayUpdateHUD(painter, rect);
}

void MapView::drawBackground(QPainter *painter, const QRectF &rect) {
    
    painter->save();

        //set opacity
        painter->setOpacity(0.1);

        //pattern
        QBrush brush;
        brush.setStyle(Qt::Dense4Pattern);
        brush.setColor(Qt::GlobalColor::black);
        painter->setBrush(brush);

        //override pen
        painter->setPen(Qt::NoPen);

        //ignore transformations
        QTransform t;
        t.scale(10, 10);
        painter->setTransform(t);

        painter->drawRect(this->rect());

    painter->restore();

}

void MapView::_mayDrawZoomIndic(QPainter* painter, const QRect &viewportRect, double currentScale) {
    
    if(!AppContext::settings()->scaleActive()) return;

    painter->save();

        //zoom indic
        auto templt = QStringLiteral(u"Zoom : %1x");
        templt = templt.arg(currentScale, 0, 0, 2, 0);

        //background
        painter->setOpacity(.75);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush("#FFF", Qt::SolidPattern));
        auto bgRect = painter->boundingRect(viewportRect, templt, QTextOption(Qt::AlignTop | Qt::AlignRight));
        bgRect = bgRect.marginsAdded(QMargins(5, 5, 5, 2));
        bgRect.moveTopRight(viewportRect.topRight());
        painter->drawRoundedRect(bgRect, 2, 2);
        
        //text
        painter->setOpacity(1);
        painter->setPen(QPen(Qt::SolidPattern, 0));
        painter->setBrush(Qt::NoBrush);
        painter->drawText(bgRect, templt, QTextOption(Qt::AlignCenter));

    painter->restore();

}

void MapView::_mayDrawScaleIndic(QPainter* painter, const QRect &viewportRect, double currentScale) {
    
    if(!AppContext::settings()->scaleActive()) return;

    painter->save();

        auto tileWidth = (int)this->_stdTileSize.width();
        auto stops = 5;
        auto rulerSize = (int)(tileWidth * stops);
        auto elipseSize = rulerSize + 50;
        auto ratio = this->_hints->tileToMeterRatio() * (1 / currentScale);

        //cover
        painter->setOpacity(.5);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush("#FFF", Qt::SolidPattern));
        painter->drawEllipse(viewportRect.bottomLeft(), elipseSize, elipseSize);
        painter->setOpacity(1);

        auto scalePos = viewportRect.bottomLeft();
        scalePos.setX(scalePos.x() + 15);
        scalePos.setY(scalePos.y() - 20);

        auto scaleDestX = scalePos;
        scaleDestX.setX(scalePos.x() + rulerSize);

        auto scaleDestY = scalePos;
        scaleDestY.setY(scalePos.y() - rulerSize);

        painter->setPen(QPen(Qt::SolidPattern, 0));
        painter->setBrush(Qt::NoBrush);
        painter->drawLine(scalePos, scaleDestX);
        painter->drawLine(scalePos, scaleDestY);

        //zero
        auto zeroRect = painter->boundingRect(viewportRect, QStringLiteral(u"0"));
        zeroRect.moveCenter(scalePos);
        zeroRect.moveTop(scalePos.y());
        painter->drawText(zeroRect, QStringLiteral(u"0"));

        for(auto stop = 1; stop <= stops; stop++) {
            
            auto pos = scalePos;
            pos.setX(
                pos.x() + (stop * tileWidth)
            );

            auto txtStr = StringHelper::fromMeters(ratio * stop);
            auto textRect = painter->boundingRect(viewportRect, txtStr);
            textRect.moveCenter(pos);
            textRect.moveTop(pos.y());
            painter->drawText(textRect, txtStr);

            auto tickDest = pos;
            tickDest.setY(
                tickDest.y() - 5
            );
            painter->drawLine(pos, tickDest);
            
        }



    painter->restore();
}

void MapView::_mayDrawGridIndic(QPainter* painter, const QRectF &rect, double currentScale) {
    
    if(!AppContext::settings()->gridActive()) return;
    if(currentScale < .1) return; //prevent if scale is too far

    painter->save();

        QPen pen(Qt::SolidPattern, 0);
        pen.setColor("#222");
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        
        auto sceneRect = this->sceneRect();
        auto center = sceneRect.center();

        auto sceneBottom = sceneRect.bottom();
        auto sceneRight = sceneRect.right();
        auto centerX = center.x();
        auto centerY = center.y();
        auto tileWidth = this->_stdTileSize.width();
        auto tileHeight = this->_stdTileSize.height();

        auto numberOfLinesX = (int)(centerX / tileWidth);
        auto numberOfLinesY = (int)(centerY / tileHeight);

        //origin lines
        auto horizontalOriginLine = QLineF(centerX, 0, centerX, sceneBottom);
        auto verticalOriginLine = QLineF(0, centerY, sceneRight, centerY);
        painter->drawLine(horizontalOriginLine); //x
        painter->drawLine(verticalOriginLine); //y

        //horizontal
        for(auto x = 1; x <= numberOfLinesX; x++) {
            auto step = x * tileWidth;
            auto centerXMore = centerX + step;
            auto centerXLess = centerX - step;
            painter->drawLine(QLineF(centerXMore, 0, centerXMore, sceneBottom));
            painter->drawLine(QLineF(centerXLess, 0, centerXLess, sceneBottom));
        }

        //vertical
        for(int y = 1; y <= numberOfLinesY; y++) {
            auto step = y * tileHeight;
            auto centerYMore = centerY + step;
            auto centerYLess = centerY - step;
            painter->drawLine(QLineF(0, centerYMore, sceneRight, centerYMore));
            painter->drawLine(QLineF(0, centerYLess, sceneRight, centerYLess));
        }

    painter->restore();

}

void MapView::_mayUpdateHUD(QPainter* painter, const QRectF &rect) {

    if(this->_heavyLoadExpectedCount > -1) return;

    painter->save();

        auto viewportRect = this->rect();
        auto currentScale = this->transform().m11();

        this->_mayDrawGridIndic(painter, rect, currentScale);

        //ignore transformations
        QTransform t;
        painter->setTransform(t);

        this->_mayDrawZoomIndic(painter, viewportRect, currentScale);
        this->_mayDrawScaleIndic(painter, viewportRect, currentScale);

    painter->restore();

}

void MapView::_updateHeavyLoadPlaceholder() {
    QPainter p(this->viewport());
    this->_mayUpdateHeavyLoadPlaceholder(&p);
}

void MapView::_mayUpdateHeavyLoadPlaceholder(QPainter* painter) {
    
    if(this->_heavyLoadExpectedCount < 0) return;

    auto viewportRect = this->rect();
    auto viewportCenter = viewportRect.center();

    painter->save();

        //hide
        painter->setBrush(QBrush("#EEE", Qt::SolidPattern));
        painter->setPen(Qt::NoPen);
        painter->setTransform(QTransform()); //ignore transformations
        painter->drawRect(viewportRect);
        
        //draw gauge
        if(this->_heavyLoadExpectedCount > 0) {
            
            //ext gauge
            QRect extGauge(QPoint(0, 0), QSize(102, 12));
            extGauge.moveCenter(viewportCenter);
            painter->setBrush(QBrush("#DDD", Qt::SolidPattern));
            painter->setPen(QPen(Qt::NoBrush, 0));
            painter->drawRect(extGauge);

            //draw inner
            if(this->_heavyLoadCurrentCount > 0) {
                
                painter->setOpacity(.5);

                //define inner gauge
                auto innerGauge = extGauge.marginsRemoved(QMargins(1, 1, 1, 1));
                auto ratio = (double)this->_heavyLoadCurrentCount / this->_heavyLoadExpectedCount;
                auto newWidth = (int)(ratio * innerGauge.width());
                innerGauge.setWidth(newWidth);

                //draw it
                painter->setBrush(QBrush(this->_heavyLoadColor, Qt::SolidPattern));
                painter->setPen(Qt::NoPen);
                painter->drawRect(innerGauge);

                painter->setOpacity(1);

            }

        }

        //draw pixmap
        auto pixmapRect = this->_heavyLoadImage.rect();
        auto alteredCenter = viewportCenter;
        alteredCenter.setY(
            viewportCenter.y() - 6 - 32
        );
        pixmapRect.moveCenter(alteredCenter);
        painter->drawPixmap(pixmapRect, this->_heavyLoadImage);

    painter->restore();

}

void MapView::_endHeavyLoadPlaceholder() {
    this->_heavyLoadExpectedCount = -1;
    this->_heavyLoadCurrentCount = -1;
    this->_heavyLoadColor = QColor();
}

void MapView::_displayHeavyLoadPlaceholder() {
    this->_heavyLoadColor = RandomColor::getRandomColor();
    this->_heavyLoadExpectedCount = 0;
    this->_heavyLoadCurrentCount = 0;
}

//////////////////////
// End UI rendering //
//////////////////////

void MapView::_onUIUpdateRequest(const QHash<QGraphicsItem*, AtomUpdates> &toUpdate) {
    
    for(auto i = toUpdate.constBegin(); i != toUpdate.constEnd(); i++) {
        this->_updateItemValue(i.key(), i.value());
    }
    
    this->_endHeavyLoadPlaceholder();
}

void MapView::_onUIUpdateRequest(const QList<QGraphicsItem*> &toUpdate, const AtomUpdates &updates) {
    for(auto item : toUpdate) {
        this->_updateItemValue(item, updates);
    }

    this->_endHeavyLoadPlaceholder();

}

void MapView::_onUIUserChangeRequest(const QList<QGraphicsItem*> &toUpdate, const RPZUser &newUser) {
    
    auto newColor = newUser.color();
    
    for(auto item : toUpdate) {
        if(auto pathItem = dynamic_cast<QGraphicsPathItem*>(item)) {
            auto c_pen = pathItem->pen();
            c_pen.setColor(newColor);
            pathItem->setPen(c_pen);
        } 
    }

    this->_endHeavyLoadPlaceholder();
    
}

void MapView::_onUIAlterationRequest(const PayloadAlteration &type, const QList<QGraphicsItem*> &toAlter) {
    
    //prevent circual selection
    QSignalBlocker b(this->scene());

    if(type == PA_Selected) this->scene()->clearSelection();
    if(type == PA_Reset) {
        this->_clearNonHUDItems();
        this->_heavyLoadExpectedCount = toAlter.count();
        this->_heavyLoadCurrentCount = 0;
    }

    for(auto item : toAlter) {
        switch(type) {

            case PA_Reset:
            case PA_Added: {
                this->_addNonHUDItem(item);
                
                this->_heavyLoadCurrentCount++;
                this->_updateHeavyLoadPlaceholder();

            }
            break;

            case PA_Focused: {
                this->focusItem(item);
            }
            break;

            case PA_Selected: {
                item->setSelected(true);
            }
            break;

            case PA_Removed: {
                this->_removeNonHUDItem(item);
                delete item;
            }
            break;

            case PA_AssetSelected: {
                Tool newTool = item ? Tool::Atom : Tool::Default;
                auto selectedAtom = item ? this->_hints->templateAtom() : RPZAtom();
                if(item) this->_addNonHUDItem(item);

                //change tool, proceed to unselect items
                this->_changeTool(newTool);

                //emit that template selection has changed
                AtomTemplateSelectedPayload payload(selectedAtom);
                AlterationHandler::get()->queueAlteration(this->_hints, payload);
            }
            break;

            default:
            break;
        }
    }

    this->_endHeavyLoadPlaceholder();
}


void MapView::_addNonHUDItem(QGraphicsItem* toAdd) {
    this->scene()->addItem(toAdd);
    this->_nonHUDItems.insert(toAdd);
}

void MapView::_removeNonHUDItem(QGraphicsItem* toRemove) {
    this->scene()->removeItem(toRemove);
    this->_nonHUDItems.remove(toRemove);
}

void MapView::_clearNonHUDItems() {
    for(auto item : this->_nonHUDItems) {
        this->scene()->removeItem(item);
    }
    this->_nonHUDItems.clear();
}

void MapView::contextMenuEvent(QContextMenuEvent *event) {

    auto pos = this->viewport()->mapToGlobal(
        event->pos()
    );
    auto ids = this->_hints->getAtomIdsFromGraphicsItems(
        this->scene()->selectedItems()
    );

    //create menu
    this->_menuHandler->invokeMenu(ids, pos);

}

void MapView::mouseDoubleClickEvent(QMouseEvent *event) {
    
    //check item
    auto item = this->itemAt(event->pos());
    if(!item) return;

    //check item is not temporary !
    auto isTemporary = item->data((int)AtomConverterDataIndex::IsTemporary).toBool();
    if(isTemporary) return;

    //notify focus
    this->_hints->notifyFocusedItem(item);
}

void MapView::resizeEvent(QResizeEvent * event) {
    this->goToSceneCenter();
}

MapHint* MapView::hints() const {
    return this->_hints;
}

void MapView::keyReleaseEvent(QKeyEvent *event) {

    switch(event->key()) {
        case Qt::Key::Key_Up:
        case Qt::Key::Key_Down:
        case Qt::Key::Key_Left:
        case Qt::Key::Key_Right:
            event->ignore();
            return this->removeAnimatedMove(event);
            break;
    }

    QGraphicsView::keyReleaseEvent(event);

}

void MapView::keyPressEvent(QKeyEvent * event) {

    switch(event->key()) {

        //deletion handling
        case Qt::Key::Key_Delete:
            this->_hints->deleteCurrentSelectionItems();
            break;
        
        //ask unselection of current tool
        case Qt::Key::Key_Escape:
            this->_resetTool();
            break;
        
        case Qt::Key::Key_Up:
        case Qt::Key::Key_Down:
        case Qt::Key::Key_Left:
        case Qt::Key::Key_Right:
            event->ignore();
            return this->addAnimatedMove(event);
            break;

    }

    QGraphicsView::keyPressEvent(event);

}

void MapView::_mightCenterGhostWithCursor() {
    
    //update ghost item position relative to cursor
    if(auto ghost = this->_hints->ghostItem()) {
        this->centerItemToPoint(
            ghost, 
            this->mapFromGlobal(QCursor::pos())
        );
    }

}

void MapView::enterEvent(QEvent *event) {
    if(auto ghost = this->_hints->ghostItem()) {
        ghost->setVisible(true);
    }  
}

void MapView::leaveEvent(QEvent *event) {
    if(auto ghost = this->_hints->ghostItem()) {

        //special case for keeping visible when right clicking
        auto cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
        if(this->geometry().contains(cursorPosInWidget)) return;

        ghost->setVisible(false);

    }
}

/////////////
/* NETWORK */
/////////////

void MapView::onRPZClientConnecting() {

    //when self user send
    QObject::connect(
        _rpzClient, &RPZClient::selfIdentityAcked,
        this, &MapView::_onIdentityReceived
    );

}
void MapView::_onIdentityReceived(const RPZUser &self) {
    
    //send map history if host
    if(self.role() == RPZUser::Role::Host) {
        this->_sendMapHistory();
    }

    //define default creation user
    this->_hints->setDefaultUser(self);

    //if host
    auto descriptor = self.role() == RPZUser::Role::Host ? NULL : this->_rpzClient->getConnectedSocketAddress();
    bool is_remote = this->_hints->defineAsRemote(descriptor);

    emit remoteChanged(is_remote);
    
}

void MapView::onRPZClientDisconnect() {

    //back to default state
    QMetaObject::invokeMethod(this->_hints, "loadDefaultRPZMap");

}

void MapView::_sendMapHistory() {
    auto payload = this->_hints->createStatePayload();
    QMetaObject::invokeMethod(this->_rpzClient, "sendMapHistory", 
        Q_ARG(ResetPayload, payload)
    );
}

/////////////////
/* END NETWORK */
/////////////////

//////////////////
/* MOUSE EVENTS */
//////////////////

//mouse click
void MapView::mousePressEvent(QMouseEvent *event) {

    //register last position
    this->_isMousePressed = true;

    switch(event->button()) {

        case Qt::MouseButton::MiddleButton: {
            auto tool = this->_quickTool == Default ? Tool::Scroll : Tool::Default;
            this->_changeTool(tool, true);
        }
        break;

        case Qt::MouseButton::LeftButton: {

            if(this->_getCurrentTool() == Atom) {
                switch(this->_hints->templateAtom().type()) {

                    case AtomType::Drawing:
                    case AtomType::Brush:
                        if(_stickyBrushIsDrawing) {
                            this->_savePosAsStickyNode(event->pos());
                        } else {
                            this->_beginDrawing(event->pos());
                        }
                    break;

                    default: {
                        this->_hints->integrateGraphicsItemAsPayload(this->_hints->ghostItem());
                    }
                    break;

                }
            }
        }
        break;

        default:
            break;
    }

    QGraphicsView::mousePressEvent(event);
}

//on movement
void MapView::mouseMoveEvent(QMouseEvent *event) {

    this->_mightCenterGhostWithCursor();

    switch(this->_getCurrentTool()) {
        case Tool::Atom:
            switch(this->_hints->templateAtom().type()) {
                case AtomType::Drawing:
                case AtomType::Brush:
                    this->_updateDrawingPath(event->pos());
                break;

                default:
                    break;
            }
        
        default:
            break;
    }

    QGraphicsView::mouseMoveEvent(event);
}

//mouse drop
void MapView::mouseReleaseEvent(QMouseEvent *event) {

    switch(event->button()) {
        case Qt::MouseButton::LeftButton: {
            
            //if was drawing...
            if(!this->_stickyBrushIsDrawing) this->_endDrawing();

            //if something moved ?
            this->_hints->mightNotifyMovement(this->scene()->selectedItems());
            
        }
        break;

        default:
            break;
    }

    this->_isMousePressed = false;

    QGraphicsView::mouseReleaseEvent(event);
}

//////////////////////
/* END MOUSE EVENTS */
//////////////////////

//////////
/* TOOL */
//////////

//returns tool
MapView::Tool MapView::_getCurrentTool() const {
    return this->_quickTool == Tool::Default ? this->_tool : this->_quickTool;
}

void MapView::_resetTool() {
    this->_quickTool = Tool::Default;
    this->_changeTool(Tool::Default);
}

//change tool
void MapView::_changeTool(Tool newTool, const bool quickChange) {

    this->_endDrawing();

    //if quick change asked
    if(quickChange) {

        this->_quickTool = newTool;

        //if unselecting quicktool
        if(newTool == Tool::Default) newTool = this->_tool;

    } 
    
    //if standard tool change
    else {

        //since clearSelection wont trigger notification, hard call notification on reset
        if(this->_tool == Atom && newTool == Default) {
            this->_hints->notifySelectedItems(
                this->scene()->selectedItems()
            );
        }

        this->_tool = newTool;
        this->scene()->clearSelection();
    }    

    //if a quicktool is selected
    if(this->_quickTool != Tool::Default) {
        newTool = this->_quickTool;
    }
    
    //depending on tool
    switch(newTool) {
        case Tool::Atom: {
            
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::NoDrag);
            
            switch(this->_hints->templateAtom().type()) {
                case AtomType::Drawing:
                    this->setCursor(Qt::CrossCursor);
                    break;
                case AtomType::Object:
                    this->setCursor(Qt::ClosedHandCursor);
                    break;
                case AtomType::Text:
                    this->setCursor(Qt::IBeamCursor);
                    break;
                default:
                    this->setCursor(Qt::CrossCursor);
                    break;
            }

        }
        break;
        case Tool::Scroll:
            this->setInteractive(false);
            this->setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
            break;
        case Tool::Default:
        default:
            this->setInteractive(true);
            this->setDragMode(QGraphicsView::DragMode::RubberBandDrag);
            this->setCursor(Qt::ArrowCursor);
            break;
    }
}

//on received action
void MapView::onActionRequested(const MapTools::Actions &action) {
    switch(action) {
        case MapTools::Actions::ResetView:
            this->goToDefaultViewState();
            break;
        case MapTools::Actions::ResetTool:
            this->_resetTool();
            break;
        default:
            break;
    }
}

void MapView::onHelperActionTriggered(QAction *action) {
    this->setForegroundBrush(Qt::NoBrush); //force foreground re-drawing
}

//////////////
/* END TOOL */
//////////////

void MapView::onAnimationManipulationTickDone() {
    this->_mightCenterGhostWithCursor();
}

//////////
/* ZOOM */
//////////

void MapView::wheelEvent(QWheelEvent *event) {

    //make sure no button is pressed
    if(this->_isMousePressed) return;

    this->animateScroll(event);

};


//////////////
/* END ZOOM */
//////////////

/////////////
/* DRAWING */
/////////////

void MapView::_destroyTempDrawing() {
    if(!this->_tempDrawing) return;
    
    //remove helpers
    for(auto helper : this->_tempDrawingHelpers) {
        this->_removeNonHUDItem(helper);
        delete helper;
    }
    this->_tempDrawingHelpers.clear();
        
    //remove drawing
    this->_removeNonHUDItem(this->_tempDrawing);
    delete this->_tempDrawing;
    this->_tempDrawing = nullptr;

    //reset sticky
    if(this->_stickyBrushIsDrawing) {   
        this->_stickyBrushIsDrawing = false;
        this->_stickyBrushValidNodeCount = 0;
    }
    
}

void MapView::_beginDrawing(const QPoint &lastPointMousePressed) {

    //destroy temp
    this->_destroyTempDrawing();

    //create base and store it
    auto item = this->_hints->generateTemporaryItemFromTemplateBuffer();
    this->_addNonHUDItem(item);
    this->_tempDrawing = static_cast<MapViewGraphicsPathItem*>(item);

    //determine if it must be sticky
    this->_stickyBrushIsDrawing = this->_hints->templateAtom().brushType() == BrushType::Cutter;
    this->_stickyBrushValidNodeCount = this->_stickyBrushIsDrawing ? this->_tempDrawing->path().elementCount() : 0;

    //update position
    this->centerItemToPoint(this->_tempDrawing, lastPointMousePressed);

    //add outline
    if(this->_stickyBrushIsDrawing) {
        auto pos = this->_tempDrawing->pos();
        auto outline = CustomGraphicsItemHelper::createOutlineRectItem(pos);
        this->_addNonHUDItem(outline);
        this->_tempDrawingHelpers.append(outline);
    }
}

void MapView::_updateDrawingPath(const QPoint &evtPoint) {
    
    //if no temp, stop
    if(!this->_tempDrawing) return;

    //get existing path
    auto existingPath = this->_tempDrawing->path();

    //define destination coordonate
    auto sceneCoord = this->mapToScene(evtPoint);
    auto pathCoord = this->_tempDrawing->mapFromScene(sceneCoord);

    switch(this->_hints->templateAtom().type()) {
        
        case AtomType::Drawing:
            existingPath.lineTo(pathCoord);
        break;

        case AtomType::Brush:
            this->_updateDrawingPathForBrush(pathCoord, existingPath, this->_tempDrawing);
        break;

        default:
            break;

    }

    //save as new path
    this->_tempDrawing->setPath(existingPath);
}

void MapView::_updateDrawingPathForBrush(const QPointF &pathCoord, QPainterPath &pathToAlter, MapViewGraphicsPathItem* sourceTemplate) {
    
    switch(this->_hints->templateAtom().brushType()) {
        
        case BrushType::Stamp: {
            
            //if contained in path, nothing to do
            if(pathToAlter.contains(pathCoord)) return;

            auto xRatio = qCeil(pathCoord.x() / sourceTemplate->sourceBrushSize().width()) - 1;
            auto yRatio = qCeil(pathCoord.y() / sourceTemplate->sourceBrushSize().height()) - 1;
            auto expectedStampPos = QPointF(
                xRatio * sourceTemplate->sourceBrushSize().width(),
                yRatio * sourceTemplate->sourceBrushSize().height()
            );

            //if something at expected, nothing to do
            if(pathToAlter.contains(expectedStampPos)) return;
            
            //add rect
            QRectF rect(expectedStampPos, sourceTemplate->sourceBrushSize());
            pathToAlter.addRect(rect);

        }
        break;

        case BrushType::Cutter: {
            auto count = pathToAlter.elementCount();
            
            //if no temporary node, create it
            if(this->_stickyBrushValidNodeCount == count) {
                pathToAlter.lineTo(pathCoord);
            } 
            
            //update temporary node
            else {
                pathToAlter.setElementPositionAt(count-1, pathCoord.x(), pathCoord.y());
            }
        }                
        break;

        case BrushType::Ovale: {
            pathToAlter = QPainterPath();
            QRectF rect(QPointF(0,0), pathCoord);
            pathToAlter.addEllipse(rect);
        }
        break;

        case BrushType::Rectangle: {
            pathToAlter = QPainterPath();
            QRectF rect(QPointF(0,0), pathCoord);
            pathToAlter.addRect(rect);
        }
        break;

        case BrushType::Scissors: {
            pathToAlter.lineTo(pathCoord);
        }
        break;

        case BrushType::RoundBrush: {
            pathToAlter.lineTo(pathCoord);
        }
        break;

        default:
            break;

    }
}

void MapView::_endDrawing() {
    
    //if no temporary drawing, stop
    if(!this->_tempDrawing) return;
    
    //if too small stop
    auto path = this->_tempDrawing->path();
    if(path.elementCount() < 2) return;

    //add definitive path
    this->_hints->integrateGraphicsItemAsPayload(this->_tempDrawing);

    //destroy temp
    this->_destroyTempDrawing();

}

void MapView::_savePosAsStickyNode(const QPoint &evtPoint) {
    
    //dest pos
    auto sceneCoord = this->mapToScene(evtPoint);
    auto destCoord = this->_tempDrawing->mapFromScene(sceneCoord);
    
    //Update
    auto path = this->_tempDrawing->path();
    auto count = path.elementCount();
    path.setElementPositionAt(count-1, destCoord.x(), destCoord.y());
    this->_tempDrawing->setPath(path);
    
    //add visual helper
    auto outline = CustomGraphicsItemHelper::createOutlineRectItem(sceneCoord);
    this->_addNonHUDItem(outline);
    this->_tempDrawingHelpers.append(outline);

    //update
    this->_stickyBrushValidNodeCount = count;
    
    //check vicinity between last node and first node
    auto firstNode = this->_tempDrawing->mapToScene(path.elementAt(0));
    auto firstNodeGlobalPos = this->mapFromScene(firstNode);
    auto gap = evtPoint - firstNodeGlobalPos;

    //if gap is minimal, understand that the users wants to end the drawing
    if(gap.manhattanLength() < 20) {
        this->_endDrawing();
    }
}

/////////////////
/* END DRAWING */
/////////////////
