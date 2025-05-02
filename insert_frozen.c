#include "postgres.h"
#include "access/heapam.h"
#include "catalog/pg_type.h"
#include "utils/typcache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(insert_frozen);

Datum
insert_frozen(PG_FUNCTION_ARGS)
{
	Oid				row_type = get_fn_expr_argtype(fcinfo->flinfo, 0);
	TypeCacheEntry *tce = lookup_type_cache(row_type, 0);
	if (tce->typtype != TYPTYPE_COMPOSITE)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("The argument type must be composite"),
				errhint("Example: select %s((1, 't')::tableName);", __func__)));

	Relation		rel = relation_open(tce->typrelid, RowExclusiveLock);
	if (rel->rd_rel->relkind != 'r')
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("The argument type must refer to an ordinary table")));

	HeapTupleHeader	row = DatumGetHeapTupleHeader(PG_GETARG_DATUM(0));
	HeapTupleData 	tup = { .t_len	= HeapTupleHeaderGetDatumLength(row),
							.t_data	= row };
	heap_insert(rel, &tup, GetCurrentCommandId(true), HEAP_INSERT_FROZEN, NULL);

	relation_close(rel, RowExclusiveLock);

	PG_RETURN_VOID();
}
