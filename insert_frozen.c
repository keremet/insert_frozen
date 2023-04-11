#include "postgres.h"
#include "access/heapam.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(insert_frozen);

Datum
insert_frozen(PG_FUNCTION_ARGS)
{
	HeapTuple	tup;
	Datum	   *values;
	bool	   *isnull;
	Relation	rel = relation_open(PG_GETARG_OID(0), RowExclusiveLock);

	if (PG_NARGS() != 1 + rel->rd_att->natts)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("invalid argument number")));

	values = (Datum *)palloc(rel->rd_att->natts * sizeof(*values));
	isnull = (bool *)palloc(rel->rd_att->natts * sizeof(*isnull));
	for (int i = 0; i < rel->rd_att->natts; i++)
	{
		values[i] = PG_GETARG_DATUM(i + 1);
		isnull[i] = PG_ARGISNULL(i + 1);
	}

	tup = heap_form_tuple(rel->rd_att, values, isnull);
	heap_insert(rel, tup, GetCurrentCommandId(true), HEAP_INSERT_FROZEN, NULL);
	heap_freetuple(tup);

	relation_close(rel, RowExclusiveLock);
	
	pfree(isnull);
	pfree(values);
	
	PG_RETURN_VOID();
}
