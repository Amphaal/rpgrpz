#pragma once

#include "ViewMapHint.h"

class MapHint : public ViewMapHint {

    Q_OBJECT

    public:
        MapHint();

        //load/unload
        QString RPZMapFilePath();
        bool loadDefaultRPZMap();
        bool loadRPZMap(const QString &filePath);
        bool saveRPZMap();
        bool saveRPZMapAs(const QString &newFilePath);
        
        bool isMapDirty();
        void mayWantToSavePendingState();

        bool isRemote();
        bool defineAsRemote(const QString &remoteMapDescriptor = QString());

    signals:
        void mapFileStateChanged(const QString &filePath, bool isMapDirty);
        void mapLoading();
        void resetRequested();

    private: 
        QString _mapFilePath;
        bool _isRemote = false;
        bool _isMapDirty = false;

        void _setMapDirtiness(bool dirty = true);
        void _shouldMakeMapDirty(AlterationPayload &payload);
    
    protected:
        virtual void _handlePayload(AlterationPayload &payload) override;
};