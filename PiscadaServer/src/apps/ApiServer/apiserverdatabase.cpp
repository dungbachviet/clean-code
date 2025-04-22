#include "apiserverdatabase.h"

#include "dbadmin.h"
#include "serverinfo.h"

#include <QUuid>


QSqlDatabase getConnection()
{
    ServerInfo *serverInfo = ServerInfo::instance();
    DBAdmin dbAdmin(serverInfo->getServerUrlAsString());

    return dbAdmin.getDB();
}


QString createUuid()
{
    return QUuid::createUuid().toString().mid(1, 36);
}

