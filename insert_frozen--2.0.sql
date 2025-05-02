-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION insert_frozen" to load this file. \quit

-- Register the C function.
CREATE PROCEDURE insert_frozen(anynonarray)
AS 'MODULE_PATHNAME', 'insert_frozen'
LANGUAGE C;
