#include "users/usersapihandler.h"


UsersApiHandler::UsersApiHandler(PiMqttClient *client) : TypedApiHandler(client)
{
    m_handlers.insert("read", &UsersApiHandler::readUser);
    m_handlers.insert("list", &UsersApiHandler::listUsers);
    m_handlers.insert("create", &UsersApiHandler::createUser);
    m_handlers.insert("update", &UsersApiHandler::updateUser);
    m_handlers.insert("delete", &UsersApiHandler::removeUser);
}

UsersApiHandler::~UsersApiHandler() = default;

void UsersApiHandler::readUser(const PiMqttMessage &msg)
{
}

void UsersApiHandler::listUsers(const PiMqttMessage &msg)
{
}

void UsersApiHandler::createUser(const PiMqttMessage &msg)
{
}

void UsersApiHandler::updateUser(const PiMqttMessage &msg)
{
}

void UsersApiHandler::removeUser(const PiMqttMessage &msg)
{
}

