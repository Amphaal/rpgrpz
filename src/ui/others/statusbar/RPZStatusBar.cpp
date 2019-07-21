#include "RPZStatusBar.h"

RPZStatusBar::RPZStatusBar(QWidget * parent) : QStatusBar(parent) {

    this->setContentsMargins(10, 0, 3, 0);
    this->setAutoFillBackground(true);

    //colors
    auto colors = this->palette();
    colors.setColor(QPalette::Window, "#DDD");
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
    this->_mapFileLabel = new QLabel();

};

void RPZStatusBar::updateMapFileLabel(const QString &filePath, bool isMapDirty) {
    this->_mapFileLabel->setText(filePath + (isMapDirty ? "*" : ""));
}

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

    this->addWidget(this->_mapFileLabel);
}


///
///
///

void RPZStatusBar::updateServerStateLabel(const QString &stateText, int state) {
    this->_serverStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateUPnPLabel(const QString &stateText, int state) {
    this->_upnpStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateExtIPLabel(const QString &stateText, int state) {
    
    auto asHTMLLink = [stateText]() {
        return "<a href='" + stateText + "'>" + stateText + "</a>";
    };

    this->_extIpLabel->updateState( 
        (RPZStatusLabel::State)state == RPZStatusLabel::State::Finished ? asHTMLLink() : stateText,
        state
    );
}