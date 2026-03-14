# KanoopDatabaseQt

Lightweight Qt database abstraction library providing credential management, data-source lifecycle, SQL parsing, and a query-loadable interface. Used as the database layer across EPC and Kanoop Qt projects.

**[API Documentation](https://StevePunak.github.io/KanoopDatabaseQt/)** | [Class List](https://StevePunak.github.io/KanoopDatabaseQt/annotated.html) | [Class Hierarchy](https://StevePunak.github.io/KanoopDatabaseQt/hierarchy.html) | [Files](https://StevePunak.github.io/KanoopDatabaseQt/files.html)

## Requirements

- C++11
- Qt 6.7.0+ (Core, Sql)
- CMake 3.16+
- [KanoopCommonQt](https://github.com/StevePunak/KanoopCommonQt)

## Building

KanoopDatabaseQt is typically built as part of the [meta-qt-mains](https://github.com/epcpower/meta-qt-mains) superproject. To build standalone, clone [KanoopCommonQt](https://github.com/StevePunak/KanoopCommonQt) as a sibling and use a workspace CMakeLists.txt:

```bash
# Clone both repos side-by-side
git clone https://github.com/StevePunak/KanoopCommonQt.git
git clone https://github.com/StevePunak/KanoopDatabaseQt.git

# Create a workspace CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(KanoopDatabaseQt-workspace)
add_subdirectory(KanoopCommonQt)
add_subdirectory(KanoopDatabaseQt)
EOF

# Build
cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/gcc_64
cmake --build build --parallel
```

## Classes

| Class | Header | Description |
|-------|--------|-------------|
| [**DataSource**](https://StevePunak.github.io/KanoopDatabaseQt/classDataSource.html) | `datasource.h` | Abstract base for database-backed controllers (MVC pattern). Manages connection lifecycle, query execution, migration, and integrity checking. Supports SQLite, MySQL, and PostgreSQL. |
| [**DatabaseCredentials**](https://StevePunak.github.io/KanoopDatabaseQt/classDatabaseCredentials.html) | `databasecredentials.h` | Value class encapsulating host, schema, username, password, and engine type for database connections. |
| [**SqlParser**](https://StevePunak.github.io/KanoopDatabaseQt/classSqlParser.html) | `sqlparser.h` | Parses multi-statement SQL strings into individual statements, stripping comments and blank lines. |
| [**QueryLoadable**](https://StevePunak.github.io/KanoopDatabaseQt/classQueryLoadable.html) | `queryloadable.h` | Pure abstract interface for objects that can populate themselves from a `QSqlQuery` result set. |

## Usage

### Subclassing DataSource

```cpp
#include <Kanoop/database/datasource.h>

class MyDatabase : public DataSource
{
public:
    MyDatabase(const QString& dbPath)
        : DataSource(DatabaseCredentials(dbPath)) {}

protected:
    QString createSql() const override
    {
        return "CREATE TABLE items ("
               "  id INTEGER PRIMARY KEY,"
               "  name TEXT NOT NULL,"
               "  created_at TEXT"
               ");";
    }

    bool migrate() override
    {
        // Run schema migrations here
        return true;
    }
};

// Open (auto-creates if SQLite file doesn't exist)
MyDatabase db("/path/to/app.db");
if (db.openConnection()) {
    // Use db...
    db.closeConnection();
}
```

### Implementing QueryLoadable

```cpp
#include <Kanoop/database/queryloadable.h>

class Item : public QueryLoadable
{
public:
    bool loadFromQuery(const QSqlQuery& query) override
    {
        _id   = query.value("id").toInt();
        _name = query.value("name").toString();
        return true;
    }

private:
    int _id = 0;
    QString _name;
};
```

## Testing

Unit tests use Qt6::Test and cover all four classes:

```bash
# Build and run tests
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

| Test | Description |
|------|-------------|
| `tst_databasecredentials` | Constructors, getters/setters, validity, engine detection |
| `tst_sqlparser` | Statement parsing, comment stripping, multi-line SQL, edge cases |
| `tst_datasource` | Connection lifecycle, query execution, prepared statements, string escaping, foreign key enforcement |

## CI

[![CI](https://github.com/StevePunak/KanoopDatabaseQt/actions/workflows/ci.yaml/badge.svg)](https://github.com/StevePunak/KanoopDatabaseQt/actions/workflows/ci.yaml)

Builds and runs tests on every push using GitHub Actions (`ubuntu-latest`, Qt 6.10.1, Ninja).

## API Documentation

Full Doxygen documentation is published to GitHub Pages:

**https://StevePunak.github.io/KanoopDatabaseQt/**

## Project Structure

```
KanoopDatabaseQt/
  include/Kanoop/database/   Public headers
  src/database/              Implementation files
  tests/                     Unit tests (Qt6::Test)
  docs/                      Generated documentation (not committed)
  CMakeLists.txt             Build configuration
```

## License

[MIT](LICENSE)
