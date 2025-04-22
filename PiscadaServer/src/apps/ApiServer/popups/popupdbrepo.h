#ifndef PISCADA_API_SERVER_POPUP_DB_REPO_H
#define PISCADA_API_SERVER_POPUP_DB_REPO_H

#include "apierror.h"


class Popup;
class PopupDetail;
class PopupDbRepo
{
public:
    static ApiError read(const QString &id, PopupDetail &popup);
    static ApiError list(QVector<Popup> &popups);
    static ApiError create(PopupDetail &popup);
    static ApiError update(PopupDetail &popup);
    static ApiError remove(const QString &id);
};

#endif // PISCADA_API_SERVER_POPUP_DB_REPO_H
