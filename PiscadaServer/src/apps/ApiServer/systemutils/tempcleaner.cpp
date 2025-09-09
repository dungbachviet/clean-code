#include "systemutils/tempcleaner.h"
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

void TempCleaner::cleanOldTempFolders(const QString &path, int olderThanMinutes)
{
    QDir baseDir(path);
    if (!baseDir.exists()) 
    {
        qWarning() << "[TempCleaner] Path does not exist:" << path;
        return;
    }

    const QDateTime threshold = QDateTime::currentDateTimeUtc().addSecs(-olderThanMinutes * 60);

    const QFileInfoList folders = baseDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &folderInfo : folders) 
    {
        QDateTime lastModified = folderInfo.lastModified();

        if (lastModified < threshold) 
        {
            QDir dir(folderInfo.absoluteFilePath());
            if (dir.removeRecursively()) 
            {
                qInfo() << "[TempCleaner] Deleted old temp folder:" << folderInfo.fileName();
            } 
            else 
            {
                qWarning() << "[TempCleaner] Failed to delete:" << folderInfo.fileName();
            }
        }
    }
}