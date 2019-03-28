#include <QtCore/QThread>

#include <QTcpSocket>
#include <QDataStream>
#include <QMessageBox>

class ChatClient : public QObject {
    public:
        ChatClient(QString domain, QString port, QObject* parent = nullptr) : QObject(parent) {
            
            qDebug() << "Chat Client : Instantiation...";
            
            //try connect...
            this->_socket = new QTcpSocket(this);
            
            this->_in.setVersion(QDataStream::Qt_5_12);
            this->_in.setDevice(_socket);

            QObject::connect(
                this->_socket, &QIODevice::readyRead, 
                this, &ChatClient::_onRR
            );
            QObject::connect(
                this->_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
                this, &ChatClient::_displayError
            );

            qDebug() << "Chat Client : Connecting...";    
            this->_socket->abort();
            this->_socket->connectToHost(domain, port.toInt());
        }
    
    private:
        QDataStream _in;
        QTcpSocket* _socket = nullptr;
        QString welcomeMsg;

        //receiving data...
        void _onRR() {

                _in.startTransaction();

                QString result; 
                _in >> result;

                if (!_in.commitTransaction()) {
                    return;
                }
                
                qDebug() << "Chat Client : Data received from server >>" << result;    
                this->welcomeMsg = result;
        }

        void _displayError(QAbstractSocket::SocketError _socketError)
        {
            switch (_socketError) {
                case QAbstractSocket::RemoteHostClosedError:
                    break;
                case QAbstractSocket::HostNotFoundError:
                    QMessageBox::information((QWidget*)this->parent(), tr("Fortune Client"),
                                            tr("The host was not found. Please check the "
                                                "host name and port settings."));
                    break;
                case QAbstractSocket::ConnectionRefusedError:
                    QMessageBox::information((QWidget*)this->parent(), tr("Fortune Client"),
                                            tr("The connection was refused by the peer. "
                                                "Make sure the fortune server is running, "
                                                "and check that the host name and port "
                                                "settings are correct."));
                    break;
                default:
                    QMessageBox::information((QWidget*)this->parent(), tr("Fortune Client"),
                                            tr("The following error occurred: %1.")
                                            .arg(this->_socket->errorString()));
            }
        }
};