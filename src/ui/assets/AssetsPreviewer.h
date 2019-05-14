#pragma once

#include <QGroupBox>
#include <QDebug>
#include <QFileInfo>
#include <QImage>
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>

#include <QVBoxLayout>
#include <QPixmap>
#include <QLabel>

class AssetsPreviewer : public QGroupBox {

    Q_OBJECT

    public:
        AssetsPreviewer(QWidget * parent = nullptr);

        void previewFile(QString localFilePath);
        void resetPreview();

    private:
        QLabel* _previewedImage = nullptr;
};