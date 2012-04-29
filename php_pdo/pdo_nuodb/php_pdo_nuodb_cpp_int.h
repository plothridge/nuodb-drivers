#ifndef PHP_PDO_NUODB_CPP_INT_H
#define PHP_PDO_NUODB_CPP_INT_H

#include <nuodb/sqlapi/SqlConnection.h>
#include <nuodb/sqlapi/SqlEnvironment.h>
#include <nuodb/sqlapi/SqlConnection.h>
#include <nuodb/sqlapi/SqlColumnMetaData.h>
#include <nuodb/sqlapi/SqlResultSet.h>
#include <nuodb/sqlapi/SqlDatabaseMetaData.h>
#include <nuodb/sqlapi/SqlPreparedStatement.h>
#include <nuodb/sqlapi/SqlExceptions.h>


using namespace nuodb::sqlapi;


class PdoNuoDbStatement;

class PdoNuoDbHandle {
    private:
        SqlEnvironment *_env;
        SqlConnection *_con;
        SqlOptionArray *_opts;
        SqlOption _opt_arr[4];
        PdoNuoDbStatement *_last_stmt;
        void deleteConnection();
        void deleteEnvrionment();
        void deleteOptions();
    public:
        PdoNuoDbHandle(SqlOptionArray *options);
        ~PdoNuoDbHandle();
        void setOptions(SqlOptionArray *options);
        SqlConnection *createConnection();
        SqlConnection *getConnection();
        PdoNuoDbStatement *createStatement(char const *sql);
        void closeConnection();
        void commit();
        void rollback();
};

class PdoNuoDbStatement {
    private:
        PdoNuoDbHandle *_dbh;
        SqlPreparedStatement *_stmt;
        SqlResultSet *_rs;
    public:
        PdoNuoDbStatement(PdoNuoDbHandle *dbh);
        ~PdoNuoDbStatement();
        SqlPreparedStatement *createStatement(char const *sql);
        void execute();
        void executeQuery();
        bool hasResultSet();
        bool next();
        size_t getColumnCount();
        char const *getColumnName(size_t column);
        int getSqlType(size_t column);
        char const *getString(size_t column);
        unsigned int getInteger(size_t column);
        unsigned long getLong(size_t column);


};

#endif	/* PHP_PDO_NUODB_INT_CPP_H */
