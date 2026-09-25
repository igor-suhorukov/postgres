/*-------------------------------------------------------------------------
 *
 * tablespace.h
 *		Tablespace management commands (create/drop tablespace).
 *
 *
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/commands/tablespace.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TABLESPACE_H
#define TABLESPACE_H

#include "access/xlogreader.h"
#include "catalog/objectaddress.h"
#include "lib/stringinfo.h"
#include "nodes/parsenodes.h"

extern PGDLLIMPORT char *default_tablespace;
extern PGDLLIMPORT char *temp_tablespaces;
extern PGDLLIMPORT bool allow_in_place_tablespaces;

/*
 * Hook for an extension that puts this server's directory of a tablespace
 * elsewhere than the location CREATE TABLESPACE gave -- a directory of its
 * own under the location, say, where servers share a machine and would
 * otherwise share the directory.  It is given the location, which the WAL
 * record carries too, and returns the directory pg_tblspc links to: one it
 * made, or one whose absence is then reported as the location's would be.
 * It is asked where the directories are made, by CREATE TABLESPACE and by
 * its redo, and not for an in-place tablespace, which is the server's own.
 */
typedef const char *(*tablespace_location_hook_type) (const char *location,
													  Oid tablespaceoid);
extern PGDLLIMPORT tablespace_location_hook_type tablespace_location_hook;

/*
 * Hook for the same extension, as the link pg_tblspc/<oid> is removed: by
 * DROP TABLESPACE, and by its redo, once the tablespace's own directories
 * are gone.  It is given the link, which it may read, so that it may remove
 * the directory the link points to if it is one tablespace_location_hook
 * made, now empty.  "redo" is as destroy_tablespace_directories() has it:
 * where it is true, a problem is to be reported as a LOG, not an ERROR.
 * It is not asked for an in-place tablespace, whose directory is the link.
 */
typedef void (*tablespace_location_drop_hook_type) (const char *linkloc,
													 Oid tablespaceoid,
													 bool redo);
extern PGDLLIMPORT tablespace_location_drop_hook_type tablespace_location_drop_hook;

/* XLOG stuff */
#define XLOG_TBLSPC_CREATE		0x00
#define XLOG_TBLSPC_DROP		0x10

typedef struct xl_tblspc_create_rec
{
	Oid			ts_id;
	char		ts_path[FLEXIBLE_ARRAY_MEMBER]; /* null-terminated string */
} xl_tblspc_create_rec;

typedef struct xl_tblspc_drop_rec
{
	Oid			ts_id;
} xl_tblspc_drop_rec;

typedef struct TableSpaceOpts
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	float8		random_page_cost;
	float8		seq_page_cost;
	int			effective_io_concurrency;
	int			maintenance_io_concurrency;
} TableSpaceOpts;

extern Oid	CreateTableSpace(CreateTableSpaceStmt *stmt);
extern void DropTableSpace(DropTableSpaceStmt *stmt);
extern ObjectAddress RenameTableSpace(const char *oldname, const char *newname);
extern Oid	AlterTableSpaceOptions(AlterTableSpaceOptionsStmt *stmt);

extern void TablespaceCreateDbspace(Oid spcOid, Oid dbOid, bool isRedo);

extern Oid	GetDefaultTablespace(char relpersistence, bool partitioned);

extern void PrepareTempTablespaces(void);

extern Oid	get_tablespace_oid(const char *tablespacename, bool missing_ok);
extern char *get_tablespace_name(Oid spc_oid);

extern bool directory_is_empty(const char *path);
extern void remove_tablespace_symlink(const char *linkloc);

extern void tblspc_redo(XLogReaderState *record);
extern void tblspc_desc(StringInfo buf, XLogReaderState *record);
extern const char *tblspc_identify(uint8 info);

#endif							/* TABLESPACE_H */
