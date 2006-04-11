/*
 * Generic single linked list to keep various information Copyright (C)
 * 1993, 1994 Free Software Foundation, Inc.
 *
 * Author: Kresten Krab Thorup
 *
 * This file is part of GNU CC.
 *
 * GNU CC is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * GNU CC is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with GNU CC; see the file COPYING.  If not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * As a special exception, if you link this library with files compiled
 * with GCC to produce an executable, this does not cause the resulting
 * executable to be covered by the GNU General Public License. This
 * exception does not however invalidate any other reasons why the
 * executable file might be covered by the GNU General Public License.
 */

#ifndef __LIST_H_
#define __LIST_H_

#include "config.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <stdlib.h>


typedef struct LinkedList {
    void *head;
    struct LinkedList *tail;
} LinkedList;


/*
 * Return a cons cell produced from (head . tail)
 */

static inline LinkedList*
list_cons(void* head, LinkedList* tail)
{
    LinkedList *cell;

    cell = (LinkedList *) malloc(sizeof(LinkedList));
    cell->head = head;
    cell->tail = tail;
    return cell;
}

/*
 * Return the length of a list, list_length(NULL) returns zero
 */

static inline int
list_length(LinkedList* list)
{
    int i = 0;
    while (list) {
	i += 1;
	list = list->tail;
    }
    return i;
}

/*
 * Remove the element at the head by replacing it by its successor
 */

static inline void
list_remove_head(LinkedList** list)
{
    if (!*list)
	return;
    if ((*list)->tail) {
	LinkedList *tail = (*list)->tail;	/* fetch next */
	*(*list) = *tail;	/* copy next to list head */
	free(tail);		/* free next */
    } else {			/* only one element in list */

	free(*list);
	(*list) = 0;
    }
}

/*
 * Free list (backwards recursive)
 */

static inline void
list_free(LinkedList* list)
{
    if (list) {
	list_free(list->tail);
	free(list);
    }
}

#endif
