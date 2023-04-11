BUILD

```
#Add path to pg_config
export PATH=/home/keremet/compile/postgresql_bin/bin:$PATH
make
sudo make install
```

The first argument of the insert_frozen() function is table name, the other arguments must have the same types and order as the table attributes. Fill free to change arguments of insert_frozen() in insert_frozen--0.1.sql to make them corresponding with the table which the function inserts in.

RUN

```
postgres=# create extension insert_frozen;
CREATE EXTENSION
postgres=# create table t (i int);
CREATE TABLE
postgres=# create table t_log(ts timestamp with time zone, msg text);
CREATE TABLE
postgres=# begin;
BEGIN
postgres=*# select insert_frozen('t_log', clock_timestamp(), 'We are ready to insert 10');
 insert_frozen 
---------------
 
(1 строка)

postgres=*# insert into t values (10);
INSERT 0 1
postgres=*# select insert_frozen('t_log', clock_timestamp(), 'We have inserted 10');
 insert_frozen 
---------------
 
(1 строка)

postgres=*# table t;
 i  
----
 10
(1 строка)

postgres=*# table t_log;
              ts               |            msg            
-------------------------------+---------------------------
 2023-04-11 21:22:38.733022+03 | We are ready to insert 10
 2023-04-11 21:22:44.212416+03 | We have inserted 10
(2 строки)

postgres=*# rollback;
ROLLBACK
postgres=# table t;
 i 
---
(0 строк)

postgres=# table t_log;
              ts               |            msg            
-------------------------------+---------------------------
 2023-04-11 21:22:38.733022+03 | We are ready to insert 10
 2023-04-11 21:22:44.212416+03 | We have inserted 10
(2 строки)

postgres=# 
```
