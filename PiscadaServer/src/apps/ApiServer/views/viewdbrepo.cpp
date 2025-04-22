#include "views/viewdbrepo.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"

#include <QSqlQuery>
#include <QSqlError>


ApiError ViewDbRepo::read(const QString &id, ViewDetail &view)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            `uuid`,
            `controllerId`,
            `path`,
            `name`,
            `description`,
            `content`,
            `facility`,
            `groups`,
            `type`,
            `themeId`
        FROM
            `piscada_system`.`processviews`
        WHERE
            `uuid` = :id;
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
        return ApiError::notFound("view", id);
    }

    JsonError jsonError;

    view.id = q.value("uuid").toString();
    view.controllerId = q.value("controllerId").toString();
    view.path = q.value("path").toString();
    view.name = q.value("name").toString();
    view.description = q.value("description").toString();
    view.content = JsonUtil::parse(q.value("content").toString().toUtf8(), &jsonError);
    view.facility = q.value("facility").isNull() ? "" : q.value("facility").toString();
    view.groups = q.value("groups").isNull() ? "" : q.value("groups").toString();
    view.type = q.value("type").toString();
    view.themeId = q.value("themeId").toString();

    if (jsonError.type() != JsonError::NoError)
    {
        return ApiError::fromJsonError(jsonError);
    }

    return ApiError();
}

ApiError ViewDbRepo::list(QVector<View> &views)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            `uuid`,
            `controllerId`,
            `path`,
            `name`,
            `description`,
            `facility`,
            `groups`,
            `type`,
            `themeId`
        FROM
            `piscada_system`.`processviews`;
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
        View view;
        view.id = q.value("uuid").toString();
        view.controllerId = q.value("controllerId").toString();
        view.path = q.value("path").toString();
        view.name = q.value("name").toString();
        view.description = q.value("description").toString();
        view.facility = q.value("facility").toString();
        view.groups = q.value("groups").toString();
        view.type = q.value("type").toString();
        view.themeId = q.value("themeId").toString();

        views.append(view);
    }

    return ApiError();
}

ApiError ViewDbRepo::create(ViewDetail &view)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        INSERT INTO `piscada_system`.`processviews`
        (
            `uuid`,
            `controllerId`,
            `path`,
            `name`,
            `description`,
            `content`,
            `facility`,
            `groups`,
            `type`,
            `themeId`
        )
        VALUES
        (
            :uuid,
            :controllerId,
            :path,
            :name,
            :description,
            :content,
            :facility,
            :groups,
            :type,
            :themeId
        );
    )SQL";

    view.id = createUuid();
    view.controllerId = info.controllerId();

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    QByteArray content = JsonUtil::encode(view.content);

    q.bindValue(":uuid", view.id);
    q.bindValue(":controllerId", view.controllerId);
    q.bindValue(":path", view.path);
    q.bindValue(":name", view.name);
    q.bindValue(":description", view.description);
    q.bindValue(":content", content);
    q.bindValue(":facility", view.facility);
    q.bindValue(":groups", view.groups);
    q.bindValue(":type", view.type);
    q.bindValue(":themeId", view.themeId);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

ApiError ViewDbRepo::update(ViewDetail &view)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        UPDATE
            `piscada_system`.`processviews`
        SET
            `controllerId` = :controllerId,
            `path` = :path,
            `name` = :name,
            `description` = :description,
            `content` = :content,
            `facility` = :facility,
            `groups` = :groups,
            `type` = :type,
            `themeId` = :themeId
        WHERE
            `uuid` = :uuid;
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }
    
    QByteArray content = JsonUtil::encode(view.content);

    q.bindValue(":controllerId", view.controllerId);
    q.bindValue(":path", view.path);
    q.bindValue(":name", view.name);
    q.bindValue(":description", view.description);
    q.bindValue(":content", content);
    q.bindValue(":facility", view.facility);
    q.bindValue(":groups", view.groups);
    q.bindValue(":type", view.type);
    q.bindValue(":themeId", view.themeId);
    q.bindValue(":uuid", view.id);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

ApiError ViewDbRepo::remove(const QString &id)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        DELETE FROM
            `piscada_system`.`processviews`
        WHERE
            `uuid` = :id;
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

