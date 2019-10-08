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
                QToolTip::showText(this->_ytUrlEdit->mapToGlobal(QPoint()), "Lien Youtube incorrect!");
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

            this->_ytUrlEdit->setPlaceholderText("Coller ici un lien de video Youtube...");
            this->_ytUrlEdit->setToolTip("Coller ici un lien de video Youtube...");
            QObject::connect(
                this->_ytUrlEdit, &QLineEdit::returnPressed, 
                this, &YoutubePlaylistItemInsertor::_handleLinkInsertionAttempt
            );

            this->_insertLinkBtn->setIcon(QIcon(":/icons/app/other/add.png"));
            this->_insertLinkBtn->setToolTip("Cliquer pour ajouter un lien à la liste de lecture");

            QObject::connect(
                this->_insertLinkBtn, &QPushButton::clicked,
                this, &YoutubePlaylistItemInsertor::_handleLinkInsertionAttempt
            );

            this->layout()->addWidget(this->_ytUrlEdit);
            this->layout()->addWidget(this->_insertLinkBtn);
        }
    



};