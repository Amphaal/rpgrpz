#pragma once

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QList>
#include "src/helpers/_const.cpp"

class LogScrollView : public QWidget {

    public:
        LogScrollView(QWidget *parent = nullptr);
        void addMessage(const std::string & newMessage, QPalette* colorPalette = nullptr);
        void updateLatestMessage(const std::string & newMessage);

            
    private:
        int _maxLogMessages = 10000;
        void limitLogSize();
};