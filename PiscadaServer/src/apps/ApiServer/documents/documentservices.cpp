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
QMap<QByteArray, QString> magicToExt = {
    {QByteArray::fromHex("25504446"), "pdf"},      // %PDF
    {QByteArray::fromHex("89504E47"), "png"},      // PNG
    {QByteArray::fromHex("FFD8FF"),   "jpg"},      // JPEG
    {QByteArray::fromHex("504B0304"), "zip"},      // ZIP, DOCX, XLSX
    {QByteArray::fromHex("D0CF11E0"), "doc"},      // DOC, XLS 
    {QByteArray::fromHex("EFBBBF"),   "txt"}       // UTF-8 BOM
};

QString DocumentServices::getFinalFilePath(const QString &id, const QString &extension)
{
    QDir().mkpath(documentPath); 
    return QString("%1/%2.%3").arg(documentPath, id, extension);
}
ApiError DocumentServices::validateMimeType(const QByteArray &fileContent, const QString &expectedExtension)
{
    const QByteArray headerPrefix = "data:";
    int commaIndex = fileContent.indexOf(',');

    if (fileContent.startsWith(headerPrefix) && commaIndex != -1)
    {
        QByteArray header = fileContent.left(commaIndex);
        QByteArray base64Data = fileContent.mid(commaIndex + 1);
        QByteArray binaryData = QByteArray::fromBase64(base64Data);

        //Seperate the header from the data
        int semiIndex = header.indexOf(';');
        QString mimeType;
        if (semiIndex != -1)
        {
            mimeType = QString::fromUtf8(header.mid(headerPrefix.length(), semiIndex - headerPrefix.length()));
        }

        QString expectedExt = expectedExtension.toLower();

        // Check if the mimeType is in the predefined map
        if (mimeToExt.contains(mimeType))
        {
            if (mimeToExt[mimeType] != expectedExtension.toLower())
            {
                    return ApiError::InvalidRequest;
            }
        }
        else
        {
            // If the mimeType is not in the map, check the magic numbers (signatures in binary data)
            bool matched = false;
            for (const auto &magic : magicToExt.keys()) 
            {
                if (binaryData.startsWith(magic))
                {
                    if (magicToExt[magic] != expectedExt)
                    {
                        return ApiError::InvalidRequest;
                    }
                    matched = true;
                    break;
                }
            }
            if (!matched)
            {
            return ApiError::InvalidRequest;
            }
        }
    }
    return ApiError(); 
}

ApiError DocumentServices::saveBase64ChunkToTemp(const QString &id, int chunkIndex, const QString &chunkBase64)
{
    QString folderPath = QString("%1/%2").arg(tempDocumentPath, id);
    QDir().mkpath(folderPath);

    QString chunkFilePath = QString("%1/chunk_%2").arg(folderPath).arg(chunkIndex, 5, 10, QChar('0')); 
    QFile file(chunkFilePath);

    if (QFile::exists(chunkFilePath)) 
    {
        return ApiError(ApiError::ConflictError, QString("This chunk file index %1 was already sent").arg(chunkIndex));
    }
    if (!file.open(QIODevice::WriteOnly))
    {
        return ApiError(ApiError::InternalError, "Failed to write chunk: " + file.errorString());
    }
    QByteArray chunkData = chunkBase64.toUtf8();
    if (file.write(chunkData) == -1 || !file.flush())
    {
        return ApiError(ApiError::InternalError, "Failed to write data to chunk file");
    }
    file.close();
    return ApiError();
}

ApiError DocumentServices::mergeBase64ChunksToFile(const QString &id, const QString &extension, int totalChunks)
{
    QString folderPath = QString("%1/%2").arg(tempDocumentPath, id);
    QString fullBase64Path = folderPath + "/chunk_full";
    QString finalPath = getFinalFilePath(id, extension);
    
    // Create the chunk_full file where all chunks will be merged to
    QFile fullBase64File(fullBase64Path);
    if (!fullBase64File.open(QIODevice::WriteOnly | QIODevice::Truncate)) 
    {
        return ApiError(ApiError::InternalError, "Cannot create chunk_full file");
    }

    for (int i = 0; i < totalChunks; ++i)
    {
        QString chunkFilePath = QString("%1/chunk_%2").arg(folderPath).arg(i, 5, 10, QChar('0'));
        if (!QFile::exists(chunkFilePath))
        {
            fullBase64File.close();
            return ApiError(ApiError::InternalError, QString("Missing chunk file: %1").arg(chunkFilePath));
        }
        QFile chunkFile(chunkFilePath);
        if (!chunkFile.open(QIODevice::ReadOnly))
        {
            fullBase64File.close();
            return ApiError(ApiError::InternalError, QString("Cannot open chunk file: %1").arg(chunkFilePath));
        }

        QByteArray chunkBase64Data = chunkFile.readAll();
        chunkFile.close();

        if (fullBase64File.write(chunkBase64Data) == -1 || !fullBase64File.flush())
        {
            fullBase64File.close();
            return ApiError(ApiError::InternalError, QString("Failed to write chunk %1 to final file").arg(i));
        }
    }
    fullBase64File.close();

    // Now we have all chunks merged into chunk_full, we can decode it and save to final file
    if (!fullBase64File.open(QIODevice::ReadOnly)) 
    {
        return ApiError(ApiError::InternalError, "Cannot reopen chunk_full for reading");
    }
    QByteArray fullBase64Data = fullBase64File.readAll();
    fullBase64File.close();

    QByteArray decodedData = QByteArray::fromBase64(fullBase64Data);

    QFile file(finalPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return ApiError(ApiError::InternalError, "Cannot open final file");
    }
    if (file.write(decodedData) == -1 || !file.flush()) 
    {
        file.close();
        return ApiError(ApiError::InternalError, "Failed to write decoded data to final file");
    }

    file.close();
    return ApiError();
}

ApiError DocumentServices::deleteTempFolder(const QString &id)
{
    QString folderPath = QString("%1/%2").arg(tempDocumentPath, id);
    QDir dir(folderPath);
    if (dir.exists())
    {
        if (!dir.removeRecursively())
        {
            return ApiError{ApiError::InternalError, "Failed to delete temporary folder: " + folderPath};
        }
    }
    return ApiError();
}

//////////////////==========================

// ApiError DocumentServices::decodeAndValidateBase64File(const QByteArray &fileContent, const QString &expectedExtension, QByteArray &decodedContent)
// {
//     const QByteArray headerPrefix = "data:";
//     int commaIndex = fileContent.indexOf(',');

//     if (fileContent.startsWith(headerPrefix) && commaIndex != -1)
//     {
//         QByteArray header = fileContent.left(commaIndex);
//         QByteArray base64Data = fileContent.mid(commaIndex + 1);
//         decodedContent = QByteArray::fromBase64(base64Data);

//         int semiIndex = header.indexOf(';');
//         QString mimeType;
//         if (semiIndex != -1)
//         {
//             mimeType = QString::fromUtf8(header.mid(headerPrefix.length(), semiIndex - headerPrefix.length()));
//         }

//         if (mimeToExt.contains(mimeType))
//         {
//             QString expectedExt = mimeToExt[mimeType];
//             if (expectedExt != expectedExtension.toLower())
//             {
//                     return ApiError::InvalidRequest;
//             }
//         }
//         else
//         {
//             return ApiError::InvalidRequest;
//         }
//     }
//     else
//     {
//         decodedContent = QByteArray::fromBase64(fileContent);
//     }
//     return ApiError(); 
// }

// ApiError DocumentServices::saveFileToSystem(const QString &id, const QString &extension, const QByteArray &fileContent)
// {
//     QDir dir(documentPath);

//     if (!dir.exists())
//     {
//         if (!dir.mkpath(documentPath))
//         {
//             return ApiError{ApiError::InternalError, "Failed to create directory on the system"};
//         }
//     }
//     QString filePath = QString("%1/%2.%3").arg(documentPath, id, extension);
//     QFile file(filePath);
//     if (!file.open(QIODevice::WriteOnly))
//     {
//         return ApiError{ApiError::InternalError, "Failed to create file on the system"};
//     }
//     file.write(fileContent);
//     file.close();

//     return ApiError();  
// }

ApiError DocumentServices::readFileFromSystem(const QString &id, const QString &extension, QByteArray *fileData)
{
    if (!fileData) 
    {
        return ApiError{ApiError::InvalidRequest, "Null pointer for fileContentBase64"};
    }
    QString filePath = QString("%1/%2.%3").arg(documentPath, id, extension);
    QFile file(filePath);

    if (!file.exists())
    {
        return ApiError::notFound("Document", id);
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        return ApiError{ApiError::InternalError, "Failed to open file on the system"};
    }

    *fileData = file.readAll();
    file.close();

    return ApiError();
}

ApiError DocumentServices::deleteFileFromSystem(const QString &id, const QString &extension, const QString &path)
{
    QString filePath = QString("%1/%2.%3").arg(path, id, extension);
    QFile file(filePath);
    if (file.exists())
    {
        if (!file.remove())
        {
            return ApiError{ApiError::InternalError, "Failed to delete file from system"};
        }
    }
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

    for (const auto &doc : folderNodePtr->files) 
    {
        plainNode.files.append(DocumentDetail{doc.id, doc.name, doc.extension, doc.type, doc.parentId, doc.description, doc.writeAccess, doc.readAccess});  
    }

    for (const auto &childPtr : folderNodePtr->subFolders) 
    {
        plainNode.subFolders.append(convertToPlainNode(childPtr));
    }

    return plainNode;
}
