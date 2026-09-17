/*-------------------------------------------------------------------------
 *
 * combocid.h
 *	  Combo command ID support routines
 *
 *
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/combocid.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef COMBOCID_H
#define COMBOCID_H

/*
 * HeapTupleHeaderGetCmin and HeapTupleHeaderGetCmax function prototypes
 * are in access/htup.h, because that's where the macro definitions that
 * those functions replaced used to be.
 */

/*
 * Hooks for an extension that runs one transaction in several backends, so
 * that a backend which did not create a combo command ID can still resolve
 * one (see XactAdoptCurrentXids()).
 *
 * combocid_create_hook is called after this backend added a combo command ID
 * to its own table, so that the extension can publish the new entry.  Note
 * that it is also called while a parallel worker rebuilds its leader's table.
 *
 * combocid_miss_hook is called for a combo command ID this backend's table
 * does not have.  It returns true after storing the real command IDs in *cmin
 * and *cmax, or false if the combo command ID is none of its business, in
 * which case the lookup fails as it would without the hook.
 */
typedef void (*combocid_create_hook_type) (CommandId combocid,
										   CommandId cmin, CommandId cmax);
extern PGDLLIMPORT combocid_create_hook_type combocid_create_hook;

typedef bool (*combocid_miss_hook_type) (CommandId combocid,
										 CommandId *cmin, CommandId *cmax);
extern PGDLLIMPORT combocid_miss_hook_type combocid_miss_hook;

extern void AtEOXact_ComboCid(void);
extern void RestoreComboCIDState(char *comboCIDstate);
extern void SerializeComboCIDState(Size maxsize, char *start_address);
extern Size EstimateComboCIDStateSpace(void);

#endif							/* COMBOCID_H */
