/*
Copyright 2011 Aiko Barz

This file is part of torrentkino.

torrentkino is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

torrentkino is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with torrentkino.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <sys/epoll.h>

#include "lookup.h"
#include "hex.h"

LOOKUP *ldb_init( UCHAR *target, IP *from ) {
	LOOKUP *l = (LOOKUP *) myalloc( sizeof(LOOKUP) );

	memcpy( l->target, target, SHA1_SIZE );
	
	if( from == NULL ) {
		l->send_reply = FALSE;
		memset( &l->c_addr, '\0', sizeof( IP ) );
	} else {
		l->send_reply = TRUE;
		memcpy( &l->c_addr, from, sizeof( IP ) );
	}

	l->hash = hash_init( 1000 );
	l->list = list_init();

	return l;
}

void ldb_free( LOOKUP *l ) {
	if( l == NULL ) {
		return;
	}
	hash_free( l->hash );
	list_clear( l->list );
	list_free( l->list );
	myfree( l );
}

ULONG ldb_put( LOOKUP *l, UCHAR *node_id, IP *from ) {
	ITEM *i = NULL;
	LNODE *new = NULL;
	LNODE *n = NULL;
	ULONG index = 0;

	/* Wow. Something is broken or this Kademlia cloud is huge. */
	if( l->size >= 32767 ) {
		info( from, 0, "ldb_put(): Too many nodes without end in sight." );
		ldb_free( l );
		return 32767;
	}

	new = (LNODE *) myalloc( sizeof( LNODE ) );
	memcpy( new->id, node_id, SHA1_SIZE );
	memcpy( &new->c_addr, from, sizeof( IP ) );
	memset( new->token, '\0', TOKEN_SIZE_MAX );
	new->token_size = 0;

	/* Create a sorted list. The first nodes are the best fitting. */
	i = list_start( l->list );
	while( i != NULL ) {
		n = list_value( i );

		/* Look, whose node_id fits better to the target */
		if( ldb_compare( node_id, n->id, l->target ) < 0 ) {
			list_ins( l->list, i, new );
			hash_put( l->hash, new->id, SHA1_SIZE, new );
			return index;
		}

		i = list_next( i );
		index++;
	}

	/* Last resort. Append the node, so that it does not get a query again. */
	list_put( l->list, new );
	hash_put( l->hash, new->id, SHA1_SIZE, new );

	return index;
}

LNODE *ldb_find( LOOKUP *l, UCHAR *node_id ) {
	return hash_get( l->hash, node_id, SHA1_SIZE );
}

void ldb_update( LOOKUP *l, UCHAR *node_id, BEN *token, IP *from ) {
	LNODE *n = NULL;

	if( ( n = hash_get( l->hash, node_id, SHA1_SIZE ) ) == NULL ) {
		return;
	}

	memcpy( &n->c_addr, from, sizeof( IP ) );
	memcpy( &n->token, token->v.s->s, token->v.s->i );
	n->token_size = token->v.s->i;
}

int ldb_compare(UCHAR *id1, UCHAR *id2, UCHAR *target) {
	UCHAR xor1;
	UCHAR xor2;
	int i = 0;

	for( i=0; i<SHA1_SIZE; i++ ) {
		if( id1[i] == id2[i] ) {
			continue;
		}
		xor1 = id1[i] ^ target[i];
		xor2 = id2[i] ^ target[i];
		if( xor1 < xor2 ) {
			return -1;
		} else {
			return 1;
		}
	}

	return 0;
}
