#pragma once

#include "ViewMapHint.h"

class MapHint : public ViewMapHint {

    Q_OBJECT

    public:
        MapHint();

        //load/unload
        QString RPZMapFilePath() const;
        bool isRemote() const;
        bool isMapDirty() const;

        static void mayWantToSavePendingState(QWidget* parent, MapHint* hint); //must block UI
        
        bool defineAsRemote(const QString &remoteMapDescriptor = QString());
        double tileToMeterRatio() const;

    public slots:
        bool loadDefaultRPZMap(); //to invoke
        bool loadRPZMap(const QString &filePath); //to invoke
        bool saveRPZMap(); //to invoke, unless from mayWantToSavePendingState()
        bool saveRPZMapAs(const QString &newFilePath); //to invoke
        bool createNewRPZMapAs(const QString &newFilePath); //to invoke

    signals:
        void mapStateChanged(const QString &mapDescriptor, bool isMapDirty);

    private: 
        QString _mapDescriptor;
        bool _isRemote = false;
        bool _isMapDirty = false;

        void _setMapDirtiness(bool dirty = true);
        void _shouldMakeMapDirty(AlterationPayload &payload);
    
        void _handleAlterationRequest(AlterationPayload &payload) final;

        AlterationActor* _sysActor = nullptr;
};