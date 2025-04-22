#ifndef PISCADA_API_SERVER_DOCUMENT_SERVICES_H
#define PISCADA_API_SERVER_DOCUMENT_SERVICES_H

#include "apierror.h"
#include "documents/documenttypes.h"
#include <QString>

class DocumentServices
{
public:
    static ApiError decodeAndValidateBase64File(const QByteArray &fileContent, const QString &expectedExtension, QByteArray &decodedContent);
    static ApiError saveFileToSystem(const QString &id, const QString &extension, const QByteArray &fileContent);
    static ApiError readFileFromSystem(const QString &id, const QString &extension, QString &fileContentBase64);
    static QSharedPointer<FolderNode> buildFolderTree(const QString &rootId, const DocumentDetail &rootFolder, const QVector<DocumentDetail> &flatList);
    static QSharedPointer<FolderNode> buildAllTree(const QVector<DocumentDetail> &flatList);
    static FolderPlainNode convertToPlainNode(const QSharedPointer<FolderNode> &node);
};

#endif // PISCADA_API_SERVER_DOCUMENT_SERVICES_H