#include "documents/documentservices.h"
#include "apiserverinfo.h"
#include "apiserverdatabase.h"
#include <QDebug>
#include <QFile>
#include <QDir>

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
    QDir dir(storageDirectory);

    if (!dir.exists())
    {
        if (!dir.mkpath(storageDirectory))
        {
            return ApiError::internalError("Failed to create directory on the system");
        }
    }
    QString filePath = QString("%1/%2.%3").arg(storageDirectory, id, extension);
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
    QString filePath = QString("%1/%2.%3").arg(storageDirectory, id, extension);
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

QSharedPointer<FolderNode> DocumentServices::buildFullTree(const QVector<DocumentDetail> &documentList)
{
    QHash<QString, QSharedPointer<FolderNode>> folderMap;

    auto rootPtr = QSharedPointer<FolderNode>::create();
    rootPtr->id = "";
    rootPtr->name = "root";
    rootPtr->type = "";

    for (const auto &item : documentList)
    {
        if (item.type == "folder")
        {
            auto folderNodePtr = QSharedPointer<FolderNode>::create();
            folderNodePtr->id = item.id;
            folderNodePtr->name = item.name;
            folderNodePtr->type = item.type;
            folderMap.insert(item.id, folderNodePtr);
        }
    }

    for (const auto &item : documentList)
    {
        if (item.type == "folder")
        {
            auto current = folderMap[item.id];
            if (item.parentId.isEmpty())
            {
                rootPtr->subFolders.append(current); 
            }
            else
            {
                if (folderMap.contains(item.parentId))
                {   
                folderMap[item.parentId]->subFolders.append(current);
                }
            }
        }
        else
        {
            if (item.parentId.isEmpty())
            {
                rootPtr->files.append(item); 
            }
            else
            {
                if (folderMap.contains(item.parentId))
                {
                folderMap[item.parentId]->files.append(item);
                }
            }
        }
    }

    return rootPtr;
}

FolderPlainNode DocumentServices::convertToPlainNode(const QSharedPointer<FolderNode> &folderNodePtr)
{
    FolderPlainNode plainNode;
    plainNode.id = folderNodePtr->id;
    plainNode.name = folderNodePtr->name;
    plainNode.type = folderNodePtr->type;
    for (const auto &doc : folderNodePtr->files) {
        plainNode.files.append(Document{doc.id, doc.name, doc.extension, doc.type});
    }
    for (const auto &childPtr : folderNodePtr->subFolders) {
        plainNode.subFolders.append(convertToPlainNode(childPtr));
    }
    return plainNode;
}

