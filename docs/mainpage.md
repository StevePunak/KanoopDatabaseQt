# KanoopDatabaseQt {#mainpage}

Qt database abstraction library providing connection management, credential handling, query loading, and SQL parsing utilities.

## Classes

| Class | Description |
|-------|-------------|
| `DataSource` | Database connection manager with connection pooling and thread-safe access |
| `DatabaseCredentials` | Connection credential container (host, port, database, user, password) |
| `IQueryLoadable` | Interface for objects that can populate themselves from a SQL query |
| `SqlParser` | SQL statement parser for extracting table names and query metadata |

## Quick Start

```cpp
#include <Kanoop/database/datasource.h>
#include <Kanoop/database/databasecredentials.h>

DatabaseCredentials creds("localhost", 3306, "mydb", "user", "pass");
DataSource ds(creds);

if (ds.open()) {
    QSqlQuery query = ds.createQuery("SELECT * FROM devices");
    // ...
}
```

## Links

- [GitHub Repository](https://github.com/StevePunak/KanoopDatabaseQt)
- [Class List](annotated.html)
- [File List](files.html)
