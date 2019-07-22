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
    this->_mapFileLabel = new RPZStatusLabel("Carte");

};

void RPZStatusBar::updateMapFileLabel(const QString &filePath, bool isMapDirty) {
    auto reflectDirtiness = filePath + (isMapDirty ? "*" : "");
    this->_mapFileLabel->updateState(reflectDirtiness, SL_Finished);
}

void RPZStatusBar::_installLayout() {
    
    setUpdatesEnabled(false);

    auto leftPart = new QWidget;
    leftPart->setLayout(new QHBoxLayout);
    leftPart->layout()->setMargin(0);
    leftPart->layout()->addWidget(this->_serverStateLabel);
    leftPart->layout()->addWidget(new QLabel(" | "));
    leftPart->layout()->addWidget(this->_extIpLabel);
    leftPart->layout()->addWidget(new QLabel(" | "));
    leftPart->layout()->addWidget(this->_upnpStateLabel);

    //append components
    this->addPermanentWidget(leftPart);
    this->addPermanentWidget(new QWidget, 1); 
    this->addPermanentWidget(this->_mapFileLabel);

    setUpdatesEnabled(true);
}


///
///
///

void RPZStatusBar::updateServerStateLabel(const QString &stateText, SLState state) {
    this->_serverStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateUPnPLabel(const QString &stateText, SLState state) {
    this->_upnpStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateExtIPLabel(const QString &stateText, SLState state) {
    
    auto asHTMLLink = [stateText]() {
        return "<a href='" + stateText + "'>" + stateText + "</a>";
    };

    this->_extIpLabel->updateState( 
        state == SLState::SL_Finished ? asHTMLLink() : stateText,
        state
    );
}