#ifndef PISCADA_API_SERVER_DOCUMENTS_API_HANDLER_H
#define PISCADA_API_SERVER_DOCUMENTS_API_HANDLER_H

#include "apihandler.h"


class DocumentApiHandler : public TypedApiHandler<DocumentApiHandler>
{
public:
    DocumentApiHandler(PiMqttClient *client);
    ~DocumentApiHandler();

private:
    void createFolder(const PiMqttMessage &msg);
    void getFolder(const PiMqttMessage &msg);
    void updateFolder(const PiMqttMessage &msg);
    void deleteFolder(const PiMqttMessage &msg);

    // void createFile(const PiMqttMessage &msg);
    void startFileUpload(const PiMqttMessage &msg);
    void uploadFileChunk(const PiMqttMessage &msg);
    void finishFileUpload(const PiMqttMessage &msg);
    void getFile(const PiMqttMessage &msg);
    void updateFile(const PiMqttMessage &msg);
    void deleteFile(const PiMqttMessage &msg);

    void archiveFile(const PiMqttMessage &msg);
    void unarchiveFile(const PiMqttMessage &msg);

    void moveDocument(const PiMqttMessage &msg);
    void listAllDocuments(const PiMqttMessage &msg);
    void getChildrenOfFolder(const PiMqttMessage &msg);
    void listArchivedFiles(const PiMqttMessage &msg);
    void getFullTree(const PiMqttMessage &msg);

};

#endif // PISCADA_API_SERVER_DOCUMENTS_API_HANDLER_H
