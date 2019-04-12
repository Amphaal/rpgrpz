#pragma once

#include <functional>
#include <QTimeLine>

class AnimationTimeLine : public QTimeLine {

    Q_OBJECT

    signals:
        void stepPassed(int* stateController, const qreal step);

    public:
        AnimationTimeLine(int* stateController, int stateModifier, QObject* parent) : 
            QTimeLine(350, parent), 
            _stateController(stateController) {
            
            //self state
            this->setUpdateInterval(20);

            //on events
            QObject::connect(this, &QTimeLine::valueChanged, [&](const qreal x) {
                emit stepPassed(this->_stateController, x);
            });
            QObject::connect(this, &QTimeLine::finished, this, &AnimationTimeLine::_onFinished);

            // if user moved the wheel in another direction, we reset previously scheduled scalings
            *this->_stateController += stateModifier;
            if (*this->_stateController * stateModifier < 0) {
                *this->_stateController = stateModifier; 
            }
        };

    private:
        int* _stateController;
        void _onFinished() {
            if (*this->_stateController > 0) {
                *this->_stateController--;
            } else {
                *this->_stateController++;
                this->sender()->deleteLater();
            }
        }
};