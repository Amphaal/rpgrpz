#include <QtCore/QString>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

#include <QDir>
#include <QLockFile>

int main(int argc, char** argv){
    
    //prevent multiples instances
    QString tmpDir = QDir::tempPath();
    QLockFile lockFile(tmpDir + "/rpgrpz.lock");
    if(!lockFile.tryLock(100)){
        return 1;
    }

    //setup app
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName(QString(APP_NAME));
    app.setStyle(QStyleFactory::create("Fusion")); 
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    //wait for the app to close
    return app.exec();
}