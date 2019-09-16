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

        void serverIsActive() {
            emit serverActive();
        }

        void serverIsInactive() {
            emit serverInactive();
        }

        void clientIsSending() {
            emit clientSending();
        }

        void clientStoppedSending() {
            emit clientSent();
        }

        void clientIsReceiving() {
            emit clientReceiving();
        }

        void clientStoppedReceiving() {
            emit clientReceived();
        }

    signals:
        void heavyAlterationProcessing();
        
        void serverActive();
        void serverInactive();
        
        void clientSending();
        void clientSent();

        void clientReceiving();
        void clientReceived();
    
    private:
        static inline ProgressTracker* _inst = nullptr;

};

        