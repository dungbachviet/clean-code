#ifndef PISCADA_API_SERVER_SYSTEM_API_HANDLER_H
#define PISCADA_API_SERVER_SYSTEM_API_HANDLER_H

#include "apihandler.h"


class SystemApiHandler : public TypedApiHandler<SystemApiHandler>
{
public:
    SystemApiHandler(PiMqttClient *client);
    ~SystemApiHandler() override;

private:
    void read(const PiMqttMessage &msg);
    void list(const PiMqttMessage &msg);
    void create(const PiMqttMessage &msg);
    void update(const PiMqttMessage &msg);
    void remove(const PiMqttMessage &msg);
};

#endif // PISCADA_API_SERVER_SYSTEM_API_HANDLER_H
