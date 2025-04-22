#ifndef PISCADA_API_SERVER_SUBSYSTEM_TYPES_H
#define PISCADA_API_SERVER_SUBSYSTEM_TYPES_H

#include "apiserverjson.h"


class Subsystem
{
public:
    QString id;
    QString controllerId;
    QString name;
    QString internalName;
    bool permanent;
    QString parentId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Subsystem, id, controllerId, name, internalName, permanent)

struct SubsystemCreateRequest
{
    QString name;
    QString parentId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SubsystemCreateRequest, name, parentId)

struct SubsystemUpdateRequest
{
    QString name;
    QString parentId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SubsystemUpdateRequest, name)

#endif // PISCADA_API_SERVER_SUBSYSTEM_TYPES_H
