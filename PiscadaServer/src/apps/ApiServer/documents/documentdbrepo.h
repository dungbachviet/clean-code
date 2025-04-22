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
};

#endif // PISCADA_API_SERVER_DOCUMENT_DB_REPO_H
