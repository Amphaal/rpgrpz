#pragma once

#include <functional>
#include <QTimeLine>
#include <QHash>

class AnimationTimeLine : public QTimeLine {
    public:
        enum class Type { Zoom };
        Q_ENUM(Type)

        void setStateModifier(const qreal modifier) { this->_currentModifier = modifier; };

        static AnimationTimeLine* use(
            const AnimationTimeLine::Type &controllerType, 
            qreal stateModifier, 
            std::function<void(qreal, qreal)> onValueChanged
        ) {
            
            AnimationTimeLine* handler = nullptr;

            //create from animation type
            if(!_handlers.contains(controllerType)) {
                
                //create
                handler = new AnimationTimeLine(stateModifier, onValueChanged);

                //add
                _handlers.insert(controllerType, handler);

            } else {

                //get existing one and update modifier
                handler = _handlers.value(controllerType);
                handler->setStateModifier(stateModifier);

            }

            //start or restart handler
            if(handler->state() != QTimeLine::State::Running) {
                handler->start();
            } else {
                handler->stop();
                handler->setCurrentTime(
                    handler->duration()
                );
                handler->start();
            }

            return handler;
        }



    private:
        //static
        static inline QHash<AnimationTimeLine::Type, AnimationTimeLine*> _handlers;

        AnimationTimeLine(qreal stateModifier, std::function<void(qreal, qreal)> onValueChanged) : 
        QTimeLine(300), 
        _currentModifier(stateModifier) {  
            
            //set to 30fps
            this->setUpdateInterval(10);
            this->setDirection(QTimeLine::Direction::Backward);
            this->setCurveShape(QTimeLine::CurveShape::EaseOutCurve);

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

inline uint qHash(const AnimationTimeLine::Type &key, uint seed = 0) {return uint(key) ^ seed;}
