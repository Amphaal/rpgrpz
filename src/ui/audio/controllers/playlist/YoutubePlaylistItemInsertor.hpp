#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QToolTip>
#include <QPushButton>

#include <QHBoxLayout>
#include <QIcon>
#include <QRegularExpression>

#include "src/helpers/_appContext.h"

class YoutubePlaylistItemInsertor : public QWidget {
    
    Q_OBJECT

    signals:
        void insertionRequested(const QString &ytUrl);

    private:
        QLineEdit* _ytUrlEdit = nullptr;
        QPushButton* _insertLinkBtn = nullptr;
        QRegularExpression* _ytUrlMatcher = nullptr;

        void _handleLinkInsertionAttempt() {
            auto url = this->_ytUrlEdit->text();
            this->_ytUrlEdit->clear();
    
            auto match = this->_ytUrlMatcher->match(url);
            auto ss = match.isValid();
            if(!match.hasMatch()) {
                QToolTip::showText(this->_ytUrlEdit->mapToGlobal(QPoint()), tr("Invalid Youtube URL !"));
                return;
            }

            emit insertionRequested(url);
        }

    public:
        YoutubePlaylistItemInsertor(QWidget *parent = nullptr) : QWidget(parent), 
            _ytUrlEdit(new QLineEdit),
            _insertLinkBtn(new QPushButton),
            _ytUrlMatcher(new QRegularExpression(AppContext::REGEX_YOUTUBE_URL)) {
            
            this->setLayout(new QHBoxLayout);

            this->_ytUrlEdit->setPlaceholderText(tr("Paste here a Youtube URL..."));
            this->_ytUrlEdit->setToolTip(this->_ytUrlEdit->placeholderText());
            QObject::connect(
                this->_ytUrlEdit, &QLineEdit::returnPressed, 
                this, &YoutubePlaylistItemInsertor::_handleLinkInsertionAttempt
            );

            this->_insertLinkBtn->setIcon(QIcon(QStringLiteral(u":/icons/app/other/add.png")));
            this->_insertLinkBtn->setToolTip(tr("Click to add to playlist"));

            QObject::connect(
                this->_insertLinkBtn, &QPushButton::clicked,
                this, &YoutubePlaylistItemInsertor::_handleLinkInsertionAttempt
            );

            this->layout()->addWidget(this->_ytUrlEdit);
            this->layout()->addWidget(this->_insertLinkBtn);
        }
    



};