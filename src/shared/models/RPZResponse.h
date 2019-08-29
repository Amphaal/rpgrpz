#pragma once

#include "base/Stampable.hpp"

class RPZResponse : public Stampable {
    public:
        enum ResponseCode {
            UnknownCommand, 
            HelpManifest, 
            Ack, 
            Error,
            ErrorRecipients,
            ConnectedToServer, 
            Status
        };
        
        RPZResponse();
        RPZResponse(RPZStampableId answererTo, const ResponseCode &code = Ack, const QVariant &data = QVariant());
        RPZResponse(const QVariantHash &hash);

        ResponseCode responseCode() const;
        QVariant responseData() const;
        RPZStampableId answerer() const;
        QString toString() const override;
        QPalette palette() const override;

    private:
        void _setAnswerer(RPZStampableId answererStampableId);
        void _setResponseCode(const ResponseCode &code);
        void _setResponseData(const QVariant &data);
};