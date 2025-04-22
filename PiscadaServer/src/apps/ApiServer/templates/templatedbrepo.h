#ifndef PISCADA_API_SERVER_TEMPLATE_DB_REPO_H
#define PISCADA_API_SERVER_TEMPLATE_DB_REPO_H

#include "apierror.h"


class Template;
class TemplateDetail;
class TemplateDbRepo
{
public:
    static ApiError read(const QString &id, TemplateDetail &t);
    static ApiError list(QVector<Template> &templates);
    static ApiError create(TemplateDetail &t);
    static ApiError update(TemplateDetail &t);
    static ApiError remove(const QString &id);
};

#endif // PISCADA_API_SERVER_TEMPLATE_DB_REPO_H
