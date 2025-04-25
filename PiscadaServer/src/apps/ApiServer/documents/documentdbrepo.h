#ifndef PISCADA_API_SERVER_DOCUMENT_DB_REPO_H
#define PISCADA_API_SERVER_DOCUMENT_DB_REPO_H

#include "apierror.h"
#include "documents/documenttypes.h"
#include <QString>

class DocumentDbRepo
{
public:
    static bool doesNameExist(const QString &name, const QString &folderId, const QString &type);
    static bool isValidParent(const QString &parentId);
    
    static ApiError create(DocumentDetail &document);
    static ApiError read(const QString &id, DocumentDetail &document);
    static ApiError update(const QString &id, DocumentDetail &document);
    static ApiError remove(const QString &id);
    static ApiError list(QVector<DocumentDetail> &documents, bool getArchivedFile);
    static ApiError archiveFile(const QString &id);
    static ApiError unarchiveFile(const QString &id, DocumentDetail &document);
    static ApiError getChildrenOfFolder(const QString &folderId, QVector<DocumentDetail> &documents);
};

#endif // PISCADA_API_SERVER_DOCUMENT_DB_REPO_H
