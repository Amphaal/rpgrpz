#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <QDateTime>

#include <algorithm>

#include "_appContext.h"

class LogWriter {
    public:
        static void customMO(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    private:

        static inline bool _latest_been_inst = false;

        static void _fprtint(const QString &channel, const QMessageLogContext &context, const QString &msg);
        static void _fprintf_to_file(FILE* _fs, const QString &channel, const QMessageLogContext &context, const QString &msg);
};