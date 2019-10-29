#pragma once

#include "src/shared/models/_base/Stampable.hpp"

class RPZResponse : public Stampable {
    public:
        enum class ResponseCode {
            UnknownCommand, 
            HelpManifest, 
            Ack, 
            Error,
            ErrorRecipients,
            ConnectedToServer, 
            Status
        };
        
        RPZResponse();
        RPZResponse(Stampable::Id answererTo, const ResponseCode &code = ResponseCode::Ack, const QVariant &data = QVariant());
        explicit RPZResponse(const QVariantHash &hash);

        ResponseCode responseCode() const;
        QVariant responseData() const;
        Stampable::Id answerer() const;
        QString toString() const override;
        QPalette palette() const override;

    private:
        void _setAnswerer(Stampable::Id answererStampableId);
        void _setResponseCode(const ResponseCode &code);
        void _setResponseData(const QVariant &data);
};