#ifndef PISCADA_API_SERVER_TEMPLATE_TYPES_H
#define PISCADA_API_SERVER_TEMPLATE_TYPES_H

#include "apiserverjson.h"

#include "piscada/optional.h"

#include <QString>


class Template
{
public:
    QString id;
    QString controllerId;
    QString name;
    QString description;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Template, id, controllerId, name, description)

class TemplateDetail : public Template
{
public:
    Json content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TemplateDetail, id, controllerId, name, description, content)

struct TemplateCreateRequest
{
    QString name;
    QString description;
    piscada::optional<Json> content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TemplateCreateRequest, name, description)

struct TemplateCloneRequest
{
    QString name;
    QString description;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TemplateCloneRequest, name, description)

struct TemplateUpdateRequest
{
    QString name;
    QString description;
    Json content;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TemplateUpdateRequest, name, description, content)

#endif // PISCADA_API_SERVER_TEMPLATE_TYPES_H
