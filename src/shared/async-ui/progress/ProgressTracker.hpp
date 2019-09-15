#pragma once

#include <QObject>

class ProgressTracker : public QObject {
    
    Q_OBJECT
    
    public:
        static ProgressTracker* get() {
            if(!_inst) _inst = new ProgressTracker;
            return _inst; 
        }

    public slots:
        void heavyAlterationStarted() {
            emit heavyAlterationProcessing();
        }

    signals:
        void heavyAlterationProcessing();
    
    private:
        static inline ProgressTracker* _inst = nullptr;

};

        