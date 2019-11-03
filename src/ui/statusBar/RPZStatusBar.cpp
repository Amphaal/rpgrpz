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
    this->_extIpLabel = new RPZStatusLabel(tr("External IP"));
    this->_extIpLabel->setToolTip(tr("Copy IP"));
    QObject::connect(
        this->_extIpLabel->dataLabel(), &QLabel::linkActivated, 
        [](const QString &link) {

            //remove html tags
            auto s = link;
            s.remove(QRegExp("<[^>]*>"));
            QApplication::clipboard()->setText(s);

            //show tooltip
            QToolTip::showText(QCursor::pos(), tr("IP copied !"));
            
        }
    );

    this->_mapFileLabel = new RPZStatusLabel(tr("Map"));
    this->_upnpStateLabel = new RPZStatusLabel("uPnP");
    
    this->_serverStateLabel = new RPZStatusLabel(tr("Server"));
    this->_dlStatus = new DownloadStatus;
    this->_activityIndicators = new ClientActivityIndicator;
};

void RPZStatusBar::updateMapFileLabel(const QString &mapDescriptor, bool isMapDirty) {
    auto reflectDirtiness = mapDescriptor + (isMapDirty ? "*" : "");
    this->_mapFileLabel->updateState(reflectDirtiness);
}

void RPZStatusBar::connectingToServer() {
    this->_activityIndicators->setVisible(true);
}

void RPZStatusBar::connectionClosed() {
    this->_activityIndicators->setVisible(false);
    this->_dlStatus->setVisible(false);
}

void RPZStatusBar::bindServerIndicators() {

    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::serverActive,
        [=]() {
            this->_serverStateLabel->dataLabel()->setStyleSheet("color:green");
        }
    );

    QObject::connect(
        ProgressTracker::get(), &ProgressTracker::serverInactive,
        [=]() {
            this->_serverStateLabel->dataLabel()->setStyleSheet("color:black");
        }
    );

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
    this->addPermanentWidget(this->_dlStatus);
    this->addPermanentWidget(this->_activityIndicators);

    setUpdatesEnabled(true);
    
}


///
///
///

void RPZStatusBar::updateServerStateLabel(const QString &stateText, RPZStatusLabel::State state) {
    this->_serverStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateUPnPLabel(const QString &stateText, RPZStatusLabel::State state) {
    this->_upnpStateLabel->updateState(
        stateText, 
        state
    );
}

void RPZStatusBar::updateExtIPLabel(const QString &stateText, RPZStatusLabel::State state) {
    
    auto asHTMLLink = [stateText]() {
        return "<a href='" + stateText + "'>" + stateText + "</a>";
    };

    this->_extIpLabel->updateState( 
        state == RPZStatusLabel::State::Finished ? asHTMLLink() : stateText,
        state
    );
}