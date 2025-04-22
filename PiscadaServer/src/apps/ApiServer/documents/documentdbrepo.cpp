#include "documents/documentdbrepo.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDir>

bool DocumentDbRepo::isNameExists(const QString &name, const QString &parentId, const QString &type)
{
    QSqlDatabase db = getConnection(); 
    QSqlQuery query(db);
    
    QString sql = R"SQL(
        SELECT 1 
        FROM `piscada_system`.`documents`
        WHERE name = :name 
        AND type = :type
        AND is_deleted = 0
        AND (parent_id = :parent_id OR (parent_id IS NULL AND :parent_id IS NULL))
        LIMIT 1
    )SQL";

    if (!query.prepare(sql))
    {
        qDebug() << "Error prepare to query in check duplicate folder" << query.lastError().text();
        return true;
    }

    query.bindValue(":name", name);
    query.bindValue(":type", type);
    query.bindValue(":parent_id", parentId.isEmpty() ? QVariant(QVariant::String) : parentId);

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

int DocumentDbRepo::getLevel(const QString &parentId)
{
    if (parentId.isEmpty()) {
        return 1;  
    }
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        SELECT level 
        FROM `piscada_system`.`documents` 
        WHERE id = :parentId
    )SQL"; 
    if (!query.prepare(sql))
    {
        qDebug() << "Error prepare to query in getlevel" << query.lastError().text();
        return 1;
    }

    query.bindValue(":parentId", parentId);
    if (!query.exec()) {
        qDebug() << "Error query in getlevel" << query.lastError().text();
        return 1;
    }

    if (query.next() && !query.value(0).isNull()) {
        return query.value(0).toInt() + 1;  
    }
    return 1;
}
ApiError DocumentDbRepo::decodeAndValidateBase64File(const QByteArray &fileContent, const QString &expectedExtension, QByteArray &decodedContent)
{
    const QByteArray headerPrefix = "data:";
    int commaIndex = fileContent.indexOf(',');

    if (fileContent.startsWith(headerPrefix) && commaIndex != -1)
    {
        QByteArray header = fileContent.left(commaIndex);
        QByteArray base64Data = fileContent.mid(commaIndex + 1);
        decodedContent = QByteArray::fromBase64(base64Data);

        int semiIndex = header.indexOf(';');
        QString mimeType;
        if (semiIndex != -1)
        {
            mimeType = QString::fromUtf8(header.mid(headerPrefix.length(), semiIndex - headerPrefix.length()));
        }

        QMap<QString, QString> mimeToExt = {
            {"application/pdf", "pdf"},
            {"text/plain", "txt"},
            {"image/png", "png"},
            {"image/jpeg", "jpg"},
            {"application/zip", "zip"},
            {"application/vnd.openxmlformats-officedocument.wordprocessingml.document", "docx"},
            {"application/msword", "doc"},
            {"application/vnd.ms-excel", "xls"},
            {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "xlsx"}
        };

        if (mimeToExt.contains(mimeType))
        {
            QString expectedExt = mimeToExt[mimeType];
            if (expectedExt != expectedExtension.toLower())
            {
                    return ApiError::InvalidRequest;
            }
        }
        else
        {
            return ApiError::InvalidRequest;
        }
    }
    else
    {
        decodedContent = QByteArray::fromBase64(fileContent);
    }
    return ApiError(); 
}

ApiError DocumentDbRepo::saveFileToSystem(const QString &id, const QString &extension, const QByteArray &fileContent)
{
    QString dirPath = "/Piscada/documents";
    QDir dir(dirPath);

    if (!dir.exists())
    {
        if (!dir.mkpath(dirPath))
        {
            return ApiError::internalError("Failed to create directory on the system");
        }
    }
    QString filePath = QString("%1/%2.%3").arg(dirPath, id, extension);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return ApiError::internalError("Failed to create file on the system");
    }
    file.write(fileContent);
    file.close();

    return ApiError();  
}

ApiError DocumentDbRepo::readFileFromSystem(const QString &id, const QString &extension, QString &fileContentBase64)
{
    QString filePath = QString("/Piscada/documents/%1.%2").arg(id, extension);
    QFile file(filePath);

    if (!file.exists())
    {
        return ApiError::notFound("File", id);
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        return ApiError::internalError("Failed to open file on the system");
    }

    QByteArray fileData = file.readAll();
    fileContentBase64 = fileData.toBase64();
    file.close();

    return ApiError();
}

QVector<DocumentDetail> DocumentDbRepo::getAllDescendants(const QString &parentId)
{
    QVector<DocumentDetail> result;

    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        WITH RECURSIVE descendants AS (
            SELECT * FROM piscada_system.documents WHERE parent_id = :parent_id
            UNION ALL
            SELECT d.* FROM piscada_system.documents d
            INNER JOIN descendants ds ON d.parent_id = ds.id
        )
        SELECT * FROM descendants
    )SQL";

    if (!query.prepare(sql)) {
        qDebug() << "Error preparing recursive descendant query:" << query.lastError().text();
        return result;
    }

    query.bindValue(":parent_id", parentId);

    if (!query.exec()) {
        qDebug() << "Error executing recursive descendant query:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        DocumentDetail doc;
        doc.id = query.value("id").toString();
        doc.parentId = query.value("parent_id").isNull() ? "" : query.value("parent_id").toString();
        doc.name = query.value("name").toString();
        doc.type = query.value("type").toString();
        doc.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
        doc.level = query.value("level").toInt();
        result.append(doc);
    }
    return result;
}

ApiError DocumentDbRepo::updateLevelForDescendants(const QString &documentId, int deltaLevel)
{
    if (deltaLevel == 0)
        return ApiError();

    QVector<DocumentDetail> descendants = getAllDescendants(documentId);
    if (descendants.isEmpty())
        return ApiError();

    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    for (const DocumentDetail &doc : descendants)
    {
        QString sql = R"SQL(
            UPDATE `piscada_system`.`documents`
            SET `level` = `level` + :delta
            WHERE `id` = :id
        )SQL";

        if (!query.prepare(sql))
        {
            return ApiError::fromSqlError(query.lastError());
        }
        query.bindValue(":delta", deltaLevel);
        query.bindValue(":id", doc.id);

        if (!query.exec())
        {
            return ApiError::fromSqlError(query.lastError());
        }
    }
    return ApiError();
}

ApiError DocumentDbRepo::create(DocumentDetail &document)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        INSERT INTO `piscada_system`.`documents` 
        (
            `id`, 
            `parent_id`, 
            `name`, 
            `description`, 
            `type`, 
            `extension`, 
            `level`, 
            `write_access`, 
            `read_access`
        )
        VALUES 
        (
            :id, 
            :parent_id, 
            :name, 
            :description, 
            :type, 
            :extension, 
            :level, 
            :write_access, 
            :read_access
        );
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":id", document.id);
    query.bindValue(":parent_id", document.parentId.isEmpty() ? QVariant() : document.parentId);    
    query.bindValue(":name", document.name);
    query.bindValue(":description", document.description);
    query.bindValue(":type", document.type);  
    query.bindValue(":extension", document.extension.isEmpty() ? QVariant() : document.extension);
    query.bindValue(":level", document.level);
    query.bindValue(":write_access", document.writeAccess);
    query.bindValue(":read_access", document.readAccess);

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
            `parent_id`,
            `name`,
            `description`,
            `type`,
            `extension`,
            `level`,
            `write_access`,
            `read_access`,
            `is_deleted`,
            `created_at`,
            `updated_at`
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
    document.parentId = query.value("parent_id").isNull() ? "" : query.value("parent_id").toString();
    document.name = query.value("name").toString();
    document.description = query.value("description").isNull() ? "" : query.value("description").toString();
    document.type = query.value("type").toString();
    document.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
    document.level = query.value("level").toInt();
    document.writeAccess = query.value("write_access").isNull() ? -1 : query.value("write_access").toInt();
    document.readAccess = query.value("read_access").isNull() ? -1 : query.value("read_access").toInt();

    JsonError jsonError;
    if (jsonError.type() != JsonError::NoError)
    {
        return ApiError::fromJsonError(jsonError);
    }

    return ApiError();
}

ApiError DocumentDbRepo::list(QVector<DocumentDetail> &documents)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        SELECT
            `id`,
            `parent_id`,
            `name`,
            `description`,
            `type`,
            `extension`,
            `level`,
            `write_access`,
            `read_access`,
            `is_deleted`,
            `created_at`,
            `updated_at`
        FROM
            `piscada_system`.`documents`
        WHERE
            `is_deleted` = 0;
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    while (query.next())
    {
        DocumentDetail doc;
        doc.id = query.value("id").toString();
        doc.parentId = query.value("parent_id").isNull() ? "" : query.value("parent_id").toString();
        doc.name = query.value("name").toString();
        doc.description = query.value("description").isNull() ? "" : query.value("description").toString();
        doc.type = query.value("type").toString();
        doc.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
        doc.level = query.value("level").toInt();
        doc.writeAccess = query.value("write_access").isNull() ? -1 : query.value("write_access").toInt();
        doc.readAccess = query.value("read_access").isNull() ? -1 : query.value("read_access").toInt();

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
            `write_access` = :write_access,
            `read_access` = :read_access
        WHERE
            `id` = :id;
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":name", document.name);
    query.bindValue(":description", document.description);
    query.bindValue(":write_access", document.writeAccess);
    query.bindValue(":read_access", document.readAccess);
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
            `id` = :id and `is_deleted` = 0;
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

ApiError DocumentDbRepo::listChildren(const QString &id, QVector<DocumentDetail> &documents)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        SELECT
            `id`,
            `parent_id`,
            `name`,
            `description`,
            `type`,
            `extension`,
            `level`,
            `write_access`,
            `read_access`,
            `is_deleted`,
            `created_at`,
            `updated_at`
        FROM
            `piscada_system`.`documents`
        WHERE
            (parent_id = :parent_id OR (parent_id IS NULL AND :parent_id IS NULL))
            AND is_deleted = 0;
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }
    query.bindValue(":parent_id", id.isEmpty() ? QVariant() : id);

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    while (query.next())
    {
        DocumentDetail doc;
        doc.id = query.value("id").toString();
        doc.parentId = query.value("parent_id").isNull() ? "" : query.value("parent_id").toString();
        doc.name = query.value("name").toString();
        doc.description = query.value("description").isNull() ? "" : query.value("description").toString();
        doc.type = query.value("type").toString();
        doc.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
        doc.level = query.value("level").toInt();
        doc.writeAccess = query.value("write_access").isNull() ? -1 : query.value("write_access").toInt();
        doc.readAccess = query.value("read_access").isNull() ? -1 : query.value("read_access").toInt();

        documents.append(doc);
    }

    return ApiError();
}

ApiError DocumentDbRepo::move(const QString &id, DocumentDetail &document)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);
    
    QString sql = R"SQL(
        UPDATE `piscada_system`.`documents`
        SET `parent_id` = :new_parent_id,
            `level` = :new_level
        WHERE `id` = :id
    )SQL";
    
    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }
    
    query.bindValue(":new_parent_id", document.parentId.isEmpty() ? QVariant() : document.parentId);  
    query.bindValue(":new_level", document.level);
    query.bindValue(":id", id);
    
    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }
    return ApiError();
}

ApiError DocumentDbRepo::softDelete(const QString &id)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        UPDATE `piscada_system`.`documents`
        SET `is_deleted` = 1, `parent_id` = NULL
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

ApiError DocumentDbRepo::restore(const QString &id, DocumentDetail &document)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        UPDATE `piscada_system`.`documents`
        SET `is_deleted` = 0, 
            `parent_id` = :new_parent_id,
            `level` = :new_level
        WHERE id = :id AND is_deleted = 1
    )SQL";

    if (!query.prepare(sql)) {
        return ApiError::fromSqlError(query.lastError());
    }

    query.bindValue(":id", id);
    query.bindValue(":new_parent_id", document.parentId.isEmpty() ? QVariant() : document.parentId);
    query.bindValue(":new_level", document.level);

    if (!query.exec()) {
        return ApiError::fromSqlError(query.lastError());
    }

    if (query.numRowsAffected() == 0) {
        return ApiError::notFound("Deleted document", id);
    }

    return ApiError();
}

ApiError DocumentDbRepo::listDeleted(QVector<DocumentDetail> &documents)
{
    QSqlDatabase db = getConnection();
    QSqlQuery query(db);

    QString sql = R"SQL(
        SELECT
            `id`,
            `parent_id`,
            `name`,
            `description`,
            `type`,
            `extension`,
            `level`,
            `write_access`,
            `read_access`,
            `is_deleted`,
            `created_at`,
            `updated_at`
        FROM
            `piscada_system`.`documents`
        WHERE
            `is_deleted` = 1;
    )SQL";

    if (!query.prepare(sql))
    {
        return ApiError::fromSqlError(query.lastError());
    }

    if (!query.exec())
    {
        return ApiError::fromSqlError(query.lastError());
    }

    while (query.next())
    {
        DocumentDetail doc;
        doc.id = query.value("id").toString();
        doc.parentId = query.value("parent_id").isNull() ? "" : query.value("parent_id").toString();
        doc.name = query.value("name").toString();
        doc.description = query.value("description").isNull() ? "" : query.value("description").toString();
        doc.type = query.value("type").toString();
        doc.extension = query.value("extension").isNull() ? "" : query.value("extension").toString();
        doc.level = query.value("level").toInt();
        doc.writeAccess = query.value("write_access").isNull() ? -1 : query.value("write_access").toInt();
        doc.readAccess = query.value("read_access").isNull() ? -1 : query.value("read_access").toInt();

        documents.append(doc);
    }

    return ApiError();
}


QSharedPointer<FolderNode> DocumentDbRepo::buildFolderTree(const QString &rootId, const DocumentDetail &rootFolder, const QVector<DocumentDetail> &flatList)
{
    QHash<QString, QSharedPointer<FolderNode>> folderMap;

    auto root = QSharedPointer<FolderNode>::create();
    root->id = rootFolder.id;
    root->name = rootFolder.name;
    root->type = rootFolder.type;
    folderMap.insert(rootId, root);

    for (const auto &item : flatList)
    {
        if (item.type == "folder")
        {
            auto node = QSharedPointer<FolderNode>::create();
            node->id = item.id;
            node->name = item.name;
            root->type = rootFolder.type;
            folderMap.insert(item.id, node);
        }
    }

    for (const auto &item : flatList)
    {
        if (!folderMap.contains(item.parentId))
            continue;

        auto parent = folderMap[item.parentId];

        if (item.type == "folder")
        {
            auto child = folderMap[item.id];
            parent->subFolders.append(child);
        }
        else
        {
            parent->files.append(item);
        }
    }

    return root;
}

QSharedPointer<FolderNode> DocumentDbRepo::buildAllTree(const QVector<DocumentDetail> &flatList)
{
    QHash<QString, QSharedPointer<FolderNode>> folderMap;

    auto root = QSharedPointer<FolderNode>::create();
    root->id = "";
    root->name = "root";
    root->type = "";

    for (const auto &item : flatList)
    {
        if (item.type == "folder")
        {
            auto node = QSharedPointer<FolderNode>::create();
            node->id = item.id;
            node->name = item.name;
            node->type = item.type;
            folderMap.insert(item.id, node);
        }
    }

    for (const auto &item : flatList)
    {
        if (item.type == "folder")
        {
            auto current = folderMap[item.id];
            if (item.parentId.isEmpty() || !folderMap.contains(item.parentId))
            {
                root->subFolders.append(current); 
            }
            else
            {
                folderMap[item.parentId]->subFolders.append(current);
            }
        }
        else
        {
            if (item.parentId.isEmpty() || !folderMap.contains(item.parentId))
            {
                root->files.append(item); 
            }
            else
            {
                folderMap[item.parentId]->files.append(item);
            }
        }
    }

    return root;
}

FolderPlainNode DocumentDbRepo::convertToPlainNode(const QSharedPointer<FolderNode> &node)
{
    FolderPlainNode plain;
    plain.id = node->id;
    plain.name = node->name;
    plain.type = node->type;
    for (const auto &doc : node->files) {
        plain.files.append(SimpleDocument{doc.id, doc.name, doc.extension, doc.type});
    }
    for (const auto &childPtr : node->subFolders) {
        plain.subFolders.append(convertToPlainNode(childPtr));
    }
    return plain;
}

