// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2021 Guillaume Vara <guillaume.vara@gmail.com>

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

#include <QLineEdit>
#include <QKeyEvent>

class LineEditHistoriable : public QLineEdit {
 public:
    void addTextToHistory() {
        // prevent saving "empty" input
        auto trimmed = this->text().trimmed();
        if(trimmed.isEmpty()) return;

        // prevent resaving the same input
        if(_history.count() > 0 && _history.front() == trimmed) return;

        // push at begin
        _history.push_front(trimmed);
    }

    void clearAndResetHistoryIndex() {
        this->clear();
        this->_historyIndex = -1;
    }

 protected:
    void keyReleaseEvent(QKeyEvent *event) override {
        switch(event->key()) {
            case Qt::Key::Key_Up : {
                _historyUp();
            }
            break;

            case Qt::Key::Key_Down : {
                _historyDown();
            }
            break;

            default : {
                QLineEdit::keyReleaseEvent(event);
            }
        }
    }

 private:
    QList<QString> _history;
    int _historyIndex = -1;

    void _historyUp() {
        if(_historyIndex < _history.size() - 1) _historyIndex++;
        else
            return;
        _fetchFromHistory();
    }

    void _historyDown() {
        if(_historyIndex > 0) _historyIndex--;
        else
            return;
        _fetchFromHistory();
    }

    void _fetchFromHistory() {
        auto &command = _history.at(_historyIndex);
        this->setText(command);
    }
};
