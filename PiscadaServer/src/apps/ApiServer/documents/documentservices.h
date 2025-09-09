#ifndef PISCADA_API_SERVER_DOCUMENT_SERVICES_H
#define PISCADA_API_SERVER_DOCUMENT_SERVICES_H

#include "apierror.h"
#include "documents/documenttypes.h"
#include <QString>

class DocumentServices
{
public:
    // NOTE: This must match the documentPath defined in dbsetup.cpp
    static constexpr const char* documentPath = "/Piscada/files/documents";
    static constexpr const char* tempDocumentPath = "/Piscada/files/tmp_documents";
    
    static QString getFinalFilePath(const QString &id, const QString &extension);

    static ApiError validateMimeType(const QByteArray &fileContent, const QString &expectedExtension);
    static ApiError saveBase64ChunkToTemp(const QString &id, int chunkIndex, const QString &chunkBase64);
    static ApiError mergeBase64ChunksToFile(const QString &id, const QString &extension, int totalChunks);
    static ApiError deleteTempFolder(const QString &id);

    static ApiError readFileFromSystem(const QString &id, const QString &extension, QByteArray *fileData);
    static ApiError deleteFileFromSystem(const QString &id, const QString &extension, const QString &path);
    static QSharedPointer<FolderNode> buildFullTree(const QVector<DocumentDetail> &documentList);
    static FolderPlainNode convertToPlainNode(const QSharedPointer<FolderNode> &node);

};

#endif // PISCADA_API_SERVER_DOCUMENT_SERVICES_H