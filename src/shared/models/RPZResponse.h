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
        RPZResponse(snowflake_uid answererTo, const ResponseCode &code = Ack, const QVariant &data = QVariant());
        RPZResponse(const QVariantHash &hash);

        ResponseCode responseCode() const;
        QVariant responseData() const;
        snowflake_uid answerer() const;
        QString toString() const override;
        QPalette palette() const override;

    private:
        void _setAnswerer(snowflake_uid answererStampableId);
        void _setResponseCode(const ResponseCode &code);
        void _setResponseData(const QVariant &data);
};