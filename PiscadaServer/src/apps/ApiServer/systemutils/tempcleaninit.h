#ifndef PISCADA_API_SERVER_TEMP_CLEAN_INIT_H
#define PISCADA_API_SERVER_TEMP_CLEAN_INIT_H

#include <QString>

class TempCleanInit
{
public:
    static void startScheduledCleanup(const QString &path, int maxAgeMinutes, int intervalHours);
};

#endif // PISCADA_API_SERVER_TEMP_CLEAN_INIT_H