/*
Copyright 2012 Aiko Barz

This file is part of masala.

masala is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

masala is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with masala.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <netinet/in.h>
#include <polarssl/sha1.h>

#include "sha1.h"

void sha1_hash( UCHAR *hash, const char *buffer, long int bytes ) {
	memset( hash, '\0', SHA_DIGEST_LENGTH );
	sha1( (const UCHAR *)buffer, bytes, hash );
}
