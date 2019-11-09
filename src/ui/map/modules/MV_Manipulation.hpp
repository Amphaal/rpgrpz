#pragma once

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QSet>
#include <QTimeLine>
#include <QTimer>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>

#include "src/ui/map/_base/AnimationTimeLine.hpp"

#include "src/shared/models/RPZMapParameters.hpp"

class MV_Manipulation {
    public:
        enum class MoveDirection { GoUndefined, GoLeft, GoUp, GoRight, GoDown };
        
        MV_Manipulation(QGraphicsView* view) : _view(view) {

            //define stiff animation
            this->_stiffMove.setInterval(10);
            QObject::connect(
                &this->_stiffMove, &QTimer::timeout,
                [=]() { this->_runAnimationOnKeyMove(); }
            );
            
            //define
            this->_moveAnimator.setDuration(300);
            this->_moveAnimator.setUpdateInterval(10);
            this->_moveAnimator.setDirection(QTimeLine::Direction::Backward);
            this->_moveAnimator.setCurveShape(QTimeLine::CurveShape::EaseOutCurve);

            //on finished, reset any move
            QObject::connect(
                &this->_moveAnimator, &QTimeLine::finished,
                [=](){
                    this->_currentMoveDirections.clear();
                    this->_currentMoveInstructions.clear();
                }
            );

            //on animation going
            QObject::connect(
                &this->_moveAnimator, &QTimeLine::valueChanged,
                [=](qreal x) { this->_runAnimationOnKeyMove(x); }
            );

        }

    protected:
        void focusItem(QGraphicsItem* toFocus) {
            
            auto bound = toFocus->sceneBoundingRect();

            //check if ignores transformation
            if(toFocus->flags().testFlag(QGraphicsItem::GraphicsItemFlag::ItemIgnoresTransformations)) {
                
                auto newCenter = bound.center() - toFocus->boundingRect().center();
                bound.moveCenter(newCenter);

                this->_view->centerOn(bound.center());
                
            }

            //move whole view to fit at scale
            else {
                
                bound = bound.marginsAdded(
                    QMarginsF(
                        bound.width() / 2,
                        bound.height() / 2,
                        bound.width() / 2,
                        bound.height() / 2
                    )
                );
                
                this->_view->fitInView(bound, Qt::AspectRatioMode::KeepAspectRatio);

            }

        }

        void goToDefaultViewState() {
            this->goToSceneCenter();
            this->goToDefaultZoom();
        }

        void goToSceneCenter() {
            auto center = this->_view->scene()->sceneRect().center();
            this->_view->centerOn(center);
        }
        
        void goToDefaultZoom() {
            auto corrected = 1 / this->_view->transform().m11();
            this->_view->scale(corrected, corrected);
        }

        void addAnimatedMove(const QKeyEvent *event) {

            auto direction = this->_keyPressEventToMoveDirection(event);

            //set direction, remove opposite
            this->_currentMoveDirections.insert(direction);
            auto opposite = this->_getOppositeDirection(direction);
            this->_currentMoveDirections.remove(opposite);

            //update instructions
            this->_currentMoveInstructions = this->_getMoveInstructions(this->_currentMoveDirections);

            //start if necessary
            this->_moveAnimator.stop();
            this->_stiffMove.start();

        }

        void removeAnimatedMove(const QKeyEvent *event) {
            
            auto direction = this->_keyPressEventToMoveDirection(event);

            //remove direction
            this->_currentMoveDirections.remove(direction);

            //update instructions
            if(this->_currentMoveDirections.isEmpty()) {
                this->_moveAnimator.start();
                this->_stiffMove.stop();
            } 
            
            else {
                this->_currentMoveInstructions = this->_getMoveInstructions(this->_currentMoveDirections);
            }
            
        }

        void animateScroll(QWheelEvent *event, const RPZMapParameters &mapParams) {
            
            //cap acceleration to 5% per tick
            auto delta = event->delta();
            auto modifier = ((double)delta / 5000);
            modifier = std::clamp(modifier, -.05, .05);

            //define animation handler
            AnimationTimeLine::use(
                AnimationTimeLine::Type::Zoom, 
                modifier, 
                [&](qreal base, qreal prc) {
                    
                    //if factor is 1, e.g. no move, skip
                    auto factor = 1.0 + (prc * base);
                    if(factor == 1) return;

                    //project scaling
                    auto transform = this->_view->transform();
                    transform.scale(factor, factor);
                    auto projectedScale = transform.m11();

                    //if not between limits, skip
                    if(projectedScale < mapParams.minimumZoomScale()) return;
                    if(projectedScale > mapParams.maximumZoomScale()) return;

                    //scale
                    this->_view->scale(factor, factor);

                    this->onAnimationManipulationTickDone();

                }
            );

        }

        virtual void onAnimationManipulationTickDone() {} //overidable

    private:
        struct MoveInstruction {
            QScrollBar* affectedScroll;
            int correction;
        };

        QGraphicsView* _view = nullptr;
        
        //moving...
        QSet<MoveDirection> _currentMoveDirections;
        QList<MoveInstruction> _currentMoveInstructions;
        QTimeLine _moveAnimator;
        QTimer _stiffMove;

        MoveDirection _keyPressEventToMoveDirection(const QKeyEvent *event) {
            switch(event->key()) {
                case Qt::Key::Key_Up:
                    return MoveDirection::GoUp;
                
                case Qt::Key::Key_Down:
                    return MoveDirection::GoDown;

                case Qt::Key::Key_Left:
                    return MoveDirection::GoLeft;

                case Qt::Key::Key_Right:
                    return MoveDirection::GoRight;

                default:
                    return MoveDirection::GoUndefined;
            }
        }
        
        void _runAnimationOnKeyMove(qreal x = 1) {
            
            for(auto &inst : this->_currentMoveInstructions) {
                
                //loss acceptable
                auto castedDelta = (int)(inst.correction * 5 * x);

                //update scrollbar
                inst.affectedScroll->setValue(
                    inst.affectedScroll->value() + castedDelta
                );

            }

            this->onAnimationManipulationTickDone();

        }

        QList<MoveInstruction> _getMoveInstructions(const QSet<MoveDirection> &directions) {
            QList<MoveInstruction> out;

            for(auto &direction : directions) {
                
                MoveInstruction temp;

                switch(direction) {
                    
                    case MoveDirection::GoLeft:
                        temp.affectedScroll = this->_view->horizontalScrollBar();
                        temp.correction = -1;
                    break;

                    case MoveDirection::GoUp:
                        temp.affectedScroll = this->_view->verticalScrollBar();
                        temp.correction = -1;
                    break;

                    case MoveDirection::GoRight:
                        temp.affectedScroll = this->_view->horizontalScrollBar();
                        temp.correction = 1;
                    break;

                    case MoveDirection::GoDown:
                        temp.affectedScroll = this->_view->verticalScrollBar();
                        temp.correction = 1;
                    break;

                    default:
                    break;

                }

                out += temp;

            }

            return out;
        }

        MoveDirection _getOppositeDirection(const MoveDirection &direction) {
            switch(direction) {
                case MoveDirection::GoLeft:
                    return MoveDirection::GoRight;
                break;

                case MoveDirection::GoUp:
                    return MoveDirection::GoDown;
                break;

                case MoveDirection::GoRight:
                    return MoveDirection::GoLeft;
                break;

                case MoveDirection::GoDown:
                    return MoveDirection::GoUp;
                break;

                default:
                    return MoveDirection::GoUndefined;
                break;
            }
        }

};

inline uint qHash(const MV_Manipulation::MoveDirection &key, uint seed = 0) {return uint(key) ^ seed;}
