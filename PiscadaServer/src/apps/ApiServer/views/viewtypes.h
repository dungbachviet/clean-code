#ifndef PISCADA_API_SERVER_VIEW_TYPES_H
#define PISCADA_API_SERVER_VIEW_TYPES_H

#include "apiserverjson.h"

#include <QString>


class View
{
public:
    QString id;
    QString controllerId;
    QString path;
    QString name;
    QString description;
    QString facility;
    QString groups;
    QString type;
    QString themeId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(View, id, controllerId, path, name, description, facility, groups, type, themeId)

class ViewDetail : public View
{
public:
    Json content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ViewDetail, id, controllerId, path, name, description, content, facility, groups, type, themeId)

struct ViewCreateRequest
{
    QString path;
    QString name;
    QString description;
    Json content;
    QString type;
    QString themeId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ViewCreateRequest, path, name, description, content, type, themeId)

struct ViewCloneRequest
{
    QString name;
    QString description;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ViewCloneRequest, name, description)

struct ViewUpdateRequest
{
    QString path;
    QString name;
    QString description;
    Json content;
    QString type;
    QString themeId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ViewUpdateRequest, path, name, description, content, type, themeId)

#endif // PISCADA_API_SERVER_VIEW_TYPES_H
