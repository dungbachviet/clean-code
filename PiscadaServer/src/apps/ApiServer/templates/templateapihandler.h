#ifndef PISCADA_API_SERVER_TEMPLATE_API_HANDLER_H
#define PISCADA_API_SERVER_TEMPLATE_API_HANDLER_H

#include "apihandler.h"


class TemplateApiHandler : public TypedApiHandler<TemplateApiHandler>
{
public:   
    TemplateApiHandler(PiMqttClient *client);
    ~TemplateApiHandler() override;

private:
    void read(const PiMqttMessage &msg);
    void list(const PiMqttMessage &msg);
    void create(const PiMqttMessage &msg);
    void clone(const PiMqttMessage &msg);
    void update(const PiMqttMessage &msg);
    void remove(const PiMqttMessage &msg);
};

#endif // PISCADA_API_SERVER_TEMPLATE_API_HANDLER_H
