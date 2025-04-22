#include "documents/documentservices.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"
#include <QDebug>
#include <QFile>
#include <QDir>

ApiError DocumentServices::decodeAndValidateBase64File(const QByteArray &fileContent, const QString &expectedExtension, QByteArray &decodedContent)
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

ApiError DocumentServices::saveFileToSystem(const QString &id, const QString &extension, const QByteArray &fileContent)
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

ApiError DocumentServices::readFileFromSystem(const QString &id, const QString &extension, QString &fileContentBase64)
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


QSharedPointer<FolderNode> DocumentServices::buildFolderTree(const QString &rootId, const DocumentDetail &rootFolder, const QVector<DocumentDetail> &flatList)
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

QSharedPointer<FolderNode> DocumentServices::buildAllTree(const QVector<DocumentDetail> &flatList)
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

FolderPlainNode DocumentServices::convertToPlainNode(const QSharedPointer<FolderNode> &node)
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

