#include "popups/popupdbrepo.h"
#include "popups/popuptypes.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


ApiError PopupDbRepo::read(const QString &id, PopupDetail &popup)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            uuid,
            name,
            description,
            content
        FROM
            `piscada_system`.`processviewpopups`
        WHERE
            uuid = :id;
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    q.bindValue(":id", id);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    if (!q.next())
    {
        return ApiError::notFound("Popup", id);
    }

    JsonError jsonError;

    popup.id = q.value(0).toString();
    popup.controllerId = info.controllerId();
    popup.name = q.value(1).toString();
    popup.description = q.value(2).toString();
    popup.content = JsonUtil::parse(q.value(3).toByteArray(), &jsonError);

    if (jsonError.type() != JsonError::NoError)
    {
        return ApiError::fromJsonError(jsonError);
    }

    return ApiError();
}

ApiError PopupDbRepo::list(QVector<Popup> &popups)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            uuid,
            name,
            description
        FROM
            `piscada_system`.`processviewpopups`;
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    while (q.next())
    {
        Popup p;
        p.id = q.value(0).toString();
        p.controllerId = info.controllerId();
        p.name = q.value(1).toString();
        p.description = q.value(2).toString();

        popups.append(p);
    }

    return ApiError();
}

ApiError PopupDbRepo::create(PopupDetail &popup)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        INSERT INTO `piscada_system`.`processviewpopups`
        (
            uuid,
            name,
            description,
            content
        )
        VALUES
        (
            :id,
            :name,
            :description,
            :content
        );
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    popup.id = createUuid();
    popup.controllerId = info.controllerId();

    QByteArray content = JsonUtil::encode(popup.content);

    q.bindValue(":id", popup.id);
    q.bindValue(":name", popup.name);
    q.bindValue(":description", popup.description);
    q.bindValue(":content", content);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

ApiError PopupDbRepo::update(PopupDetail &popup)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        UPDATE
            `piscada_system`.`processviewpopups`
        SET
            name = :name,
            description = :description,
            content = :content
        WHERE
            uuid = :id;
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    popup.controllerId = info.controllerId();

    q.bindValue(":id", popup.id);
    q.bindValue(":name", popup.name);
    q.bindValue(":description", popup.description);
    q.bindValue(":content", JsonUtil::encode(popup.content));

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

ApiError PopupDbRepo::remove(const QString &id)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        DELETE FROM
            `piscada_system`.`processviewpopups`
        WHERE
            uuid = :id
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    q.bindValue(":id", id);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

