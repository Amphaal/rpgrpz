#pragma once

#include <QTcpSocket>
#include <QString>
#include <QVariant>

class RPZJSON {
    
    Q_GADGET
    
    public:
        enum class Method { 
            Message,
            ServerStatus,
            ServerResponse, 
            Handshake, 
            AllConnectedUsers, 
            ChatLogHistory,
            MapChanged,
            AckIdentity,
            AskForAssets,
            RequestedAsset,
            AudioStreamUrlChanged,
            AudioStreamPlayingStateChanged,
            AudioStreamPositionChanged,
            MapChangedHeavily,
            AvailableAssetsToUpload,
            UserIn,
            UserOut,
            CharacterChanged,
            UserDataChanged,
            QuickDrawHappened
        };
        Q_ENUM(Method)
       
};