#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

#include <QString>
#include <QMovie>

enum SLState {SL_Finished = 0, SL_Waiting, SL_Processing, SL_Error };

class RPZStatusLabel : public QWidget {

    Q_OBJECT

    public:
        RPZStatusLabel(QString descriptor, QWidget* parent = nullptr);

        void updateState(const QString &stateText, SLState state = SL_Finished);
        void setWaiting(bool waiting);
        QLabel* dataLabel();

    private:
        QLabel* _descriptorLbl = nullptr;
        QLabel* _dataLbl = nullptr;
        QLabel* _loaderLbl = nullptr;
        QMovie* _loader = nullptr;
       
};