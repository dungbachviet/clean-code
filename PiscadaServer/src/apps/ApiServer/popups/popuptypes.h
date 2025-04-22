#ifndef PISCADA_API_SERVER_POPUP_TYPES_H
#define PISCADA_API_SERVER_POPUP_TYPES_H

#include "apiserverjson.h"

#include "piscada/optional.h"


class Popup
{
public:
    QString id;
    QString controllerId;
    QString name;
    QString description;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Popup, id, controllerId, name, description)

class PopupDetail : public Popup
{
public:
    Json content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PopupDetail, id, controllerId, name, description, content)

struct PopupCreateRequest
{
    QString name;
    QString description;
    piscada::optional<Json> content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PopupCreateRequest, name, description)

struct PopupCloneRequest
{
    QString name;
    QString description;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PopupCloneRequest, name, description)

struct PopupUpdateRequest
{
    QString name;
    QString description;
    Json content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PopupUpdateRequest, name, description, content)

#endif // PISCADA_API_SERVER_POPUP_TYPES_H
