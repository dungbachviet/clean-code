#ifndef PISCADA_API_SERVER_TEMP_CLEANER_H
#define PISCADA_API_SERVER_TEMP_CLEANER_H
#include <QString>

class TempCleaner
{
public:
    static void cleanOldTempFolders(const QString &path, int olderThanMinutes);
};
#endif // PISCADA_API_SERVER_TEMP_CLEANER_H