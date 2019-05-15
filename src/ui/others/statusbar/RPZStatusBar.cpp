#include "RPZStatusBar.h"

RPZStatusBar::RPZStatusBar(QWidget * parent) : QStatusBar(parent) {
    
    qDebug() << "UI : StatusBar instantiation";

    this->setContentsMargins(10, 5, 10, 5);
    this->setAutoFillBackground(true);

    //colors
    auto colors = this->palette();
    colors.setColor(QPalette::Background, "#DDD");
    this->setPalette(colors);

    //install...
    this->_installComponents();
    this->_installLayout();
}

void RPZStatusBar::_installComponents() {

    //ext ip
    this->_extIpLabel = new RPZStatusLabel("IP externe");
    this->_extIpLabel->setToolTip("Copier l'IP");
    QObject::connect(
        this->_extIpLabel->dataLabel(), &QLabel::linkActivated, 
        [](const QString &link) {

            //remove html tags
            auto s = link;
            s.remove(QRegExp("<[^>]*>"));
            QApplication::clipboard()->setText(s);

            //show tooltip
            QToolTip::showText(QCursor::pos(), "IP copiée !");
        }
    );

    this->_upnpStateLabel = new RPZStatusLabel("uPnP");
    this->_serverStateLabel = new RPZStatusLabel("Serveur");
    this->_sentAudioKbpsStateLabel = new RPZStatusLabel("Envoyé");
    this->_receivedAudioKbpsStateLabel = new RPZStatusLabel("Reçu");

};

void RPZStatusBar::_installLayout() {

    auto addSeparator = [&]() {
        this->addWidget(new QLabel(" | "));
    };

    //append components
    this->addWidget(this->_serverStateLabel);
    addSeparator();
    this->addWidget(this->_extIpLabel);
    addSeparator();
    this->addWidget(this->_upnpStateLabel);
    
    this->addWidget(new QWidget, 1); 
    
    this->addWidget(this->_sentAudioKbpsStateLabel);
    addSeparator();
    this->addWidget(this->_receivedAudioKbpsStateLabel);

}

///
///
///

void RPZStatusBar::updateSentAudioKbps(double kbps, int clients) {
    QString lbl(QString::number(kbps) + " kbps / " + QString::number(clients) + " client(s)");
    this->_sentAudioKbpsStateLabel->updateState(lbl);
}

void RPZStatusBar::updateReceivedAudioKbps(double kbps) {
    QString lbl(QString::number(kbps) + " kbps");
    this->_receivedAudioKbpsStateLabel->updateState(lbl);
}

void RPZStatusBar::updateServerStateLabel(const std::string &stateText, int state) {
    this->_serverStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateUPnPLabel(const std::string &stateText, int state) {
    this->_upnpStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateExtIPLabel(const std::string &stateText, int state) {
    
    auto asHTMLLink = [stateText]() {
        return "<a href='" + stateText + "'>" + stateText + "</a>";
    };

    this->_extIpLabel->updateState( 
        (RPZStatusLabel::State)state == RPZStatusLabel::State::Finished ? asHTMLLink() : stateText,
        state
    );
}