#include "subsystems/subsystemdbrepo.h"
#include "subsystems/subsystemtypes.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>


ApiError SubsystemDbRepo::read(const QString &id, Subsystem &subsystem)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            `uuid`,
            `name`,
            `internalName`,
            `permanent`,
            `parentId`
        FROM
            `piscada_system`.`subsystems`
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
        return ApiError::notFound("subsystem", id);
    }

    subsystem.id = q.value("uuid").toString();
    subsystem.controllerId = info.controllerId();
    subsystem.name = q.value("name").toString();
    subsystem.internalName = q.value("internalName").toString();
    subsystem.permanent = q.value("permanent").toBool();
    subsystem.parentId = q.value("parentId").toString();

    return ApiError();
}

ApiError SubsystemDbRepo::list(QVector<Subsystem> &subsystems)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        SELECT
            `uuid`,
            `name`,
            `description`,
            `permanent`,
            `parentId`
        FROM
            `piscada_system`.`subsystems`;
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
        Subsystem subsystem;
        subsystem.id = q.value(0).toString();
        subsystem.controllerId = info.controllerId();
        subsystem.internalName = q.value(1).toString();
        subsystem.name = q.value(2).toString();
        subsystem.permanent = q.value(3).toBool();
        subsystem.parentId = q.value(4).toString();

        subsystems.append(subsystem);
    }
    
    return ApiError();
}

ApiError SubsystemDbRepo::create(Subsystem &subsystem)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        INSERT INTO
            `piscada_system`.`subsystems`
            (
                `uuid`,
                `name`,
                `description`,
                `permanent`,
                `parentId`
            )
        VALUES
            (
                :uuid,
                :internalName,
                :name,
                :permanent,
                :parentId
            );
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    subsystem.controllerId = info.controllerId();

    q.bindValue(":uuid", subsystem.id);
    q.bindValue(":name", subsystem.name);
    q.bindValue(":internalName", subsystem.internalName);
    q.bindValue(":permanent", subsystem.permanent);
    q.bindValue(":parentId", subsystem.parentId);

    if (!q.exec())
    {
        return ApiError::fromSqlError(q.lastError());
    }
    
    return ApiError();
}

ApiError SubsystemDbRepo::update(Subsystem &subsystem)
{
    QSqlDatabase db = getConnection();

    QSqlQuery q(db);

    QString query = R"SQL(
        UPDATE
            `piscada_system`.`subsystems`
        SET
            `name` = :internalName,
            `description` = :name,
            `permanent` = :permanent,
            `parentId` = :parentId
        WHERE
            `uuid` = :uuid;"
    )SQL";

    if (!q.prepare(query))
    {
        return ApiError::fromSqlError(q.lastError());
    }

    return ApiError();
}

ApiError SubsystemDbRepo::remove(const QString &id)
{
    Q_UNUSED(id)

    return ApiError();
}

