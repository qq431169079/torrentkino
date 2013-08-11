/*
Copyright 2006 Aiko Barz

This file is part of masala/tumbleweed.

masala/tumbleweed is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

masala/tumbleweed is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with masala/tumbleweed.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/epoll.h>
#include <limits.h>

#ifdef TUMBLEWEED
#include "main.h"
#include "conf.h"
#include "str.h"
#include "list.h"
#else
#include "list.h"
#endif

LIST *list_init( void ) {
	LIST *list = (LIST *) myalloc( sizeof(LIST), "list_init" );

	list->item = NULL;
	list->size = 0;

	return list;
}

void list_free( LIST *list ) {
	if( list == NULL ) {
		return;
	}
	
	while( list->item != NULL ) {
		list_del( list, list->item );
	}

	myfree( list, "list_free" );
}

void list_clear( LIST *list ) {
	ITEM *item = NULL;

	if( list == NULL ) {
		return;
	}

	item = list_start( list );
	while( item != NULL ) {
		myfree( item->val, "list_clear" );
		item = list_next( item );
	}
}

ITEM *list_start( LIST *list ) {
	ITEM *item = NULL;

	if( list == NULL ) {
		return NULL;
	}
	
	if( list->item == NULL ) {
		return NULL;
	}

	item = list->item;
	while( item->prev != NULL ) {
		item = list_prev( item );
	}

	if( item != list->item ) {
		list->item = item;
	}

	return item;
}

ITEM *list_stop( LIST *list ) {
	ITEM *item = NULL;

	if( list == NULL ) {
		return NULL;
	}
	
	if( list->item == NULL ) {
		return NULL;
	}

	item = list->item;
	while( item->next != NULL ) {
		item = list_next( item );
	}

	return item;
}

ULONG list_size( LIST *list ) {
	return list->size;
}

ITEM *list_put( LIST *list, void *payload ) {
	ITEM *item = NULL;
	ITEM *stop = NULL;

	if( list == NULL ) {
		return NULL;
	}
	
	if( list->size == ULONG_MAX ) {
		return NULL;
	}

	item = (ITEM *) myalloc( sizeof(ITEM), "list_put" );
	item->val = payload;
	item->next = NULL;
	item->prev = NULL;

	/* First item? */
	if( list->item == NULL ) {
		list->item = item;
		list->size = 1;
		return item;
	}

	stop = list_stop( list );

	item->prev = stop;
	stop->next = item;

	list->size += 1;

	return item;
}

ITEM *list_join( LIST *list, ITEM *here, void *payload ) {
	ITEM *item = NULL;
	ITEM *next = NULL;

	if( list == NULL ) {
		return NULL;
	}
	
	if( list->size == ULONG_MAX ) {
		return NULL;
	}

	/* This insert is like a normal list_put */
	if( list->item == NULL ) {
		return list_put( list, payload );
	}
	
	/* This insert is like a normal list_put */
	if( here->next == NULL ) {
		return list_put( list, payload );
	}

	/* Payload */
	item = (ITEM *) myalloc( sizeof(ITEM), "list_join" );
	item->val = payload;

	/* Pointer */
	next = here->next;

	item->next = next;
	item->prev = here;
	
	here->next = item;
	next->prev = item;

	list->size += 1;

	return item;
}

ITEM *list_del( LIST *list, ITEM *item ) {
	if( list == NULL ) {
		return NULL;
	}
	if( item == NULL ) {
		return NULL;
	}

	if( item->next == NULL && item->prev == NULL ) {
		list->item = NULL;
	} else if( item->next == NULL ) {
		item->prev->next = NULL;
	} else if( item->prev == NULL ) {
		list->item = item->next;
		item->next->prev = NULL;
	} else {
		item->prev->next = item->next;
		item->next->prev = item->prev;
	}

	myfree( item, "list_del" );

	list->size -= 1;

	return list_start( list );
}

ITEM *list_next( ITEM *item ) {
	if( item == NULL ) {
		return NULL;
	}
	return item->next;
}

ITEM *list_prev( ITEM *item ) {
	if( item == NULL ) {
		return NULL;
	}
	return item->prev;
}

void *list_value( ITEM *item ) {
	if( item == NULL ) {
		return NULL;
	}
	return item->val;
}
