#pragma once

#include <QProgressBar>
#include <QLayout>
#include <QLocale>
#include <QStyleOptionProgressBar>

#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class DownloadStatus : public QProgressBar {

    Q_OBJECT

    public:
        DownloadStatus() {
            
            this->setVisible(false);
            this->setContentsMargins(0, 0, 0, 0);
            this->setMinimum(0);
            this->setMaximumHeight(8);

            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::downloadStarted,
                [=](const ProgressTracker::Kind &kind, qint64 size) { 

                    auto tempDescr = _tooltipDescriptionTemplate.arg(kindToText(kind))
                        .arg("%1")
                        .arg(valToText(kind, size));
                    
                    //store prefilled
                    this->_tooltipDescriptionTemplatePrefilled = tempDescr;

                    //add missing value
                    tempDescr = tempDescr.arg(valToText(kind, 0));
                    this->setToolTip(tempDescr);
                    
                    //set style
                    this->_setStyleForKind(kind);

                    //at last set visible
                    this->setMaximum((int)size);
                    this->setValue(0);
                    this->setVisible(true);

                }
            );

            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::downloadEnded,
                [=](const ProgressTracker::Kind &kind) { 
                    this->setVisible(false); 
                }
            );

            QObject::connect(
                ProgressTracker::get(), &ProgressTracker::downloadProgress,
                [=](const ProgressTracker::Kind &kind, qint64 progress) { 
                    
                    auto toolTipText = this->_tooltipDescriptionTemplatePrefilled
                                                .arg(valToText(kind, progress));

                    this->setToolTip(toolTipText);

                    this->setValue((int)progress);

                }
            );

        }
    
    private:
        QString _tooltipDescriptionTemplatePrefilled;

        void _setStyleForKind(const ProgressTracker::Kind &kind) {
            
            QString style;

            switch (kind) {
                case ProgressTracker::Kind::Asset:
                    // style += "QProgressBar { border: 2px solid grey; border-radius: 5px;}";
                    // style += "QProgressBar::chunk {background-color: #05B8CC; width: 20px;}";
                    break;
                case ProgressTracker::Kind::Map:
                    //default style
                    break;
            }

            this->setStyleSheet(style);

        }

        static inline QString _tooltipDescriptionTemplate = "%1 : %2/%3";
        static QString kindToText(const ProgressTracker::Kind &kind) {
            switch (kind) {
                case ProgressTracker::Kind::Asset:
                    return tr("Assets donwload");
                case ProgressTracker::Kind::Map:
                    return tr("Map download");
                default:
                    return tr("Download");
                    break;
            }
        }

        static QString valToText(const ProgressTracker::Kind &kind, unsigned long long size) {
            switch (kind) {
                case ProgressTracker::Kind::Map:
                    return QLocale::system().formattedDataSize(size);
                default:
                    return QString::number(size);
            }
        }
};