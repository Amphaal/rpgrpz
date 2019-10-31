#pragma once

#include <QTcpSocket>
#include <QString>
#include <QVariant>

class RPZJSON : public QObject {
    
    Q_OBJECT
    
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
            UserDataChanged
        };
        Q_ENUM(Method)
       
};

Q_DECLARE_METATYPE(RPZJSON::Method)