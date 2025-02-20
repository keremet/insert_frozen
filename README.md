Some DBMS support autonomous transactions to create log records, but PostgreSQL does not. This PostgreSQL extension can be used to create log records in faster way. The extension contains the `insert_frozen` function to insert frozed rows into a table.

# BUILD

```
#Add path to pg_config
export PATH=/home/keremet/compile/postgresql_bin/bin:$PATH
make
sudo make install
```

# CREATE FUNCTIONS

PostgreSQL doesn't support functions which accept variable numbers of arguments, where the optional arguments are of the different data types (https://www.postgresql.org/docs/17/xfunc-sql.html#XFUNC-SQL-VARIADIC-FUNCTIONS). So you should create one function per each attribute set of the tables you want to insert frozen rows into. The first argument of the function is a table name, the other arguments must have the same types and order as the table attributes.

If a table is created using the query:
```
create table t_log(ts timestamp with time zone, msg text);
```
then the function for the table can be created so:
```
create function insert_frozen(regclass, timestamp with time zone, text) returns void
as '$libdir/insert_frozen.so', 'insert_frozen'
language C;
```

# RUN

Frozen rows remain when the transaction is rolled back.

```
postgres=# create table t (i int);
CREATE TABLE
postgres=# begin;
BEGIN
postgres=*# select insert_frozen('t_log', clock_timestamp(), 'We are ready to insert 10');
 insert_frozen 
---------------
 
(1 row)

postgres=*# insert into t values (10);
INSERT 0 1
postgres=*# select insert_frozen('t_log', clock_timestamp(), 'We have inserted 10');
 insert_frozen 
---------------
 
(1 row)

postgres=*# table t;
 i
----
 10
(1 row)

postgres=*# table t_log;
              ts               |            msg            
-------------------------------+---------------------------
 2025-02-16 15:08:38.29895+03  | We are ready to insert 10
 2025-02-16 15:08:53.466983+03 | We have inserted 10
(2 rows)

postgres=*# rollback;
ROLLBACK
postgres=# table t;
 i 
---
(0 rows)

postgres=# table t_log;
              ts               |            msg            
-------------------------------+---------------------------
 2025-02-16 15:08:38.29895+03  | We are ready to insert 10
 2025-02-16 15:08:53.466983+03 | We have inserted 10
(2 rows)

postgres=# 
```

# TODO

The extension doesn't support partitioned tables. Insert rows into partitions directly.
