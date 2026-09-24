/*-------------------------------------------------------------------------
 *
 * parse_relation.h
 *	  prototypes for parse_relation.c.
 *
 *
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/parser/parse_relation.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_RELATION_H
#define PARSE_RELATION_H

#include "parser/parse_node.h"
#include "storage/lockdefs.h"


extern ParseNamespaceItem *refnameNamespaceItem(ParseState *pstate,
												const char *schemaname,
												const char *refname,
												int location,
												int *sublevels_up);
extern CommonTableExpr *scanNameSpaceForCTE(ParseState *pstate,
											const char *refname,
											Index *ctelevelsup);
extern bool scanNameSpaceForENR(ParseState *pstate, const char *refname);
extern void checkNameSpaceConflicts(ParseState *pstate, List *namespace1,
									List *namespace2);
extern ParseNamespaceItem *GetNSItemByRangeTablePosn(ParseState *pstate,
													 int varno,
													 int sublevels_up);
extern ParseNamespaceItem *GetNSItemByVar(ParseState *pstate, Var *var);
extern RangeTblEntry *GetRTEByRangeTablePosn(ParseState *pstate,
											 int varno,
											 int sublevels_up);
extern CommonTableExpr *GetCTEForRTE(ParseState *pstate, RangeTblEntry *rte,
									 int rtelevelsup);
extern Node *scanNSItemForColumn(ParseState *pstate, ParseNamespaceItem *nsitem,
								 int sublevels_up, const char *colname,
								 int location);
extern Node *colNameToVar(ParseState *pstate, const char *colname, bool localonly,
						  int location);
extern void markNullableIfNeeded(ParseState *pstate, Var *var);
extern void markVarForSelectPriv(ParseState *pstate, Var *var);
extern Relation parserOpenTable(ParseState *pstate, const RangeVar *relation,
								LOCKMODE lockmode);

/*
 * Hook for an extension to choose the lock a query takes on a relation it
 * reads or writes, where PostgreSQL chooses it: as the parser opens a
 * relation the query names -- its target, or one in FROM -- and as the
 * rewriter brings in the relations of a view or a rule, among them the table
 * an automatically updatable view writes.  It is given the relation, the
 * mode PostgreSQL would take, and the permissions the query requires on the
 * relation, which tell a plain INSERT from an UPDATE, a DELETE or an
 * INSERT ... ON CONFLICT DO UPDATE; a relation in FROM requires ACL_SELECT,
 * and its mode is RowShareLock when a locking clause applies to it.  It
 * returns the mode to take, which for a relation the query writes is never
 * weaker than PostgreSQL's, and the relation's range table entry records it,
 * so that a cached plan takes it again when it is executed.
 *
 * A stronger mode is for an extension that serializes writers with one,
 * taken first rather than after PostgreSQL's weaker mode, which two
 * transactions that each held it would deadlock upgrading.  A weaker mode,
 * for a relation the query only reads, is for an extension that decides the
 * relation's lock only once the query is planned, and takes it then,
 * recording it in the plan: AccessShareLock conflicts with no mode but
 * AccessExclusiveLock, so no lock taken after it is an upgrade two such
 * transactions deadlock on.  The parser looks a relation it opens by name up
 * without a lock to ask about it, so the relation it opens is the one the
 * name resolves to once it is locked -- the same one, unless DDL renamed or
 * replaced one meanwhile.
 */
typedef LOCKMODE (*query_lockmode_hook_type) (Oid relid, LOCKMODE lockmode,
											  AclMode requiredPerms);
extern PGDLLIMPORT query_lockmode_hook_type query_lockmode_hook;

extern LOCKMODE queryLockMode(Oid relid, LOCKMODE lockmode,
							  AclMode requiredPerms);
extern LOCKMODE parserQueryLockMode(ParseState *pstate,
									const RangeVar *relation,
									LOCKMODE lockmode, AclMode requiredPerms);

extern ParseNamespaceItem *addRangeTableEntry(ParseState *pstate,
											  RangeVar *relation,
											  Alias *alias,
											  bool inh,
											  bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForRelation(ParseState *pstate,
														 Relation rel,
														 LOCKMODE lockmode,
														 Alias *alias,
														 bool inh,
														 bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForSubquery(ParseState *pstate,
														 Query *subquery,
														 Alias *alias,
														 bool lateral,
														 bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForFunction(ParseState *pstate,
														 List *funcnames,
														 List *funcexprs,
														 List *coldeflists,
														 RangeFunction *rangefunc,
														 bool lateral,
														 bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForValues(ParseState *pstate,
													   List *exprs,
													   List *coltypes,
													   List *coltypmods,
													   List *colcollations,
													   Alias *alias,
													   bool lateral,
													   bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForTableFunc(ParseState *pstate,
														  TableFunc *tf,
														  Alias *alias,
														  bool lateral,
														  bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForJoin(ParseState *pstate,
													 List *colnames,
													 ParseNamespaceColumn *nscolumns,
													 JoinType jointype,
													 int nummergedcols,
													 List *aliasvars,
													 List *leftcols,
													 List *rightcols,
													 Alias *join_using_alias,
													 Alias *alias,
													 bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForCTE(ParseState *pstate,
													CommonTableExpr *cte,
													Index levelsup,
													RangeVar *rv,
													bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForENR(ParseState *pstate,
													RangeVar *rv,
													bool inFromCl);
extern ParseNamespaceItem *addRangeTableEntryForGroup(ParseState *pstate,
													  List *groupClauses);
extern RTEPermissionInfo *addRTEPermissionInfo(List **rteperminfos,
											   RangeTblEntry *rte);
extern RTEPermissionInfo *getRTEPermissionInfo(List *rteperminfos,
											   RangeTblEntry *rte);
extern bool isLockedRefname(ParseState *pstate, const char *refname);
extern void addNSItemToQuery(ParseState *pstate, ParseNamespaceItem *nsitem,
							 bool addToJoinList,
							 bool addToRelNameSpace, bool addToVarNameSpace);
pg_noreturn extern void errorMissingRTE(ParseState *pstate, RangeVar *relation);
pg_noreturn extern void errorMissingColumn(ParseState *pstate,
										   const char *relname, const char *colname, int location);

/*
 * Hook for an extension to keep columns of a relation it manages out of "*"
 * expansion, for example counters it maintains in a materialized view of its
 * own.  It is called once per expansion of a relation's columns in a target
 * list, which is where "*" and "rel.*" are expanded, and returns the
 * attribute numbers to leave out, or NULL to leave out none; the set has to
 * stay valid until the expansion is finished.
 *
 * The columns still exist: they can be selected by name, and a whole-row
 * reference still has them, because its value has the relation's composite
 * type and every field of that type belongs to it.
 */
typedef Bitmapset *(*star_expansion_filter_hook_type) (Oid relid);
extern PGDLLIMPORT star_expansion_filter_hook_type star_expansion_filter_hook;

extern void expandRTE(RangeTblEntry *rte, int rtindex, int sublevels_up,
					  VarReturningType returning_type,
					  int location, bool include_dropped,
					  List **colnames, List **colvars);
extern List *expandNSItemVars(ParseState *pstate, ParseNamespaceItem *nsitem,
							  int sublevels_up, int location,
							  List **colnames);
extern List *expandNSItemAttrs(ParseState *pstate, ParseNamespaceItem *nsitem,
							   int sublevels_up, bool require_col_privs,
							   int location);
extern int	attnameAttNum(Relation rd, const char *attname, bool sysColOK);
extern const NameData *attnumAttName(Relation rd, int attid);
extern Oid	attnumTypeId(Relation rd, int attid);
extern Oid	attnumCollationId(Relation rd, int attid);

#endif							/* PARSE_RELATION_H */
