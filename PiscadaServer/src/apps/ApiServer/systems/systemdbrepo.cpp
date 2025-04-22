#include "systems/systemdbrepo.h"
#include "systems/systemtypes.h"
#include "apiserverdatabase.h"
#include "apiserverinfo.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


ApiError SystemDbRepo::read(const QString &id, System &system)
{
    QSqlDatabase db = getConnection();    

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            `name`,
            `shortname`,
            `premanent`,
        FROM
            `piscada_system`.`folders`
        WHERE
            `uuid` = '%1';
    )SQL";

    if (!q.exec(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    if (!q.next())
    {
        return ApiError::notFound("System", id);
    }

    system.id = id;
    system.controllerId = info.controllerId();
    system.name = q.value(0).toString();
    system.internalName = q.value(1).toString();
    system.permanent = q.value(2).toBool();

    return ApiError();
}

ApiError SystemDbRepo::list(QVector<System> &systems)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            `uuid`,
            `name`,
            `shortname`,
            `premanent`,
        FROM
            `piscada_system`.`folders`;
    )SQL";

    if (!q.exec(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    while (q.next())
    {
        System system;
        system.id = q.value(0).toString();
        system.controllerId = info.controllerId();
        system.name = q.value(1).toString();
        system.internalName = q.value(2).toString();
        system.permanent = q.value(3).toBool();

        systems.append(system);
    }

    return ApiError();
}

ApiError SystemDbRepo::create(System &system)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        INSERT INTO
            `piscada_system`.`folders`
            (`uuid`, `name`, `shortname`, `premanent`)
        VALUES
            (:uuid, :name, :shortname, :permanent);
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    system.id = createUuid();

    q.bindValue(":uuid", system.id);
    q.bindValue(":name", system.name);
    q.bindValue(":shortname", system.internalName);
    q.bindValue(":permanent", system.permanent);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    system.controllerId = info.controllerId();

    return ApiError();
}

ApiError SystemDbRepo::update(System &system)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        UPDATE
            `piscada_system`.`folders`
        SET
            `name` = :name,
            `shortname` = :shortname,
            `premanent` = :permanent
        WHERE
            `uuid` = :uuid;
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    q.bindValue(":name", system.name);
    q.bindValue(":shortname", system.internalName);
    q.bindValue(":permanent", system.permanent);
    q.bindValue(":uuid", system.id);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    system.controllerId = info.controllerId();

    return ApiError();
}

ApiError SystemDbRepo::remove(const QString &id)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        DELETE FROM
            `piscada_system`.`folders`
        WHERE
            `uuid` = :uuid;
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    q.bindValue(":uuid", id);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

