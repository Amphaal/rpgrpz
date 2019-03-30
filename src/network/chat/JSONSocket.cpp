#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

class JSONSocket : public QTcpSocket {

    Q_OBJECT

    public:
        JSONSocket(QString logId) : _logId(logId) {

            this->_in = new QDataStream;
            this->_in->setVersion(QDataStream::Qt_5_12);
            this->_in->setDevice(this);

            QObject::connect(
                this, &QIODevice::readyRead,
                this, &JSONSocket::_processIncomingData
            );
        }

        void sendJSON(QString method, QVariant data) {
            
            //checks
            if(method.isEmpty() || data.isNull()) {
                return;
            }

            //format document
            auto json_data = QJsonDocument::fromVariant(data);
            auto dataAsString = QString(json_data.toJson(QJsonDocument::Compact));
            auto peeet = QString("{\"_m\":" + method + ",\"_d\":" + dataAsString + "}");
            auto json_payload = QJsonDocument::fromJson(peeet.toUtf8());

            qDebug() << _logId << " : json to be sent >> " << method << "<<";

            //send !
            this->_sendJSONAsBinary(json_payload.toBinaryData());
        }

    signals:
        void JSONReceived(QString method, QVariant data);


    private:
        QString _logId;
        QDataStream* _in = nullptr;

        void _sendJSONAsBinary(QByteArray data) {
            
            //send welcome message
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_5_12);

            //send...
            out << data;
            auto written = this->write(block);

            //wait end send
            this->waitForBytesWritten();

            qDebug() << _logId << " : json sent !";
        }

        void _processIncomingData() {
            
            //process incoming data
            _in->startTransaction();

            QByteArray block;
            (*_in) >> block;

            if (!_in->commitTransaction()) {
                qWarning() << _logId << " : issue while reading incoming data";  
                return;
            }

            this->_processIncomingAsJson(block);

        }

        void _processIncomingAsJson(QByteArray data) {

            //parse to json
            auto json = QJsonDocument::fromBinaryData(data);
            if(json.isNull()) {
                qWarning() << _logId << " : Data received was not JSON and thus cannot be read.";
                return;
            }

            //prepare
            if(!json.isObject()) {
                qWarning() << _logId << " : JSON received is not Object and thus cannot be handled.";
                return;
            }

            auto content = json.object();
            auto mainKeys = content.keys();

            //check requirements
            auto required = mainKeys.contains("_m") && mainKeys.contains("_d");
            if(!required) {
                qWarning() << _logId << " : JSON received has missing keys and thus cannot be handled.";
                return;
            }

            //check value types
            auto ss = content["_d"];
            auto okTypes = content["_m"].isString();
            if(!okTypes) {
                qWarning() << _logId << " : JSON received has unexpected data and thus cannot be handled.";
                return;
            }

            auto method = content["_m"].toString();
            emit JSONReceived(method, content["_m"].toVariant());

            qDebug() << _logId << " : json received >> " << method << "<<";
        }
};