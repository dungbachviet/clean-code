#ifndef PISCADA_USERS_API_HANDLER_H
#define PISCADA_USERS_API_HANDLER_H

#include "apihandler.h"


class UsersApiHandler : public TypedApiHandler<UsersApiHandler>
{
public:
    UsersApiHandler(PiMqttClient *client);
    ~UsersApiHandler() override;

    void handle(const PiMqttMessage &msg) override;

private:
    void readUser(const PiMqttMessage &msg);
    void listUsers(const PiMqttMessage &msg);
    void createUser(const PiMqttMessage &msg);
    void updateUser(const PiMqttMessage &msg);
    void removeUser(const PiMqttMessage &msg);
};

#endif // PISCADA_USERS_API_HANDLER_H
