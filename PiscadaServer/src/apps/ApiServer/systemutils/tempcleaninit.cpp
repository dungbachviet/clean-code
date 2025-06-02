#include "systemutils/tempcleaninit.h"
#include "systemutils/tempcleaner.h"
#include <QTimer>
#include <QtConcurrent>

void TempCleanInit::startScheduledCleanup(const QString &path, int maxAgeMinutes, int intervalHours)
{
    //Run clean immediately on startup
    TempCleaner::cleanOldTempFolders(path, maxAgeMinutes);  

    // Schedule the cleanup to run periodically
    QTimer *cleanupTimer = new QTimer();
    QObject::connect(cleanupTimer, &QTimer::timeout, [=]() {
        QtConcurrent::run([=]() {
            TempCleaner::cleanOldTempFolders(path, maxAgeMinutes);
        });
    });

    cleanupTimer->start(intervalHours * 60 * 60 * 1000);  
}
