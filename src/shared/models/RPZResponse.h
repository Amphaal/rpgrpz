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
        RPZResponse(const snowflake_uid &answererTo, const ResponseCode &code = Ack, const QVariant &data = QVariant());
        RPZResponse(const QVariantHash &hash);

        ResponseCode responseCode();
        QVariant responseData();
        snowflake_uid answerer();
        QString toString() override;
        QPalette palette() override;

    private:
        void _setAnswerer(const snowflake_uid &answererStampableId);
        void _setResponseCode(const ResponseCode &code);
        void _setResponseData(const QVariant &data);
};