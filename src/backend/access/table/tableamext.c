/*-------------------------------------------------------------------------
 *
 * tableamext.c
 *	  The registry of what table access methods add to their TableAmRoutine
 *
 * See access/tableamext.h.  An extension registers while the postmaster
 * loads it, so the registry is the same in every backend, and it is looked
 * up by the TableAmRoutine pointer a relation's rd_tableam holds.  It holds a
 * handful of entries at most, so it is a short array searched in order.
 *
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/table/tableamext.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/table.h"
#include "access/tableamext.h"
#include "miscadmin.h"
#include "utils/memutils.h"

int			TableAmExtensionCount = 0;

/* The registered methods, and what each registered, side by side */
static const TableAmRoutine **TableAmExtensionAms = NULL;
static TableAmExtRoutine **TableAmExtensionRoutines = NULL;

void
RegisterTableAmExtension(const TableAmRoutine *am,
						 const TableAmExtRoutine *ext)
{
	TableAmExtRoutine *copy;

	if (!process_shared_preload_libraries_in_progress)
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("a table access method's extension routine can only be registered while \"shared_preload_libraries\" is loaded")));

	if (am == NULL || ext == NULL)
		elog(ERROR, "invalid table access method extension routine");

	if (ext->size < offsetof(TableAmExtRoutine, reloptions) ||
		ext->size > sizeof(TableAmExtRoutine))
		elog(ERROR, "table access method extension routine of %zu bytes, where this server's has %zu",
			 ext->size, sizeof(TableAmExtRoutine));

	if (GetTableAmExtension(am) != NULL)
		elog(ERROR, "a table access method may register one extension routine");

	/*
	 * Keep a copy as big as this server's struct, so that the members an
	 * extension built against an older one does not have read as absent.
	 */
	copy = MemoryContextAllocZero(TopMemoryContext, sizeof(TableAmExtRoutine));
	memcpy(copy, ext, ext->size);
	copy->size = sizeof(TableAmExtRoutine);

	if (TableAmExtensionCount == 0)
	{
		TableAmExtensionAms = MemoryContextAlloc(TopMemoryContext,
												 sizeof(TableAmRoutine *));
		TableAmExtensionRoutines = MemoryContextAlloc(TopMemoryContext,
													  sizeof(TableAmExtRoutine *));
	}
	else
	{
		TableAmExtensionAms = repalloc(TableAmExtensionAms,
									   sizeof(TableAmRoutine *) *
									   (TableAmExtensionCount + 1));
		TableAmExtensionRoutines = repalloc(TableAmExtensionRoutines,
											sizeof(TableAmExtRoutine *) *
											(TableAmExtensionCount + 1));
	}

	TableAmExtensionAms[TableAmExtensionCount] = am;
	TableAmExtensionRoutines[TableAmExtensionCount] = copy;
	TableAmExtensionCount++;
}

const TableAmExtRoutine *
GetTableAmExtension(const TableAmRoutine *am)
{
	for (int i = 0; i < TableAmExtensionCount; i++)
	{
		if (TableAmExtensionAms[i] == am)
			return TableAmExtensionRoutines[i];
	}
	return NULL;
}

void
table_scan_extractcolumns(TableScanDesc scan, PlanState *ps)
{
	const TableAmExtRoutine *ext = GetTableAmExtension(scan->rs_rd->rd_tableam);

	if (ext != NULL && ext->scan_extractcolumns != NULL)
		ext->scan_extractcolumns(scan, ps);
}

bool
table_scans_by_column(Oid relid)
{
	Relation	rel;
	const TableAmExtRoutine *ext;
	bool		result;

	/* The planner holds a lock on every relation it plans a scan of. */
	rel = table_open(relid, NoLock);
	ext = rel->rd_tableam ? GetTableAmExtension(rel->rd_tableam) : NULL;
	result = (ext != NULL && ext->scan_by_column);
	table_close(rel, NoLock);

	return result;
}
