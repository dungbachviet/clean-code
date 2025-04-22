#ifndef PISCADA_API_SERVER_VIEW_DB_REPO_H
#define PISCADA_API_SERVER_VIEW_DB_REPO_H

#include "apierror.h"
#include "views/viewtypes.h"


class ViewDbRepo
{
public:
    static ApiError read(const QString &id, ViewDetail &view);
    static ApiError list(QVector<View> &views);
    static ApiError create(ViewDetail &view);
    static ApiError update(ViewDetail &view);
    static ApiError remove(const QString &id);
};

#endif // PISCADA_API_SERVER_VIEW_DB_REPO_H
