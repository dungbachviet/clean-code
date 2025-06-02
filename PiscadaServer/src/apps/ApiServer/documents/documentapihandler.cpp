#include "documents/documentapihandler.h"
#include "documents/documenttypes.h"
#include "documents/documentdbrepo.h"
#include "documents/documentservices.h"

#include <QDebug>
#include <QJsonObject>
DocumentApiHandler::DocumentApiHandler(PiMqttClient *client) : TypedApiHandler(client)
{
    m_handlers.insert("createFolder", &DocumentApiHandler::createFolder);
    m_handlers.insert("getFolder", &DocumentApiHandler::getFolder);
    m_handlers.insert("updateFolder", &DocumentApiHandler::updateFolder);
    m_handlers.insert("deleteFolder", &DocumentApiHandler::deleteFolder);

    m_handlers.insert("startFileUpload", &DocumentApiHandler::startFileUpload);
    m_handlers.insert("uploadFileChunk", &DocumentApiHandler::uploadFileChunk);
    m_handlers.insert("finishFileUpload", &DocumentApiHandler::finishFileUpload);

    m_handlers.insert("getFile", &DocumentApiHandler::getFile);
    m_handlers.insert("updateFile", &DocumentApiHandler::updateFile);
    m_handlers.insert("deleteFile", &DocumentApiHandler::deleteFile);
    
    m_handlers.insert("archiveFile", &DocumentApiHandler::archiveFile);   
    m_handlers.insert("unarchiveFile", &DocumentApiHandler::unarchiveFile);

    m_handlers.insert("moveDocument", &DocumentApiHandler::moveDocument);
    m_handlers.insert("listAllDocuments", &DocumentApiHandler::listAllDocuments);
    m_handlers.insert("getChildrenOfFolder", &DocumentApiHandler::getChildrenOfFolder);
    m_handlers.insert("listArchivedFiles", &DocumentApiHandler::listArchivedFiles);
    m_handlers.insert("getFullTree", &DocumentApiHandler::getFullTree);

}
DocumentApiHandler::~DocumentApiHandler() = default;

void DocumentApiHandler::createFolder(const PiMqttMessage &msg)
{
    FolderCreateRequest request;

    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Validate name uniqueness
    if (DocumentDbRepo::doesNameExist(request.name, request.parentId, request.type, request.extension))
    {
        return sendErrorResponse(msg, ApiError{ApiError::ConflictError, "This name already exists in the parent folder"});
    }

    // Validate parent folder
    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    DocumentDetail document;
    document.name = request.name;
    document.description = request.description;
    document.parentId = request.parentId;
    document.type = "folder";
    document.extension = request.extension;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;
    document.id = QUuid::createUuid().toString().remove("{").remove("}");

    // Insert the folder into the database
    ApiError error = DocumentDbRepo::create(document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::getFolder(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }
    
    // Fetch folder details from database
    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::updateFolder(const PiMqttMessage &msg)
{
    // Extract folder ID from topic
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Parse JSON payload to get update request
    DocumentUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }
    
    // Retrieve the existing folder from database
    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Check if the new name folder existed in parent folder
    if ((document.name != request.name) && (DocumentDbRepo::doesNameExist(request.name, document.parentId, document.type, document.extension)))
    {
        return sendErrorResponse(msg, ApiError{ApiError::ConflictError, "This name already exists in the parent folder"});
    }

    document.name = request.name;
    document.description = request.description;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;

    // Save updated folder back to database
    error = DocumentDbRepo::update(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::deleteFolder(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Retrieve the folder from the database to confirm it exists
    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Delete all descendant files of the folder in system
    QVector<DocumentDetail> descendantFiles;
    error = DocumentDbRepo::getAllDescendantFiles(descendantFiles, id);
    if (error.isError()) 
    {
        return sendErrorResponse(msg, error);
    }
    for (const DocumentDetail &document : descendantFiles) 
    {
        error = DocumentServices::deleteFileFromSystem(document.id, document.extension, DocumentServices::documentPath);
        if (error.isError()) 
        {
            return sendErrorResponse(msg, error);  
        }
    }

    // Delete the folder from the database
    error = DocumentDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

// Fetch all records from the documents table
void DocumentApiHandler::listAllDocuments(const PiMqttMessage &msg)
{
    QVector<DocumentDetail> documents;

    ApiError error = DocumentDbRepo::list(documents, false);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, documents);
}

void DocumentApiHandler::getChildrenOfFolder(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Query the database for child documents of the given folder
    QVector<DocumentDetail> documents;
    error = DocumentDbRepo::getChildrenOfFolder(id, documents);

    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, documents);
}

void DocumentApiHandler::startFileUpload(const PiMqttMessage &msg)
{
    FileStartRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Validate request parameters
    if (DocumentDbRepo::doesNameExist(request.name, request.parentId, "file", request.extension))
    {
        return sendErrorResponse(msg, ApiError{ApiError::ConflictError, "Name already exists in the parent folder"});
    }
    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    // Create id for the file upload session
    QString id = QUuid::createUuid().toString().remove("{").remove("}");

    QJsonObject result {
        { "id", id },
        { "name", request.name },
        { "extension", request.extension },
        { "totalChunks", request.totalChunks }
    };
    sendResponse(msg, result);
}

void DocumentApiHandler::uploadFileChunk(const PiMqttMessage &msg)
{
    FileChunkRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Validate request parameters
    if (request.id.isEmpty())
    {
        return sendErrorResponse(msg, ApiError{ApiError::InvalidRequest, "File's id is missing"});
    }
    if (request.chunkIndex < 0 || request.chunkIndex >= request.totalChunks)
    {
        return sendErrorResponse(msg, ApiError{ApiError::InvalidRequest, "Invalid chunkIndex"});
    }

    // Validate mime type 
    if (request.chunkIndex == 0)
    {
        ApiError mimeError = DocumentServices::validateMimeType(request.chunkBase64.toUtf8(), request.extension);
        if (mimeError.isError())
        {
            return sendErrorResponse(msg, mimeError);
        }
    }

    // Save the base64 chunk file to a temporary folder
    ApiError saveError = DocumentServices::saveBase64ChunkToTemp(request.id, request.chunkIndex, request.chunkBase64);
    if (saveError.isError())
    {
        return sendErrorResponse(msg, saveError);
    }

    QJsonObject result{
        { "id", request.id },
        { "name", request.name },
        { "extension", request.extension },
        { "totalChunks", request.totalChunks },
        { "chunkIndex", request.chunkIndex },
    };
    sendResponse(msg, result);
}

// This function is called when all file chunks have been uploaded
void DocumentApiHandler::finishFileUpload(const PiMqttMessage &msg)
{
    FileFinishRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Validate request parameters
    if (request.id.isEmpty())
    {
        return sendErrorResponse(msg, ApiError{ApiError::InvalidRequest, "File's id is missing"});
    }
    if (DocumentDbRepo::doesNameExist(request.name, request.parentId, "file", request.extension))
    {
        return sendErrorResponse(msg, ApiError{ApiError::ConflictError, "Name already exists in the parent folder"});
    }
    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    // Merge all base64 chunks into a single file
    ApiError mergeError = DocumentServices::mergeBase64ChunksToFile(request.id, request.extension, request.totalChunks);
    if (mergeError.isError())
    {
        return sendErrorResponse(msg, mergeError);
    }
    DocumentServices::deleteTempFolder(request.id);

    // Create a DocumentDetail object to save to the database
    DocumentDetail document;
    document.id = request.id;
    document.name = request.name;
    document.description = request.description;
    document.parentId = request.parentId;
    document.type = "file";
    document.extension = request.extension;
    document.readAccess = request.readAccess;
    document.writeAccess = request.writeAccess;

    ApiError error = DocumentDbRepo::create(document);
    if (error.isError())
    {
        DocumentServices::deleteFileFromSystem(document.id, document.extension, DocumentServices::documentPath);
        return sendErrorResponse(msg, error);
    }

    QJsonObject result{
        { "id", request.id },
        { "name", request.name },
        { "parentId", request.parentId },
        { "extension", request.extension },
        { "totalChunks", request.totalChunks },
    };
    return sendResponse(msg, result);
}

void DocumentApiHandler::getFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Fetch file metadata from database
    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Read file content from file system 
    QByteArray fileData;
    ApiError fileError = DocumentServices::readFileFromSystem(document.id, document.extension, &fileData);
    if (fileError.isError())
    {
        return sendErrorResponse(msg, fileError);
    }

    // Encode to base64 and separate it
    QByteArray base64Data = fileData.toBase64();
    int chunkSize = 1024 * 1024; // 1MB
    int totalChunks = base64Data.size() / chunkSize + (base64Data.size() % chunkSize != 0 ? 1 : 0);

    for (int i = 0; i < totalChunks; ++i)
    {
        QByteArray chunk = base64Data.mid(i * chunkSize, chunkSize);

        FileChunkResponse chunkResponse{
            .id = document.id,
            .parentId = document.parentId,
            .name = document.name,
            .description = document.description,
            .type = document.type,
            .extension = document.extension,
            .writeAccess = document.writeAccess,
            .readAccess = document.readAccess,
            .chunkBase64 = QString::fromUtf8(chunk),
            .chunkIndex = i,
            .totalChunks = totalChunks
        };

        sendResponse(msg, chunkResponse);
        qDebug() << "Sending chunk" << i << "/" << totalChunks << ":" << QString::fromUtf8(chunk);
    }
    QJsonObject result{
        { "status", "completed" },
        { "id", document.id },
        { "name", document.name },
        { "parentId", document.parentId },
        { "extension", document.extension },
        { "description", document.description },
        { "type", document.type },
        { "readAccess", document.readAccess },
        { "writeAccess", document.writeAccess },
        { "totalChunks", totalChunks },
    };
    return sendResponse(msg, result);
}

void DocumentApiHandler::updateFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Parse the JSON payload 
    DocumentUpdateRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Retrieve file from database
    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Check if the file name already existed with same type in the parent folder
    if ((document.name != request.name) && (DocumentDbRepo::doesNameExist(request.name, document.parentId, document.type, document.extension)))
    {
        return sendErrorResponse(msg, ApiError{ApiError::ConflictError, "This name already exists in the parent folder"});
    }

    document.name = request.name;
    document.description = request.description;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;

    // Save updated file back to database
    error = DocumentDbRepo::update(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::deleteFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Retrieve file from database
    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Delete  file from system
    error = DocumentServices::deleteFileFromSystem(document.id, document.extension, DocumentServices::documentPath);
    if (error.isError()) 
    {
        return sendErrorResponse(msg, error);
    }

    // Delete file from db
    error = DocumentDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

// This function moves a document (a file or a folder) to the new destination folder
void DocumentApiHandler::moveDocument(const PiMqttMessage &msg) {

    DocumentMoveRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);

    if (jsonError.type() != JsonError::NoError) 
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Validate the parent folder
    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    // Retrieve the existing metadata from database
    DocumentDetail document;
 
    ApiError error = DocumentDbRepo::read(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Check if the name already existed with same type in the destination folder
    if (DocumentDbRepo::doesNameExist(document.name, request.parentId, document.type, document.extension))
    {
        return sendErrorResponse(msg, ApiError{ApiError::ConflictError, "This name already exists in the parent folder"});
    }

    // Update in the database
    document.parentId = request.parentId;
    error = DocumentDbRepo::update(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::archiveFile(const PiMqttMessage &msg)
{
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Retrieve file from database
    DocumentDetail document;
    error = DocumentDbRepo::read(id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Update the file to mark it as deleted
    error = DocumentDbRepo::archiveFile(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }
    sendResponse(msg, document);

}

void DocumentApiHandler::unarchiveFile(const PiMqttMessage &msg)
{
    DocumentRestoreRequest request;
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Validate the parent folder which will contain file after restoration
    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    // Retrieve the file from the database
    DocumentDetail document;
    ApiError error = DocumentDbRepo::read(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    // Check if a file with the same name exists in the destination folder
    if (DocumentDbRepo::doesNameExist(document.name, request.parentId, document.type, document.extension))
    {
        return sendErrorResponse(msg, ApiError{ApiError::ConflictError, "This name already exists in the parent folder"});
    }

    // Restore the document in the database
    document.parentId = request.parentId;
    error = DocumentDbRepo::unarchiveFile(request.id, document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }
    sendResponse(msg, document);
}

// Retrieve a list of deleted (archived) files from the database
void DocumentApiHandler::listArchivedFiles(const PiMqttMessage &msg)
{
    QVector<DocumentDetail> deletedFiles; 

    ApiError error = DocumentDbRepo::list(deletedFiles, true);
    
    if (error.isError()) 
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, deletedFiles);
}

void DocumentApiHandler::getFullTree(const PiMqttMessage &msg)
{

    // Retrieve a list of all documents from the database
    QVector<DocumentDetail> documentList;

    ApiError err = DocumentDbRepo::list(documentList, false);

    if (err.isError())
    {
        return sendErrorResponse(msg, err);
    }

    // Build the full tree structure from the list of documents
    QSharedPointer<FolderNode> rootTreePtr = DocumentServices::buildFullTree(documentList);

    // Convert the tree structure from pointer to a plain node and send it as the response
    sendResponse(msg, DocumentServices::convertToPlainNode(rootTreePtr));
}