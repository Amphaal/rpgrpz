#pragma once

#include "ViewMapHint.h"

class MapHint : public ViewMapHint {

    Q_OBJECT

    public:
        MapHint(QGraphicsView* boundGv);

        //load/unload
        QString mapFilePath();
        bool loadDefaultMap();
        bool loadMap(const QString &filePath);
        bool saveMap();
        bool saveMapAs(const QString &newFilePath);
        
        bool isMapDirty();
        void mayWantToSavePendingState();

        bool isRemote();
        bool defineAsRemote(const QString &remoteMapDescriptor = QString());

    signals:
        void mapFileStateChanged(const QString &filePath, bool isMapDirty);
    
    private: 
        QString _mapFilePath;
        bool _isRemote = false;
        bool _isMapDirty = false;

        void _setMapDirtiness(bool dirty = true);
        void _shouldMakeMapDirty(AlterationPayload &payload);
    
    protected:
        virtual void _handlePayload(AlterationPayload &payload) override;
};