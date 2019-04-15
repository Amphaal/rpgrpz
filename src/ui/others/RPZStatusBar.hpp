#pragma once

#include <QStatusBar>
#include <QDebug>
#include <QLabel>
#include <QClipboard>
#include <QApplication>
#include <QToolTip>

class RPZStatusBar : public QStatusBar {

    Q_OBJECT

    public:
        RPZStatusBar(QWidget * parent = nullptr) : QStatusBar(parent) {
            
            qDebug() << "UI : StatusBar instantiation";

            this->setContentsMargins(10, 5, 10, 5);
            this->setAutoFillBackground(true);

            //colors
            auto colors = this->palette();
            colors.setColor(QPalette::Background, "#DDD");
            this->setPalette(colors);

            this->_installComponents();
        }

    public slots:
        void updateSentAudioKbps(double kbps, int clients) {
            QString lbl(QString::number(kbps) + " kbps / " + QString::number(clients) + " client(s)");
            this->_sentAudioKbpsStateLabel->setText(lbl);
        }

        void updateReceivedAudioKbps(double kbps) {
            QString lbl(QString::number(kbps) + " kbps");
            this->_receivedAudioKbpsStateLabel->setText(lbl);
        }

        void updateServerStateLabel(const std::string &state) {
            this->_serverStateLabel->setText(QString::fromStdString(state));
        }

        void updateUPnPLabel(const std::string &state) {
            this->_upnpStateLabel->setText(QString::fromStdString(state));
        }

        void updateExtIPLabel(const std::string &state, const bool isOn) {
            
            this->_extIpLabel->setText(QString::fromStdString(state));

            if(isOn) {
                this->_extIpLabel->setText("<a href='" + this->_extIpLabel->text() + "'>" + this->_extIpLabel->text() + "</a>");
            }
        }

    private:
        QLabel* _extIpLabel;
        QLabel* _upnpStateLabel;
        QLabel* _serverStateLabel;
        QLabel* _sentAudioKbpsStateLabel;
        QLabel* _receivedAudioKbpsStateLabel;

        void _installComponents() {
            //descr helpers
                auto serverDescrLabel = new QLabel("Serveur:");
                auto extIpDescrLabel = new QLabel("IP externe:");
                auto upnpDescrLabel = new QLabel("uPnP:");
                auto audioDescrLabel = new QLabel("Audio:");
                auto audioSentDescrLabel = new QLabel("Envoyé -");
                auto audioReceivedDescrLabel = new QLabel(", Reçu -");

                //data wielder
                auto syncMsg = "<En attente...>";
                this->_extIpLabel = new QLabel(syncMsg);
                this->_upnpStateLabel = new QLabel(syncMsg);
                this->_serverStateLabel = new QLabel(syncMsg);
                this->_sentAudioKbpsStateLabel = new QLabel(syncMsg);
                this->_receivedAudioKbpsStateLabel = new QLabel(syncMsg);

                //on external IP click
                this->_extIpLabel->setToolTip("Copier l'IP");
                QObject::connect(
                    this->_extIpLabel, &QLabel::linkActivated, 
                    this, &RPZStatusBar::_onExtIpClicked
                );

                //append components
                this->addWidget(serverDescrLabel);
                this->addWidget(this->_serverStateLabel);
                this->addWidget(this->_getSep());
                this->addWidget(extIpDescrLabel);
                this->addWidget(this->_extIpLabel);
                this->addWidget(this->_getSep());
                this->addWidget(upnpDescrLabel);
                this->addWidget(this->_upnpStateLabel);
                
                this->addWidget(new QWidget, 1); 
                
                this->addWidget(audioDescrLabel);
                this->addWidget(audioSentDescrLabel);
                this->addWidget(this->_sentAudioKbpsStateLabel);
                this->addWidget(audioReceivedDescrLabel);
                this->addWidget(this->_receivedAudioKbpsStateLabel);
        };

        void _onExtIpClicked() {

            //remove html tags
            auto s = this->_extIpLabel->text();
            s.remove(QRegExp("<[^>]*>"));
            QApplication::clipboard()->setText(s);

            //show tooltip
            QToolTip::showText(QCursor::pos(), "IP copiée !");
        }

        QLabel* _getSep() {
            return new QLabel(" | ");
        }
}; 