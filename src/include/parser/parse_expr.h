/*-------------------------------------------------------------------------
 *
 * parse_expr.h
 *	  handle expressions in parser
 *
 * Portions Copyright (c) 1996-2026, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/parser/parse_expr.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_EXPR_H
#define PARSE_EXPR_H

#include "parser/parse_node.h"

/* GUC parameters */
extern PGDLLIMPORT bool Transform_null_equals;

/*
 * Hook for an extension to give a column reference that names no column, and
 * that no PostParseColumnRefHook resolved, a meaning of its own: an
 * expression, or NULL to have the reference reported as missing.  It is
 * consulted only where the parser would otherwise raise that error.
 */
typedef Node *(*columnref_fallback_hook_type) (ParseState *pstate,
											   ColumnRef *cref);
extern PGDLLIMPORT columnref_fallback_hook_type columnref_fallback_hook;

extern Node *transformExpr(ParseState *pstate, Node *expr, ParseExprKind exprKind);

extern const char *ParseExprKindName(ParseExprKind exprKind);

#endif							/* PARSE_EXPR_H */
