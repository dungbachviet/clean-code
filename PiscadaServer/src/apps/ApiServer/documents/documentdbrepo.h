#ifndef PISCADA_API_SERVER_DOCUMENT_DB_REPO_H
#define PISCADA_API_SERVER_DOCUMENT_DB_REPO_H

#include "apierror.h"
#include "documents/documenttypes.h"
#include <QString>

class DocumentDbRepo
{
public:
    static bool isNameExists(const QString &name, const QString &parentId, const QString &type);
    static bool isValidParent(const QString &parentId);
    static int getLevel(const QString &parentId);
    static QVector<DocumentDetail> getAllDescendants(const QString &parentId);
    static ApiError updateLevelForDescendants(const QString &documentId, int deltaLevel);
    static ApiError decodeAndValidateBase64File(const QByteArray &fileContent, const QString &expectedExtension, QByteArray &decodedContent);
    static ApiError saveFileToSystem(const QString &id, const QString &extension, const QByteArray &fileContent);
    static ApiError readFileFromSystem(const QString &id, const QString &extension, QString &fileContentBase64);
    
    static ApiError create(DocumentDetail &document);
    static ApiError read(const QString &id, DocumentDetail &document);
    static ApiError list(QVector<DocumentDetail> &documents);
    static ApiError update(const QString &id, DocumentDetail &document);
    static ApiError remove(const QString &id);
    static ApiError listChildren(const QString &id, QVector<DocumentDetail> &documents);
    static ApiError move(const QString &id, DocumentDetail &document);
    static ApiError softDelete(const QString &id);
    static ApiError restore(const QString &id, DocumentDetail &document);
    static ApiError listDeleted(QVector<DocumentDetail> &documents);
    static QSharedPointer<FolderNode> buildFolderTree(const QString &rootId, const DocumentDetail &rootFolder, const QVector<DocumentDetail> &flatList);
    static QSharedPointer<FolderNode> buildAllTree(const QVector<DocumentDetail> &flatList);
    static FolderPlainNode convertToPlainNode(const QSharedPointer<FolderNode> &node);
};

#endif // PISCADA_API_SERVER_DOCUMENT_DB_REPO_H
