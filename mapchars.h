/*
 *	mapchars.h
 *
 * Structure definitions for mapchars.c
 *
 * There are two structure types defined: MAPCHAR and MAPNODE. The latter
 * is used by itself only from the top-level character index table.
 * Otherwise, it is always used in conjunction with the MAPCHAR structure.
 *
 * The structures are linked together in a tree. The tree corresponds to
 * the different keystroke combinations that are (within the limits of
 * the timeoutlen mode-value) considered to be a single keycode.
 *
 * At a given level of expansion of the tree, we look for alternative
 * completions via a linked list (rather than an index table).  The
 * entries in the linked list are sorted by (unsigned) character value.
 *
 * $Header: /usr/build/VCS/pgf-vile/RCS/mapchars.h,v 1.2 1994/07/11 22:56:20 pgf Exp $
 *
 */

#define	MAP_NEXT        0
#define	MAP_STRING      1
#define	MAP_COMMAND     2
#define MAP_BUFFER      3

#define	MAPCHAR	struct	_mapchar

typedef	struct	{
	union	{
		MAPCHAR	*m_next;	/* MAP_NEXT - another node */
		TBUFF	*m_string;	/* MAP_STRING */
		CMDFUNC	*m_command;	/* MAP_COMMAND */
		BUFFER	*m_buffer;	/* MAP_BUFFER */
		}	n_data;		/* contains data indicated by type */
	UCHAR		n_type;		/* e.g., MAP_NODE */
	} MAPNODE;
#define	n_next    n_data.m_next
#define n_string  n_data.m_string
#define n_command n_data.m_command
#define n_buffer  n_data.m_buffer

	MAPCHAR {
	MAPCHAR *	c_list;
	MAPNODE		c_node;
	UCHAR		c_key;		/* actual key, e.g., '!' */
	};
