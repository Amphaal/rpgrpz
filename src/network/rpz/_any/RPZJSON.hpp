#pragma once

#include <QTcpSocket>
#include <QString>
#include <QVariant>

class RPZJSON {
    
    Q_GADGET
    
    public:
        enum class Method {
            Handshake = 0,  
            Message,
            ServerStatus,
            ServerResponse, 
            MapChanged,
            MapChangedHeavily,
            AskForAssets,
            RequestedAsset,
            AudioStreamUrlChanged,
            AudioStreamPlayingStateChanged,
            AudioStreamPositionChanged,
            AvailableAssetsToUpload,
            UserIn,
            UserOut,
            CharacterChanged,
            UserDataChanged,
            QuickDrawHappened,
            GameSessionSync
        };
        Q_ENUM(Method)
       
};