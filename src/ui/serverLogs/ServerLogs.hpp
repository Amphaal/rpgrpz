// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>

#include "src/ui/_others/ConnectivityObserver.h"
#include "src/network/rpz/server/RPZServer.h"

class ServerLogs : public QDialog {
    Q_OBJECT

 public:
    explicit ServerLogs(RPZServer* toListenTo, QWidget * parent = nullptr) : QDialog(parent), _loglist(new QTableWidget(0, 4, this)) {
        this->setWindowTitle(tr("Server logs"));

        this->_initTable();

        this->setMinimumWidth(450);
        this->setLayout(new QVBoxLayout);
        this->layout()->addWidget(this->_loglist);

        QObject::connect(
            toListenTo, &RPZServer::startUploadToClient,
            this, &ServerLogs::_onStartUploadToClient
        );

        QObject::connect(
            toListenTo, &RPZServer::uploadingToClient,
            this, &ServerLogs::_onUploadingToClient
        );

        QObject::connect(
            toListenTo, &RPZServer::uploadedToClient,
            this, &ServerLogs::_onUploadedToClient
        );
    }

 private slots:
    void _onStartUploadToClient(RPZJSON::Method method, qint64 totalToUpload, const RPZUser &sentTo) {
        this->_loglist->insertRow(0);
        _waitingRow++;

        // package
        auto packageItem = new QTableWidgetItem;
        auto methodToStr = QVariant::fromValue(method).toString();
        packageItem->setText(methodToStr);
        this->_loglist->setItem(0, 0, packageItem);

        // target
        auto targetItem = new QTableWidgetItem;
        targetItem->setText(sentTo.toString());
        this->_loglist->setItem(0, 1, targetItem);

        // completion
        auto completionItem = new QTableWidgetItem;
        completionItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        completionItem->setData(ExpectedUploadedRole, totalToUpload);
        this->_loglist->setItem(0, 2, completionItem);

        // time
        auto cdt = QDateTime::currentDateTime();
        auto timeItem = new QTableWidgetItem;
        timeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        timeItem->setData(StartedAtRole, cdt);
        this->_loglist->setItem(0, 3, timeItem);

        //
        this->_updateUploadCompletionText(0, 0);
        this->_updateTimeText(cdt, 0);
    }

    void _onUploadingToClient(qint64 bytesUploaded) {
        this->_updateUploadCompletionText(bytesUploaded, _waitingRow);
        this->_updateTimeText();
    }

    void _onUploadedToClient() {
        this->_updateUploadCompletionText();
        this->_updateTimeText();
        if (_waitingRow > -1) _waitingRow--;
    }

 private:
    QTableWidget* _loglist = nullptr;
    static inline int CurrentUploadedRole = 257;
    static inline int ExpectedUploadedRole = 258;
    static inline int StartedAtRole = 259;
    int _waitingRow = -1;

    void _updateTimeText() {
        _updateTimeText(QDateTime::currentDateTime(), _waitingRow);
    }

    void _initTable() {
        this->_loglist->setHorizontalHeaderLabels({
            tr("Package Type"),
            tr("Targeted User"),
            tr("Upload State"),
            tr("Elapsed Time")
        });
        this->_loglist->setEditTriggers(QAbstractItemView::NoEditTriggers);
        this->_loglist->verticalHeader()->setVisible(false);
        this->_loglist->horizontalHeader()->setStretchLastSection(false);
        this->_loglist->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
        this->_loglist->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
        this->_loglist->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::Stretch);
        this->_loglist->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::Stretch);
    }

    void _updateTimeText(const QDateTime &currentTime, int rowToUpdate) {
        auto item = this->_loglist->item(rowToUpdate, 3);  // time
        auto startedAt = item->data(StartedAtRole).toDateTime();
        auto elapsed = startedAt.secsTo(currentTime);
        item->setText(QStringLiteral("%1s").arg(elapsed));
    }

    void _updateUploadCompletionText() {
        auto item = this->_loglist->item(this->_waitingRow, 2);  // completion
        auto expectedUploaded = item->data(ExpectedUploadedRole).toDouble();
        // update text
        auto locale = QLocale::system();
        auto text = QStringLiteral("%1 / %2")
            .arg(locale.formattedDataSize(expectedUploaded))
            .arg(locale.formattedDataSize(expectedUploaded));
        item->setText(text);
    }

    void _updateUploadCompletionText(qint64 uploadedBytes, int rowToUpdate) {
        auto item = this->_loglist->item(rowToUpdate, 2);  // completion

        // fetch current
        auto currentUploaded = item->data(CurrentUploadedRole).toDouble();
        auto expectedUploaded = item->data(ExpectedUploadedRole).toDouble();

        // update inner value
        currentUploaded += uploadedBytes;
        item->setData(CurrentUploadedRole, currentUploaded);

        // update text
        auto locale = QLocale::system();
        auto text = QStringLiteral("%1/%2")
            .arg(locale.formattedDataSize(currentUploaded))
            .arg(locale.formattedDataSize(expectedUploaded));
        item->setText(text);
    }
};
