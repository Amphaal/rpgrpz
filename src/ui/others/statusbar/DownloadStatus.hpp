#pragma once

#include <QProgressBar>
#include <QLayout>
#include <QLocale>
#include <QStyleOptionProgressBar>

#include "src/shared/async-ui/progress/ProgressTracker.hpp"

class DownloadStatus : public QProgressBar {
    public:
        DownloadStatus() {
            
            this->setVisible(false);
            this->setContentsMargins(0, 0, 0, 0);
            this->setMinimum(0);
            this->setMaximumHeight(10);

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
            
            auto style = new QStyleOptionProgressBar;
            style->initFrom(this);
            
            QColor c;
            switch (kind) {
                case ProgressTracker::Kind::Asset:
                    c = Qt::green;
                    break;
                case ProgressTracker::Kind::Map:
                    c = Qt::red;
            }
            style->palette.setColor(QPalette::Highlight, c);
            
            this->initStyleOption(style);

        }

        static inline QString _tooltipDescriptionTemplate = "%1 : %2/%3";
        static QString kindToText(const ProgressTracker::Kind &kind) {
            switch (kind) {
                case ProgressTracker::Kind::Asset:
                    return "Téléchargement des assets";
                case ProgressTracker::Kind::Map:
                    return "Téléchargement de la map";
                default:
                    return "Téléchargement";
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