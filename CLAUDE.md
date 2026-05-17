# CLAUDE.md — KanoopDatabaseQt

Guidance for Claude Code sessions working with this library. Read this before touching code in this submodule.

## Overview

KanoopDatabaseQt is a thin SQL abstraction over `QSqlDatabase`. It provides a `DataSource` base for connection lifecycle and migration, a credentials value type, a query-loadable interface for entities, and a SQL-statement parser. It supports SQLite, MySQL, and PostgreSQL. It is **not** an ORM, has no caching layer, and provides no transaction wrappers.

- Trunk branch: **`master`** (not `main`).
- Public include path: `<Kanoop/database/...>` — for example `#include <Kanoop/database/datasource.h>`.
- **No export macro** in this library — public classes are unmarked (compiles cleanly on both Linux and Windows because the library is small). Consuming libraries define their own export macros for any `DataSource` subclasses they expose.
- Depends on KanoopCommonQt (uses `Log` and `CommonException`).
- Public surface is just four headers: `datasource.h`, `databasecredentials.h`, `queryloadable.h`, `sqlparser.h`.

## Library conventions

- **Thread affinity is strict.** All operations on a `DataSource` must occur on the thread that called `openConnection()`. The base checks this on every operation and fails (with a log) if violated. Multi-threaded apps create one `DataSource` per worker thread, typically held in a `QMap<QThread*, DataSource*>` keyed by `QThread::currentThread()`.
- **One connection per `DataSource` instance.** No pooling. Give each instance a unique `connectionName` if you open multiple, to avoid colliding in Qt's global `QSqlDatabase` registry.
- **Entity / repository split.** Domain objects implement `QueryLoadable::loadFromQuery()` to self-populate. Repository/DAO methods (`entityInsert`, `entityGet`, `entityUpdate`, `entityDelete`, `entitiesGetAll`) live on a `DataSource` subclass — not on the entity.
- **No ORM.** Write SQL directly. Entities are dumb value objects; the `DataSource` subclass owns all query logic.
- **UTC for timestamps.** `QueryLoadable::utcTime(QVariant)` and the base helper `currentTimestamp()` always return UTC. Don't store local time.
- **Errors via `errorText()`.** Operations return `bool`; on failure call `errorText()` (combines driver + database + native error strings). Critical failures throw `CommonException` from KanoopCommonQt.

## Code style

- **Class names**: PascalCase. **Methods**: camelCase. **Members**: underscore-prefixed camelCase (`_foo`). **Statics**: PascalCase, no prefix.
- Opening brace on same line for `if`/`for`/`while`/`try`; on new line for function/method definitions.
- No space before parens in control flow: `if(cond) {`, not `if (cond) {`.
- Explicit boolean comparison: `if(isReady() == false)` and `if(ok == true)`, not `if(!isReady())`.
- Includes ordered: same-library (`"..."`), then dependency libs (`<Kanoop/...>`), then Qt/system.
- File-local helper **functions** used by one class → `static` private members. File-local **constants** → anonymous namespace is fine.
- Doxygen: single-line `/** @brief ... */`; multi-line with `@brief` on the line after `/**`. **Never** document member variables.
- Single-exit functions preferred; avoid early returns.

## Commonly used classes

### `DataSource` — `<Kanoop/database/datasource.h>`
Abstract base for all database controllers. Manages the connection, runs migration on open, and provides query helpers for subclasses.
```cpp
class MyDatabase : public DataSource {
public:
    MyDatabase(QObject* parent) : DataSource(DatabaseCredentials("/path/to/file.db"), parent) {}
protected:
    QString createSql() const override   { return "CREATE TABLE devices (uuid TEXT PRIMARY KEY, name TEXT);"; }
    bool    migrate() override           { /* version-aware migration */ return true; }
    bool    integrityCheck() override    { return true; }
};

MyDatabase db(nullptr);
if(db.openConnection() == false) {
    Log::logText(LVL_ERROR, db.errorText());
}
// ... query on the same thread ...
db.closeConnection();
```
Pitfalls:
- Override `createSql()` to emit the full schema (used on first open and on recreate).
- `openConnection()` calls `migrate()` then `integrityCheck()` — return `false` from either to abort.
- All subsequent operations must be on the thread that called `openConnection()`. Cross-thread calls log an error and return failure silently.
- Set a unique `connectionName` if opening more than one `DataSource` — Qt's `QSqlDatabase` registry is process-global.
- `createOnOpenFailure` is `true` by default — set `false` if a missing file should error out instead of being created.

### `DatabaseCredentials` — `<Kanoop/database/databasecredentials.h>`
Value type holding host, schema, username, password, engine.
```cpp
// SQLite: schema is the file path
DatabaseCredentials sqlite("/var/lib/myapp/app.db");

// MySQL / PostgreSQL
DatabaseCredentials mysql("localhost", "schema", "user", "pass",
                          DatabaseCredentials::SQLENG_MYSQL);

if(creds.isValid() == false) { /* missing schema */ }
if(creds.isSqlite()) { /* SQLite-specific path */ }
```
Pitfalls: `isValid()` only checks that schema/host is non-empty — it does not test connectivity. Pre-validate credentials at config time, not just before open.

### `QueryLoadable` — `<Kanoop/database/queryloadable.h>`
Pure abstract interface for entities that self-populate from a `QSqlQuery` result row.
```cpp
class Device : public QueryLoadable {
public:
    bool loadFromQuery(const QSqlQuery& q) override {
        for(int i = 0; i < q.record().count(); ++i) {
            const QString col = q.record().fieldName(i);
            if(col == "uuid")        _uuid = q.value(i).toUuid();
            else if(col == "name")   _name = q.value(i).toString();
            else if(col == "created") _created = utcTime(q.value(i));
        }
        return _uuid.isNull() == false;
    }
};

// In the DataSource subclass:
QList<Device> MyDatabase::devicesGetAll() {
    QList<Device> out;
    QSqlQuery q = executeQuery("SELECT * FROM devices");
    while(q.next()) {
        Device d;
        if(d.loadFromQuery(q)) out.append(d);
    }
    return out;
}
```
Pitfalls: iterate columns by field name (`q.record().fieldName(i)`) — column order from `SELECT *` isn't guaranteed across schema versions. Return `false` to signal a corrupt row so the repository can skip it.

### `SqlParser` — `<Kanoop/database/sqlparser.h>`
Splits a multi-statement SQL string on `;`, stripping `--` and `/* */` comments. Used internally by `DataSource::executeMultiple()`. Rarely needed in application code.
```cpp
SqlParser parser(sqlBlob);
if(parser.isValid()) {
    for(const QString& stmt : parser.statements()) executeQuery(stmt);
}
```
Pitfalls: no awareness of `;` inside string literals — assumes well-formed SQL.

## Common gotchas

- **Thread affinity check is silent.** Operations from the wrong thread log to the Kanoop logger and return `false` — they do not throw. If a query "fails" mysteriously and the log shows a thread mismatch, you're calling from the wrong thread; spin up a thread-local `DataSource`.
- **Always test migrations against populated data.** `migrate()` runs on every `openConnection()`. A buggy migration on a populated database can corrupt it. Downstream subclasses that implement a recreate-on-failure path will silently wipe the user's data — test migrations before merging.
- **No transaction wrapper.** If you need atomic multi-statement operations, use `QSqlDatabase::transaction()`/`commit()`/`rollback()` directly via `database()`.
- **Connection name collisions.** Qt's `QSqlDatabase` registry is global. If two `DataSource`s share a `connectionName` (default is auto-generated, but be careful when you set one manually), the second open will replace the first.
- **Never hardcode credentials.** Read from config / environment at runtime. Don't check `.db` files with embedded credentials into the repo.
- **SQLite foreign keys.** Disabled by default in SQLite, but `DataSource::openConnection()` enables them automatically. Don't rely on inserts that violate FKs "just working" — they will fail.
- **Repository methods return `bool`.** Failures are logged internally but not thrown. Always check the return value at the call site.

## Working in this repo

- When this library is consumed as a git submodule, the submodule will typically be in **detached HEAD** state pointing at a pinned commit. Check out a working branch before committing.
- Conventional-commits message format is used: `<type>(<scope>): <description>` with types `fix`, `feat`, `doc`, `refactor`, `test`, `chore`. Add a `Co-Authored-By` trailer if pair-programming.
- Submodule pointer changes belong in the consuming repo, not here.
