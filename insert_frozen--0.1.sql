-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION insert_frozen" to load this file. \quit

-- Register the C function.
CREATE FUNCTION insert_frozen(regclass, timestamp with time zone, text)
RETURNS void
AS 'MODULE_PATHNAME', 'insert_frozen'
LANGUAGE C STRICT PARALLEL SAFE;
