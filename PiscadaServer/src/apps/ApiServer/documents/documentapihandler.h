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
    void listAll(const PiMqttMessage &msg);
    void listChildren(const PiMqttMessage &msg);

    void createFile(const PiMqttMessage &msg);
    void getFile(const PiMqttMessage &msg);
    void updateFile(const PiMqttMessage &msg);
    void deleteFile(const PiMqttMessage &msg);
    void moveDocument(const PiMqttMessage &msg);
    void softDeleteFile(const PiMqttMessage &msg);
    void restoreFile(const PiMqttMessage &msg);
    void listDeletedFiles(const PiMqttMessage &msg);
    void getTree(const PiMqttMessage &msg);
    void getAllTree(const PiMqttMessage &msg);

};

#endif // PISCADA_API_SERVER_DOCUMENTS_API_HANDLER_H
