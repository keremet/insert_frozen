#include "postgres.h"
#include "access/heapam.h"
#include "catalog/pg_type.h"
#include "utils/typcache.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(insert_frozen);

Datum
insert_frozen(PG_FUNCTION_ARGS)
{
	Oid row_type = get_fn_expr_argtype(fcinfo->flinfo, 0);
	if (unlikely(InvalidOid == row_type))
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Error getting the argument type"),
				errhint("Is the argument passed?")));

	TypeCacheEntry *tce = lookup_type_cache(row_type, 0);
	if (unlikely(tce->typtype != TYPTYPE_COMPOSITE))
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("The argument type must be composite"),
				errhint("Example: call %s((1, 't')::tableName);", __func__)));

	Relation rel = relation_open(tce->typrelid, RowExclusiveLock);
	if (unlikely(rel->rd_rel->relkind != 'r'))
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("The argument type must refer to an ordinary table")));

	TupleTableSlot *slot = MakeTupleTableSlot(rel->rd_att, &TTSOpsVirtual);
	ExecStoreHeapTupleDatum(PG_GETARG_DATUM(0), slot);
	table_tuple_insert(rel, slot, GetCurrentCommandId(true), TABLE_INSERT_FROZEN, NULL);
	ExecDropSingleTupleTableSlot(slot);

	relation_close(rel, RowExclusiveLock);

	PG_RETURN_VOID();
}
