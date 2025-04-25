#include "documents/documentapihandler.h"
#include "documents/documenttypes.h"
#include "documents/documentdbrepo.h"
#include "documents/documentservices.h"

#include <QDebug>

DocumentApiHandler::DocumentApiHandler(PiMqttClient *client) : TypedApiHandler(client)
{
    m_handlers.insert("createFolder", &DocumentApiHandler::createFolder);
    m_handlers.insert("getFolder", &DocumentApiHandler::getFolder);
    m_handlers.insert("updateFolder", &DocumentApiHandler::updateFolder);
    m_handlers.insert("deleteFolder", &DocumentApiHandler::deleteFolder);

    m_handlers.insert("createFile", &DocumentApiHandler::createFile);
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
    DocumentCreateRequest request;

    // Parse JSON from MQTT payload
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }

    // Validate name uniqueness
    if (DocumentDbRepo::doesNameExist(request.name, request.parentId, request.type))
    {
        return sendErrorResponse(msg, ApiError::conflictError("This name already exists in the parent folder"));
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
    document.type = request.type;
    document.extension = request.extension;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;
    document.id = QUuid::createUuid().toString(QUuid::WithoutBraces);

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
    // Extract folder ID from the MQTT topic    
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
    // Extract folder ID from the topic
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

    // Delete the folder from the database
    error = DocumentDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

void DocumentApiHandler::listAllDocuments(const PiMqttMessage &msg)
{
    //Fetch all records from the documents table

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
    //Extract the folder ID from the topic
    QString id;
    ApiError error = getStringIdFromTopic(msg.topic(), id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    //Query the database for child documents of the given folder

    QVector<DocumentDetail> documents;
    error = DocumentDbRepo::getChildrenOfFolder(id, documents);

    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, documents);
}

void DocumentApiHandler::createFile(const PiMqttMessage &msg)
{
    FileCreateRequest request;
    // Parse JSON payload into FileCreateRequest
    JsonError jsonError = JsonUtil::decode(msg.payload(), &request);
    if (jsonError.type() != JsonError::NoError)
    {
        return sendErrorResponse(msg, ApiError::fromJsonError(jsonError));
    }
    // Check if a file with the same name already exists in the parent folder
    if (DocumentDbRepo::doesNameExist(request.name, request.parentId, request.type))
    {
        return sendErrorResponse(msg, ApiError::conflictError("This name already exists in the parent folder"));
    }

    // Validate that the parent folder exists
    if (!DocumentDbRepo::isValidParent(request.parentId))
    {
        return sendErrorResponse(msg, ApiError::notFound("Parent folder", request.parentId));
    }

    DocumentDetail document;
    document.name = request.name;
    document.description = request.description;
    document.parentId = request.parentId;
    document.type = request.type;
    document.extension = request.extension;
    document.writeAccess = request.writeAccess;
    document.readAccess = request.readAccess;
    document.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    // Decode base64 content and validate MIME type
    QByteArray decodedContent;
    ApiError decodeError = DocumentServices::decodeAndValidateBase64File(request.fileContent.toUtf8(), document.extension, decodedContent);
    if (decodeError.isError()) 
    { 
        return sendErrorResponse(msg, decodeError);
    }

    // Save decoded file to the file system
    ApiError fileError = DocumentServices::saveFileToSystem(document.id, document.extension, decodedContent);

    if (fileError.isError())
    {
        return sendErrorResponse(msg, fileError);
    }

    // Store file to the database
    ApiError error = DocumentDbRepo::create(document);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }
    sendResponse(msg, document);
}

void DocumentApiHandler::getFile(const PiMqttMessage &msg)
{
    // Extract file ID from MQTT topic
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

    // Read file content from file system (base64 encoded)
    QString fileContentBase64;
    ApiError fileError = DocumentServices::readFileFromSystem(document.id, document.extension, fileContentBase64);
    if (fileError.isError())
    {
        return sendErrorResponse(msg, fileError);
    }

    FileResponse response;
    response.id = document.id;
    response.parentId = document.parentId;
    response.name = document.name;
    response.description = document.description;
    response.type = document.type;
    response.extension = document.extension;
    response.writeAccess = document.writeAccess;
    response.readAccess = document.readAccess;
    response.fileContent = fileContentBase64;

    sendResponse(msg, response);
}

void DocumentApiHandler::updateFile(const PiMqttMessage &msg)
{
    // Extract file ID from MQTT topic
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
    // Extract file ID from MQTT topic
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

    // Delete the file from the file system
    error = DocumentDbRepo::remove(id);
    if (error.isError())
    {
        return sendErrorResponse(msg, error);
    }

    sendResponse(msg, document);
}

// This function moves a document (a file or a folder) to the new destination folder
void DocumentApiHandler::moveDocument(const PiMqttMessage &msg) {

    // Extract the document ID from the topic
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
    if (DocumentDbRepo::doesNameExist(document.name, request.parentId, document.type))
    {
        return sendErrorResponse(msg, ApiError::conflictError("This name already exists in the parent folder"));
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
    // Extract the document ID from the topic
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
    // Parse the JSON payload 
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
    if (DocumentDbRepo::doesNameExist(document.name, request.parentId, document.type))
    {
        return sendErrorResponse(msg, ApiError::conflictError("This name already exists in the parent folder"));
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

void DocumentApiHandler::listArchivedFiles(const PiMqttMessage &msg)
{
    // Retrieve a list of deleted (archived) files from the database

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