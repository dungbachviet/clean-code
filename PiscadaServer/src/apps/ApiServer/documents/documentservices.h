#ifndef PISCADA_API_SERVER_DOCUMENT_SERVICES_H
#define PISCADA_API_SERVER_DOCUMENT_SERVICES_H

#include "apierror.h"
#include "documents/documenttypes.h"
#include <QString>

class DocumentServices
{
public:
    static constexpr const char* storageDirectory = "/Piscada/documents";

    static ApiError decodeAndValidateBase64File(const QByteArray &fileContent, const QString &expectedExtension, QByteArray &decodedContent);
    static ApiError saveFileToSystem(const QString &id, const QString &extension, const QByteArray &fileContent);
    static ApiError readFileFromSystem(const QString &id, const QString &extension, QString &fileContentBase64);
    static QSharedPointer<FolderNode> buildFullTree(const QVector<DocumentDetail> &documentList);
    static FolderPlainNode convertToPlainNode(const QSharedPointer<FolderNode> &node);
};

#endif // PISCADA_API_SERVER_DOCUMENT_SERVICES_H