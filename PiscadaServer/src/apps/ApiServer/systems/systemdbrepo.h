#ifndef PISCADA_API_SYSTEM_DB_REPO_H
#define PISCADA_API_SYSTEM_DB_REPO_H

#include "apierror.h"

#include <QVector>


class System;
class SystemDbRepo
{
public:
    static ApiError read(const QString &id, System &system);
    static ApiError list(QVector<System> &systems);
    static ApiError create(System &system);
    static ApiError update(System &system);
    static ApiError remove(const QString &id);
};

#endif // PISCADA_API_SYSTEM_DB_REPO_H
