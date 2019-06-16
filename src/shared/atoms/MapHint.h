#pragma once

#include "ViewMapHint.h"

class MapHint : public ViewMapHint {
    public:
        MapHint(QGraphicsView* boundGv);

        //load/unload
        QString stateFilePath();
        bool loadDefaultState();
        bool loadState(QString &filePath);
        bool saveState();
        bool saveStateAs(QString &newFilePath);
        
        bool isDirty();
        void mayWantToSavePendingState();

        bool isRemote();
        bool defineAsRemote(QString &remoteMapDescriptor = QString());

    signals:
        void mapFileStateChanged(const QString &filePath, bool isDirty);
    
    private: 
        QString _stateFilePath;
        bool _isRemote = false;
        bool _isDirty = false;

        void _setDirty(bool dirty = true);
        void _shouldMakeDirty(AlterationPayload &payload);
    
    protected:
        virtual void _handlePayload(AlterationPayload &payload) override;
};