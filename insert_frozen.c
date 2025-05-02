#include "postgres.h"
#include "access/heapam.h"
#include "catalog/pg_type.h"
#include "executor/executor.h"
#include "nodes/execnodes.h"
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

	/* 
	 * Protect the table against concurrent data changes to avoid a new tuple
	 * with the same unique key insertion in the period between index constraints
	 * have already been checked and a new tuple has not yet been inserted.
	 */
	Relation rel = relation_open(tce->typrelid, ShareLock);
	if (unlikely(rel->rd_rel->relkind != 'r'))
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("The argument type must refer to an ordinary table")));

	TupleTableSlot *slot = MakeTupleTableSlot(rel->rd_att, &TTSOpsVirtual);
	ExecStoreHeapTupleDatum(PG_GETARG_DATUM(0), slot);

	EState		 *estate = CreateExecutorState();
	ResultRelInfo resultRelInfo;
	InitResultRelInfo(&resultRelInfo, rel, 0, NULL, 0);
	ExecOpenIndices(&resultRelInfo, true /* for ExecCheckIndexConstraints */);
	if (rel->rd_att->constr)
		ExecConstraints(&resultRelInfo, slot, estate);

	/*
	 * An error on unique constraint violation in ExecInsertIndexTuples cannot
	 * rollback the insertion into a table, because of the TABLE_INSERT_FROZEN
	 * option. So it is necessary to check index constraints before the insertion.
	 */
	ItemPointerData conflictTid;
	if (!ExecCheckIndexConstraints(&resultRelInfo, slot, estate, &conflictTid, NIL))
		ereport(ERROR,
				(errcode(ERRCODE_UNIQUE_VIOLATION),
				 errmsg("duplicate key value violates unique constraint \"%s\"",
						RelationGetRelationName(rel)),
				 errtableconstraint(rel, RelationGetRelationName(rel))));

	table_tuple_insert(rel, slot, GetCurrentCommandId(true), TABLE_INSERT_FROZEN, NULL);

	/* 
	 * We need slot->tts_tid to update indexes. The slot->tts_tid value is set in
	 * table_tuple_insert, so it should be called before ExecInsertIndexTuples.
	 */
	if (rel->rd_rel->relhasindex)
	{
		list_free(ExecInsertIndexTuples(&resultRelInfo, slot, estate, false,
										false, NULL, NIL, false));
	}

	ExecCloseIndices(&resultRelInfo);
	ExecDropSingleTupleTableSlot(slot);
	relation_close(rel, ShareLock);
	FreeExecutorState(estate);

	PG_RETURN_VOID();
}
