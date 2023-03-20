#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlQuery>
#include "user_db.h"

static QSqlQuery* sqlquery;


int db_user_add(user_info_t *user)
{
    QString sql_cmd;

    sql_cmd = QString("insert into %1(%2,%3) values(%4,'%5');").arg(TABLE_USER_INFO).arg(DB_COL_ID).arg(DB_COL_NAME).arg(user->id).arg(user->name);
    qDebug() << sql_cmd;
    if(sqlquery->exec(sql_cmd))
        qDebug() << "add user ok." ;
    else
        qDebug() << "add user failed!" ;

    return 0;
}

int db_user_del(int id)
{
    QString sql_cmd;

    sql_cmd = QString("delete from %1 where %2=%3;").arg(TABLE_USER_INFO).arg(DB_COL_ID).arg(id);
    qDebug() << sql_cmd;
    if(sqlquery->exec(sql_cmd))
        qDebug() << "del user ok." ;
    else
        qDebug() << "del user failed!";

    return 0;
}

int db_create_user_tbl(void)
{
    QString sql_cmd;

    sql_cmd = QString("create table if not exists %1(%2 int primary key not null,%3 char(32));").arg(TABLE_USER_INFO).arg(DB_COL_ID).arg(DB_COL_NAME);
    qDebug() << sql_cmd;
    if(sqlquery->exec(sql_cmd))
        qDebug() << "create user table success." ;
    else
        qDebug() << "create car table failed!" ;

    return 0;
}

int user_db_init(QSqlDatabase &sql)
{

    sql = QSqlDatabase::addDatabase("QSQLITE");
    sql.setDatabaseName(USER_DB_NAME);
    if(!sql.open())
    {
        qDebug() << "fail to open sqlite!" ;
    }

    sqlquery = new QSqlQuery(sql);

    db_create_user_tbl();

    qDebug() << "sql db init ok." ;

    return 0;
}
