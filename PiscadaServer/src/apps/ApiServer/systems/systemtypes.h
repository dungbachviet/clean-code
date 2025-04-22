#ifndef PISCADA_API_SERVER_SYSTEM_TYPES_H
#define PISCADA_API_SERVER_SYSTEM_TYPES_H

#include "apiserverjson.h"


class System
{
public:
    QString id;
    QString controllerId;
    QString name;
    QString internalName;
    bool permanent;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(System, id, controllerId, name, internalName, permanent)

struct SystemCreateRequest
{
    QString name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SystemCreateRequest, name)

struct SystemUpdateRequest
{
    QString name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SystemUpdateRequest, name)

#endif // PISCADA_API_SERVER_SYSTEM_TYPES_H
