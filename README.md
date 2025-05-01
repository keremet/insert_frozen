Some DBMS support autonomous transactions to create log records, but PostgreSQL does not. This PostgreSQL extension can be used to create log records in faster way. The extension contains the `insert_frozen` function to insert frozed rows into a table.

# BUILD

```
#Add path to pg_config
export PATH=/home/keremet/compile/postgresql_bin/bin:$PATH
make
make install
```

# CREATE EXTENSION

```
create extension insert_frozen;
```

# EXAMPLE

Frozen rows remain when the transaction is rolled back.

```
postgres=# create table t_log (ts timestamp with time zone, msg text);
CREATE TABLE
postgres=# create table t (i int);
CREATE TABLE
postgres=# begin;
BEGIN
postgres=*# select insert_frozen((clock_timestamp(), 'We are ready to insert 10')::t_log);
 insert_frozen 
---------------
 
(1 row)

postgres=*# insert into t values (10);
INSERT 0 1
postgres=*# select insert_frozen((clock_timestamp(), 'We have inserted 10')::t_log);
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
 2025-05-01 20:47:45.22164+03  | We are ready to insert 10
 2025-05-01 20:48:11.149597+03 | We have inserted 10
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
 2025-05-01 20:47:45.22164+03  | We are ready to insert 10
 2025-05-01 20:48:11.149597+03 | We have inserted 10
(2 rows)

postgres=# 
```

# LIMITATIONS

The extension doesn't support partitioned tables. Insert rows into partitions directly.
The extension supports heap tables only. Heap is the default table access method.
