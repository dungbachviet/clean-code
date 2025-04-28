#include "documents/documentdbrepo.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDir>

bool DocumentDbRepo::doesNameExist(const QString &name, const QString &folderId, const QString &type)
{
    QSqlDatabase db = getConnection(); 
    QSqlQuery query(db);
    
    QString sql = R"SQL(
        SELECT 1 
        FROM `piscada_system`.`documents`
        WHERE name = :name 
        AND type = :type
        AND isDeleted = 0
        AND (parentId = :folderId OR (parentId IS NULL AND :folderId IS NULL))
        LIMIT 1
    )SQL";

    if (!query.prepare(sql))
    {
        qDebug() << "Error prepare to query in check duplicate folder" << query.lastError().text();
        return true;
    }

    query.bindValue(":name", name);
    query.bindValue(":type", type);
    query.bindValue(":folderId", folderId.isEmpty() ? QVariant(QVariant::String) : folderId);

    if (!query.exec())
    {
        qDebug() << "Error query in check duplicate folder" << query.lastError().text();
        return true;
    }

    return query.next(); 
}

bool DocumentDbRepo::isValidParent(const QString &parentId)
{
    if (parentId.isEmpty()) {
        return true;  
    }
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        SELECT 1
        FROM `piscada_system`.`documents`
        WHERE id = :id AND type = 'folder'
        LIMIT 1
    )SQL";

    if (!query.prepare(sql)) {
        qDebug() << "Error preparing query in isValidParentFolder:" << query.lastError().text();
        return false;
    }

    query.bindValue(":id", parentId);

    if (!query.exec()) {
        qDebug() << "Error executing query in isValidParentFolder:" << query.lastError().text();
        return false;
    }
    return query.next(); 
}

ApiError DocumentDbRepo::create(DocumentDetail &document)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        INSERT INTO `piscada_system`.`documents` 
        (
            `id`, 
            `parentId`, 
            `name`, 
            `description`, 
            `type`, 
            `extension`, 
            `writeAccess`, 
            `readAccess`
        )
        VALUES 
        (
            :id, 
            :parentId, 
            :name, 
            :description, 
            :type, 
            :extension, 
            :writeAccess, 
            :readAccess
        );
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":id", document.id);
    query.bindValue(":parentId", document.parentId.isEmpty() ? QVariant() : document.parentId);    
    query.bindValue(":name", document.name);
    query.bindValue(":description", document.description);
    query.bindValue(":type", document.type);  
    query.bindValue(":extension", document.extension.isEmpty() ? QVariant() : document.extension);
    query.bindValue(":writeAccess", document.writeAccess);
    query.bindValue(":readAccess", document.readAccess);

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    return ApiError();
}

ApiError DocumentDbRepo::read(const QString &id, DocumentDetail &document)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        SELECT
            `id`,
            `parentId`,
            `name`,
            `description`,
            `type`,
            `extension`,
            `writeAccess`,
            `readAccess`,
            `isDeleted`,
            `createdAt`,
            `updatedAt`
        FROM
            `piscada_system`.`documents` 
        WHERE
            `id` = :id;
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":id", id);

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    if (!query.next())
    {
        return ApiError::notFound("document", id);
    }

    document.id = query.value("id").toString();
    document.parentId = query.value("parentId").isNull() ? "" : query.value("parentId").toString();
    document.name = query.value("name").toString();
    document.description = query.value("description").isNull() ? "" : query.value("description").toString();
    document.type = query.value("type").toString();
    document.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
    document.writeAccess = query.value("writeAccess").isNull() ? -1 : query.value("writeAccess").toInt();
    document.readAccess = query.value("readAccess").isNull() ? -1 : query.value("readAccess").toInt();

    JsonError jsonError;
    if (jsonError.type() != JsonError::NoError)
    {
        return ApiError::fromJsonError(jsonError);
    }

    return ApiError();
}

ApiError DocumentDbRepo::list(QVector<DocumentDetail> &documents, bool getArchivedFile)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        SELECT
            `id`,
            `parentId`,
            `name`,
            `description`,
            `type`,
            `extension`,
            `writeAccess`,
            `readAccess`,
            `isDeleted`,
            `createdAt`,
            `updatedAt`
        FROM
            `piscada_system`.`documents`
        WHERE 
            `isDeleted` = :isDeleted
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":isDeleted", getArchivedFile);

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    while (query.next())
    {
        DocumentDetail doc;
        doc.id = query.value("id").toString();
        doc.parentId = query.value("parentId").isNull() ? "" : query.value("parentId").toString();
        doc.name = query.value("name").toString();
        doc.description = query.value("description").isNull() ? "" : query.value("description").toString();
        doc.type = query.value("type").toString();
        doc.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
        doc.writeAccess = query.value("writeAccess").isNull() ? -1 : query.value("writeAccess").toInt();
        doc.readAccess = query.value("readAccess").isNull() ? -1 : query.value("readAccess").toInt();

        documents.append(doc);
    }

    return ApiError();
}

ApiError DocumentDbRepo::update(const QString &id, DocumentDetail &document)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        UPDATE `piscada_system`.`documents` 
        SET
            `name` = :name,
            `description` = :description,
            `writeAccess` = :writeAccess,
            `readAccess` = :readAccess,
            `parentId` = :parentId
        WHERE
            `id` = :id;
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":name", document.name);
    query.bindValue(":description", document.description);
    query.bindValue(":writeAccess", document.writeAccess);
    query.bindValue(":readAccess", document.readAccess);
    query.bindValue(":parentId", document.parentId.isEmpty() ? QVariant() : document.parentId);
    query.bindValue(":id", id);

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    return ApiError();
}

ApiError DocumentDbRepo::remove(const QString &id)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        DELETE FROM
            `piscada_system`.`documents`
        WHERE
            `id` = :id and `isDeleted` = 0;
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":id", id);

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    return ApiError();
}

ApiError DocumentDbRepo::archiveFile(const QString &id)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        UPDATE `piscada_system`.`documents`
        SET `isDeleted` = 1, `parentId` = NULL
        WHERE `id` = :id
    )SQL";

    if (!query.prepare(sql)) {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":id", id);

    if (!query.exec()) {
        return ApiError::fromSqlError(query.lastError());
    }

    return ApiError();
}

ApiError DocumentDbRepo::unarchiveFile(const QString &id, DocumentDetail &document)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        UPDATE `piscada_system`.`documents`
        SET `isDeleted` = 0, 
            `parentId` = :parentId
        WHERE id = :id AND isDeleted = 1
    )SQL";

    if (!query.prepare(sql)) {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":id", id);
    query.bindValue(":parentId", document.parentId.isEmpty() ? QVariant() : document.parentId);

    if (!query.exec()) {
        return ApiError::fromSqlError(query.lastError());
    }

    if (query.numRowsAffected() == 0) {
        return ApiError::notFound("Deleted document", id);
    }

    return ApiError();
}

ApiError DocumentDbRepo::getChildrenOfFolder(const QString &folderId, QVector<DocumentDetail> &documents)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);
    QString sql = R"SQL(
        SELECT
            `id`,
            `parentId`,
            `name`,
            `description`,
            `type`,
            `extension`,
            `writeAccess`,
            `readAccess`,
            `isDeleted`,
            `createdAt`,
            `updatedAt`
        FROM
            `piscada_system`.`documents`
        WHERE
            (parentId = :folderId OR (parentId IS NULL AND :folderId IS NULL))
            AND isDeleted = 0;
    )SQL";
    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }
    query.bindValue(":folderId", folderId.isEmpty() ? QVariant() : folderId);
    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }
    while (query.next())
    {
        DocumentDetail doc;
        doc.id = query.value("id").toString();
        doc.parentId = query.value("parentId").isNull() ? "" : query.value("parentId").toString();
        doc.name = query.value("name").toString();
        doc.description = query.value("description").isNull() ? "" : query.value("description").toString();
        doc.type = query.value("type").toString();
        doc.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
        doc.writeAccess = query.value("writeAccess").isNull() ? -1 : query.value("writeAccess").toInt();
        doc.readAccess = query.value("readAccess").isNull() ? -1 : query.value("readAccess").toInt();
        documents.append(doc);
    }
    return ApiError();
}

