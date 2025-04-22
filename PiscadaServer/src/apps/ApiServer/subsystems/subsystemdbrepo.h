#ifndef PISCADA_API_SERVER_SUBSYSTEM_DB_REPO_H
#define PISCADA_API_SERVER_SUBSYSTEM_DB_REPO_H

#include "apierror.h"


class Subsystem;
class SubsystemDbRepo
{
public:
    static ApiError read(const QString &id, Subsystem &subsystem);
    static ApiError list(QVector<Subsystem> &subsystems);
    static ApiError create(Subsystem &subsystem);
    static ApiError update(Subsystem &subsystem);
    static ApiError remove(const QString &id);
};

#endif // PISCADA_API_SERVER_SUBSYSTEM_DB_REPO_H
