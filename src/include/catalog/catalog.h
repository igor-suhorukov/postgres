/*-------------------------------------------------------------------------
 *
 * catalog.h
 *	  prototypes for functions in backend/catalog/catalog.c
 *
 *
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/catalog.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef CATALOG_H
#define CATALOG_H

#include "catalog/pg_class.h"
#include "utils/relcache.h"


extern bool IsSystemRelation(Relation relation);
extern bool IsToastRelation(Relation relation);
extern bool IsCatalogRelation(Relation relation);
extern bool IsInplaceUpdateRelation(Relation relation);

extern bool IsSystemClass(Oid relid, Form_pg_class reltuple);
extern bool IsToastClass(Form_pg_class reltuple);

extern bool IsCatalogRelationOid(Oid relid);
extern bool IsCatalogTextUniqueIndexOid(Oid relid);
extern bool IsInplaceUpdateOid(Oid relid);

extern bool IsCatalogNamespace(Oid namespaceId);
extern bool IsToastNamespace(Oid namespaceId);

extern bool IsReservedName(const char *name);

extern bool IsSharedRelation(Oid relationId);

extern bool IsPinnedObject(Oid classId, Oid objectId);

/*
 * Hook for an extension to supply the OID that GetNewOidWithIndex() returns,
 * for example to give an object the same OID on several servers.  Returning
 * InvalidOid leaves the OID to be generated as usual.  An OID the hook
 * supplies is used as is: it is not checked against the relation, and for a
 * relation OID it is not checked against existing files either, so the hook
 * is responsible for handing out OIDs that are unused everywhere it is set.
 */
typedef Oid (*new_oid_hook_type) (Relation relation, Oid indexId,
								  AttrNumber oidcolumn);
extern PGDLLIMPORT new_oid_hook_type new_oid_hook;

extern Oid	GetNewOidWithIndex(Relation relation, Oid indexId,
							   AttrNumber oidcolumn);
extern bool LastNewOidWasPreassigned(void);
extern RelFileNumber GetNewRelFileNumber(Oid reltablespace,
										 Relation pg_class,
										 char relpersistence);

#endif							/* CATALOG_H */
