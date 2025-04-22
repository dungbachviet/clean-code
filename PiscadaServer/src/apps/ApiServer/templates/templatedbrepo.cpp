#include "templates/templatedbrepo.h"
#include "templates/templatetypes.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"

#include <QSqlQuery>
#include <QSqlError>


ApiError TemplateDbRepo::read(const QString &id, TemplateDetail &t)
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
            `piscada_system`.`processviewtemplates`
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
        return ApiError::notFound("Template", id);
    }

    JsonError jsonError;

    t.id = q.value("uuid").toString();
    t.controllerId = info.controllerId();
    t.name = q.value("name").toString();
    t.description = q.value("description").toString();
    t.content = JsonUtil::parse(q.value("content").toString().toUtf8(), &jsonError);

    if (jsonError.type() != JsonError::NoError)
    {
        return ApiError::fromJsonError(jsonError);
    }

    return ApiError();
}

ApiError TemplateDbRepo::list(QVector<Template> &templates)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            `uuid`,
            name,
            description
        FROM
            `piscada_system`.`processviewtemplates`;
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
        Template t;
        t.id = q.value("id").toString();
        t.name = q.value("name").toString();
        t.description = q.value("description").toString();

        templates.append(t);
    }

    return ApiError();
}

ApiError TemplateDbRepo::create(TemplateDetail &t)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        INSERT INTO
            `piscada_system`.`processviewtemplates`
            (uuid, name, description, content)
        VALUES
            (:id, :name, :description, :content);
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    t.id = createUuid();
    t.controllerId = info.controllerId();

    QByteArray content = JsonUtil::encode(t.content);

    q.bindValue(":id", t.id);
    q.bindValue(":name", t.name);
    q.bindValue(":description", t.description);
    q.bindValue(":content", content);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

ApiError TemplateDbRepo::update(TemplateDetail &t)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        UPDATE
            `piscada_system`.`processviewtemplates`
        SET
            name = :name,
            description = :description,
            content = :content
        WHERE
            `uuid` = :id;
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    QByteArray content = JsonUtil::encode(t.content);

    q.bindValue(":id", t.id);
    q.bindValue(":name", t.name);
    q.bindValue(":description", t.description);
    q.bindValue(":content", content);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

ApiError TemplateDbRepo::remove(const QString &id)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        DELETE FROM
            `piscada_system`.`processviewtemplates`
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

