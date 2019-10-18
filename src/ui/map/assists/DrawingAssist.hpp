#pragma once

#include "src/shared/hints/MapHint.h"
#include <QGraphicsView>

class DrawingAssist {
    public:
        DrawingAssist(MapHint* hints, QGraphicsView* view) : _hints(hints), _view(view) { }

        void addDrawingPoint(const QPoint &pos) {
            
            if(this->_stickyBrushIsDrawing) {
                this->_savePosAsStickyNode(pos);
            } 
            
            else {
                this->_beginDrawing(pos);
            }

        }

        void updateDrawingPath(const QPoint &evtPoint, const AtomType &type) {
            
            //if no temp, stop
            if(!this->_tempDrawing) return;

            //get existing path
            auto existingPath = this->_tempDrawing->path();

            //define destination coordonate
            auto sceneCoord = this->_view->mapToScene(evtPoint);
            auto pathCoord = this->_tempDrawing->mapFromScene(sceneCoord);

            switch(type) {
                
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

        void onMouseRelease() {
            if(!this->_stickyBrushIsDrawing) this->mayCommitDrawing();
        }

        void mayCommitDrawing() {
            
            //if no temporary drawing, stop
            if(!this->_tempDrawing) return;
            
            //if too small stop
            auto path = this->_tempDrawing->path();
            if(path.elementCount() < 2) return;

            //add definitive path
            this->_commitedDrawingId = this->_hints->integrateGraphicsItemAsPayload(this->_tempDrawing);

        }
        
        void compareItemToCommitedDrawing(QGraphicsItem* itemInserted) {
            
            if(!this->_commitedDrawingId) return;
            if(this->_hints->getAtomIdFromGraphicsItem(itemInserted) != this->_commitedDrawingId) return;

            this->_destroyTempDrawing();

            this->_commitedDrawingId = 0;

        }

    private:
        QGraphicsView* _view = nullptr;
        MapHint* _hints = nullptr;
        RPZAtomId _commitedDrawingId = 0;

        //drawing...
        MapViewGraphicsPathItem* _tempDrawing = nullptr;
        QList<QGraphicsItem*> _tempDrawingHelpers;
        bool _stickyBrushIsDrawing = false;
        int _stickyBrushValidNodeCount = 0;

        void _destroyTempDrawing() {
            if(!this->_tempDrawing) return;
            
            //remove helpers
            for(auto helper : this->_tempDrawingHelpers) {
                delete helper;
            }
            this->_tempDrawingHelpers.clear();
                
            //remove drawing
            delete this->_tempDrawing;
            this->_tempDrawing = nullptr;

            //reset sticky
            if(this->_stickyBrushIsDrawing) {   
                this->_stickyBrushIsDrawing = false;
                this->_stickyBrushValidNodeCount = 0;
            }
            
        }

        void _beginDrawing(const QPoint &lastPointMousePressed) {

            //destroy temp
            this->_destroyTempDrawing();

            //create base and store it
            auto item = this->_hints->generateTemporaryItemFromTemplateBuffer();
            this->_view->scene()->addItem(item);
            this->_tempDrawing = static_cast<MapViewGraphicsPathItem*>(item);

            //determine if it must be sticky
            this->_stickyBrushIsDrawing = this->_hints->templateAtom().brushType() == BrushType::Cutter;
            this->_stickyBrushValidNodeCount = this->_stickyBrushIsDrawing ? this->_tempDrawing->path().elementCount() : 0;

            //add outline
            if(this->_stickyBrushIsDrawing) {
                auto pos = this->_tempDrawing->pos();
                auto outline = CustomGraphicsItemHelper::createOutlineRectItem(pos);
                this->_view->scene()->addItem(outline);
                this->_tempDrawingHelpers.append(outline);
            }
        }

        void _updateDrawingPathForBrush(const QPointF &pathCoord, QPainterPath &pathToAlter, MapViewGraphicsPathItem* sourceTemplate) {
            
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

        void _savePosAsStickyNode(const QPoint &evtPoint) {
            
            //dest pos
            auto sceneCoord = this->_view->mapToScene(evtPoint);
            auto destCoord = this->_tempDrawing->mapFromScene(sceneCoord);
            
            //Update
            auto path = this->_tempDrawing->path();
            auto count = path.elementCount();
            path.setElementPositionAt(count-1, destCoord.x(), destCoord.y());
            this->_tempDrawing->setPath(path);
            
            //add visual helper
            auto outline = CustomGraphicsItemHelper::createOutlineRectItem(sceneCoord);
            this->_view->scene()->addItem(outline);
            this->_tempDrawingHelpers.append(outline);

            //update
            this->_stickyBrushValidNodeCount = count;
            
            //check vicinity between last node and first node
            auto firstNode = this->_tempDrawing->mapToScene(path.elementAt(0));
            auto firstNodeGlobalPos = this->_view->mapFromScene(firstNode);
            auto gap = evtPoint - firstNodeGlobalPos;

            //if gap is minimal, understand that the users wants to end the drawing
            if(gap.manhattanLength() < 20) {
                this->mayCommitDrawing();
            }
        }


};