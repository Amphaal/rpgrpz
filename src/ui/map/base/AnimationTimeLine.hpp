#pragma once

#include <functional>
#include <QTimeLine>
#include <QHash>

class AnimationTimeLine : public QTimeLine {

    Q_OBJECT

    public:
        enum Type { Zoom, HorizontalMove, VerticalMove };

        void setStateModifier(const qreal modifier) { this->_currentModifier = modifier; };

        static AnimationTimeLine* use(const AnimationTimeLine::Type &controllerType,  qreal stateModifier,  QObject* parent, std::function<void(qreal, qreal)> onValueChanged) {
            
            AnimationTimeLine* handler = nullptr;

            //create from animation type
            if(!_handlers.contains(controllerType)) {
                
                //create
                handler = new AnimationTimeLine(stateModifier, parent, onValueChanged);

                //add
                _handlers.insert(controllerType, handler);

            } else {

                //get existing one and update modifier
                handler = _handlers[controllerType];
                handler->setStateModifier(stateModifier);

            }

            //start or restart handler
            if(handler->state() != QTimeLine::Running) handler->start();
            return handler;
        }




    private:
        //static
        static inline QHash<AnimationTimeLine::Type, AnimationTimeLine*> _handlers = QHash<AnimationTimeLine::Type, AnimationTimeLine*>();


        //
        AnimationTimeLine(qreal stateModifier, QObject* parent, std::function<void(qreal, qreal)> onValueChanged) : 
        QTimeLine(300, parent), 
        _currentModifier(stateModifier) {  
            
            //self state, x15 animations
            this->setUpdateInterval(20);

            //bind
            QObject::connect(
                this, &QTimeLine::valueChanged,
                [&, onValueChanged](qreal value) {
                    onValueChanged(this->_currentModifier, value);
                }
            );

        };

        qreal _currentModifier = 0;

};