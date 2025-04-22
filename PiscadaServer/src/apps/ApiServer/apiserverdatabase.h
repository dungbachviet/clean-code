#ifndef PISCADA_API_SERVER_DATABASE_H
#define PISCADA_API_SERVER_DATABASE_H

#include <QSqlDatabase>

QSqlDatabase getConnection();

QString createUuid();

#endif // PISCADA_API_SERVER_DATABASE_H
