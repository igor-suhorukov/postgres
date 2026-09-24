/*-------------------------------------------------------------------------
 *
 * tableamext.h
 *	  What a table access method may add to its TableAmRoutine, registered
 *	  by the extension that provides it
 *
 * A table access method whose storage is not heap's -- columns kept apart,
 * rows appended in blocks of their own -- needs a few things of the core
 * that TableAmRoutine does not say: that it parses its tables' options, that
 * it wants to know the columns a scan reads, that it can answer a unique
 * index's probe without a fetch.  The extension that provides such a method
 * registers a TableAmExtRoutine for it while the postmaster loads it, and
 * the places that would do something else for such a table ask the registry.
 *
 * Every such place tests TableAmExtensionCount first.  It is zero until an
 * extension registers, so on a server where none has, each of them takes
 * the path it always took.
 *
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/access/tableamext.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TABLEAMEXT_H
#define TABLEAMEXT_H

#include "access/tableam.h"

/* forward references in this file */
typedef struct PlanState PlanState;

/*
 * A run of block numbers a table's row IDs use.  A method whose row IDs
 * leave gaps -- a block number that is a file number in its high bits --
 * gives BRIN the runs it has rows in, so that it walks those and not the
 * gaps between them.
 */
typedef struct TableAmBlockSequence
{
	BlockNumber startblknum;
	BlockNumber nblocks;
} TableAmBlockSequence;

typedef struct TableAmExtRoutine
{
	/*
	 * sizeof(TableAmExtRoutine) as the extension was built, so that a member
	 * added to this struct later reads as absent (NULL, false) for an
	 * extension built before it.
	 */
	Size		size;

	/*
	 * Parse and, when validate is true, check a table's options, in place of
	 * heap_reloptions().  What it returns begins with a StdRdOptions, filled
	 * from heap's option names, which the core reads for every table; the
	 * method's own options follow it.  relkind is RELKIND_RELATION or
	 * RELKIND_MATVIEW.
	 */
	bytea	   *(*reloptions) (Datum reloptions, char relkind, bool validate);

	/*
	 * The plan node a sequential or bitmap scan of the table was begun for,
	 * given once the scan has begun and before its first tuple is asked for,
	 * so that the method reads only the columns that node's target list and
	 * quals use.  A scan begun for anything else is never given one, and
	 * reads every column.
	 */
	void		(*scan_extractcolumns) (TableScanDesc scan, PlanState *ps);

	/*
	 * The method reads a table by column, so the planner gives its scans the
	 * columns they need rather than a physical target list of every column.
	 */
	bool		scan_by_column;

	/*
	 * Is there a row version at tid that a unique index's check must count,
	 * as table_index_fetch_tuple_check() asks, answered without the slot and
	 * the fetch the core makes for each probe.
	 */
	bool		(*index_unique_check) (Relation rel, ItemPointer tid,
									   Snapshot snapshot, bool *all_dead);

	/*
	 * ALTER TABLE ... ADD COLUMN whose values cannot be kept as a missing
	 * value: write the values of the new columns, attnums[i] computed by
	 * exprs[i] from each row, in place of rewriting the table.  exprs are
	 * planned; one whose generated[i] is true is a stored generated column,
	 * computed once the others have their values, as ATRewriteTable() does.
	 */
	void		(*relation_add_columns) (Relation rel, int ncolumns,
										 const AttrNumber *attnums,
										 Expr *const *exprs,
										 const bool *generated);

	/*
	 * The runs of block numbers the table's row IDs use, in order, palloc'd,
	 * their number in *nseqs.
	 */
	TableAmBlockSequence *(*relation_get_block_sequences) (Relation rel,
														   int *nseqs);

} TableAmExtRoutine;

/* How many methods have registered; zero on a server where none has. */
extern PGDLLIMPORT int TableAmExtensionCount;

/*
 * Register ext for the method whose handler returns am.  Only while the
 * postmaster loads shared libraries, so that every backend has the same
 * registry.  The registry keeps a copy.
 */
extern void RegisterTableAmExtension(const TableAmRoutine *am,
									 const TableAmExtRoutine *ext);

/* What was registered for am, or NULL. */
extern const TableAmExtRoutine *GetTableAmExtension(const TableAmRoutine *am);

/*
 * Give the method of a sequential or bitmap scan just begun the plan node
 * it serves, if its extension asked to know (scan_extractcolumns).
 */
extern void table_scan_extractcolumns(TableScanDesc scan, PlanState *ps);

/* Does the method of the table relid read by column (scan_by_column)? */
extern bool table_scans_by_column(Oid relid);

#endif							/* TABLEAMEXT_H */
