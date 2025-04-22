#ifndef PISCADA_API_SERVER_INFO_H
#define PISCADA_API_SERVER_INFO_H

#include <QString>


class ApiServerInfo
{
public:
    ApiServerInfo();
    ~ApiServerInfo();

    bool startUp();
    bool shutDown();

    QString controllerId() const;

private:
    QString m_controllerId;
    bool m_initialized;
};

extern ApiServerInfo info;

#endif // PISCADA_API_SERVER_INFO_H
