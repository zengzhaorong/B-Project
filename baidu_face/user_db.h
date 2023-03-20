#ifndef USER_DB_H
#define USER_DB_H

#include <QSqlDatabase>

#define USER_DB_NAME        "user.db"

#define TABLE_USER_INFO     "user_table"
#define TABLE_HISTORY       "history_table"

#define DB_COL_ID           "id"
#define DB_COL_NAME         "name"

#define USER_NAME_LEN       32

typedef struct {
    int id;
    char name[USER_NAME_LEN];
}user_info_t;


int db_user_add(user_info_t *user);
int db_user_del(int id);

int user_db_init(QSqlDatabase &sql);


#endif // USER_DB_H
