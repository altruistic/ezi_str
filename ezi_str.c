/* Start-Of-File: ezi_str.c
'Ezi' String routines using the EZI_STR structures (c) 1994-2022 Alf Lacis
mail 1: alfredo4570 at gmail dot com, or 2: lacis_alfredo at yahoo dot com
http://alfredo4570.net

This library is free software; you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation; either version 2.1 of
the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this library;
if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA
----------------------------------------------------------------------------------------------------

Rules regarding malloc():
=========================
   If the space cannot be allocated, a null pointer should be returned. If the size of the space
   requested is 0, the behavior is implementation-defined: the value returned should be either a
   null pointer or a unique pointer.

Rules regarding realloc():
==========================
   If the new size of the memory object would would require movement of the object, the space for
   the previous instantiation of the object should be freed. If the new size is larger, the
   contents of the newly allocated portion of the object are unspecified. If size is 0 and ptr is
   not a null pointer, the object pointed to should be freed. If the space cannot be allocated, the
   object should remain unchanged.

   If ptr does not match a pointer returned earlier by ezi_calloc() or if the space has previously
   been deallocated by a call to free() or ezi_realloc(), the behavior is undefined.

To compile & run the test programs:
===================================
$ rm ezi_str.exe
$ gcc -Wall -o ezi_str d_string.c convescs.c strncpyxx.c sproc.c ezi_alloc.c -DEZI_TEST_APP ezi_str.c
$ ./ezi_str

Or to check that a warning is generated for an over-initialised string:
$ gcc -Wall -o ezi_str d_string.c convescs.c strncpyxx.c sproc.c ezi_alloc.c -DEZI_TEST_APP -DEZI_TEST_APP_OVERFLOW ezi_str.c

$ rm ezi_alloc.exe
$ gcc -Wall -o ezi_alloc strncpyxx.c -DEZI_ALLOC_TEST_APP ezi_str.c
$ ./ezi_alloc


20220401 abb Combined ezi_snprintf.c and ezi_alloc.c into this file.
20180208 abb Small changes following the review of 'max' and 'max_index'.
20120827 alf Added ezi_overlay() & ezi_overlay_raw().
20120507 alf Added ezi_trail().
20120401 alf Added ezi_dup_full() & ezi_dup_part().  Renamed ezi_salloc() to ezi_dup_raw().
20120313 alf Added ezi_reverse() which calls reversen().
20120309 alf Changed url to http://alfredo4570.net
20100708 alf Added ezi_trunc().
20090727 alf Changed return from size_t to int for ezi_snprintf() & ezi_vsnprintf() to fit in with
             standard definitions for snprintf() & vsnprintf().
20090718 alf Added ezi_vsnprintf().
20090715 alf Added ezi_cpy_cat() & ezi_cpy_cat_raw().
20080422 alf Removed old header files, built and ran test application.
20080408 alf Added (char) cast in ezi_ch() to placate HEW.
20080105 alf Moved 'from IS NULL' test to outside of the MEMORY_CONFIDENCE fence, since this seems
             like desirable behaviour. Removed need for is_not.h.
20071130 alf Renamed ezi_set() to ezi_cpy_raw(), but added ezi_set() macro for old code. Added
             ezi_cat_raw(). Modded test app.
20071021 alf Renamed reverse() to reversen().
20070721 alf Changed protos for some functions from returning char* to EZI_STR_T*.
20061219 alf Added ezi_ltrim() & ezi_rtrim().
20061201 alf Created from ezi_str.c, bitmapfs.c & others.
20061117 alf Added ezi_snprintf().
20061114 alf Added #ifndef MEMORY_CONFIDENCE fence.
20061108 alf Split ezi_dbg() & ezi_esc() to separate files, since these call the large
             libraries d_string_safe() & ConvertEscapeSequencesSafe(), and may not be
             required, especially in an embedded environment.
20061105 alf Added #ifndef MEMORY_CONFIDENCE fence.
20061105 alf Created from strncpyn.c, d_string.c & others.
20050831 alf No code change: clarified documentation & added Example 2. Changed the
             variable name 'max' to 'last_index' (not regarded as a code change) to help
             me remember the functioning of this variable.
20050523 alf Modified 'reverse()' to return char* (was void).
20041025 alf Mod'd 3rd parameter for strncpyn() from int to size_t.
20040421 alf Modified for longs, ulongs & radix.  Note argument placement!
20010130 alf Fixed include path for "is_not.h".
???????? alf Created strncpyn from "#define STRNCPYNULL(a,b,c) {strncpy(a,b,c);a[c]='\0';}" from OT_DATA.H.
*/

#include <stdlib.h>        // for malloc(), etc
#include <string.h>        // for strlen()
#include <stddef.h>
#include <stdio.h>         // for vsnprintf()
#include <stdarg.h>        // for va_xxxxx()

#include "ezi_str.h"       // for strncpyn(), EZI_STR_T & prototypes, etc

#define USING_MEMCPY 1

#ifndef MIN
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

/***************************************************************************************************************
 *
 *    @  @  @@@@  @     @@@   @@@@  @@@         @@@@  @  @  @  @   @@   @@@@@  @@@   @@   @  @   @@
 *    @  @  @     @     @  @  @     @  @        @     @  @  @  @  @  @    @     @   @  @  @  @  @  @
 *    @  @  @     @     @  @  @     @  @        @     @  @  @@ @  @       @     @   @  @  @@ @  @
 *    @@@@  @@@   @     @@@   @@@   @@@@        @@@   @  @  @ @@  @       @     @   @  @  @ @@   @@
 *    @  @  @     @     @     @     @@          @     @  @  @  @  @       @     @   @  @  @  @     @
 *    @  @  @     @     @     @     @ @         @     @  @  @  @  @  @    @     @   @  @  @  @  @  @
 *    @  @  @@@@  @@@@  @     @@@@  @  @        @      @@   @  @   @@     @    @@@   @@   @  @   @@
 *
 ***************************************************************************************************************
	These helper functions may also be used outside of EZI_STR type functions.
	->	strncpysz(), strncpyn(), reversen()
*/

char *strncpysz(char dest[], char from[], size_t sizeof_dest)
{
	volatile size_t last_null_pos = sizeof_dest - 1; // 2019-05-12

#ifndef MEMORY_CONFIDENCE
	if ( dest == NULL )
		return NULL;
#endif

	if ( from == NULL )
		dest[0] = '\0';
	else
		strncpy(dest, from, last_null_pos);

	dest[last_null_pos] = '\0';

	return dest;
}

char *strncpyn(char dest[], char from[], size_t last_index)
{
	return strncpysz(dest, from, last_index + 1);
}

char *reversen(char string[], size_t size)
{
	size_t i, j;
	char c;

	for ( i = 0, j = size - 1; i < j; i++, j-- )
	{
		c         = string[i];
		string[i] = string[j];
		string[j] = c;
	}

	return string;
}

/********************************************************************************************************************
 *
 *    @@@@  @@@@  @@@        @@   @@@@@  @@@         @@@@  @  @  @  @   @@   @@@@@  @@@   @@   @  @   @@
 *    @        @   @        @  @    @    @  @        @     @  @  @  @  @  @    @     @   @  @  @  @  @  @
 *    @       @    @        @       @    @  @        @     @  @  @@ @  @       @     @   @  @  @@ @  @
 *    @@@   @@@@   @         @@     @    @@@@        @@@   @  @  @ @@  @       @     @   @  @  @ @@   @@
 *    @      @     @           @    @    @@          @     @  @  @  @  @       @     @   @  @  @  @     @
 *    @     @      @        @  @    @    @ @         @     @  @  @  @  @  @    @     @   @  @  @  @  @  @
 *    @@@@  @@@@  @@@  @@@   @@     @    @  @        @      @@   @  @   @@     @    @@@   @@   @  @   @@
 *
 ********************************************************************************************************************/

EZI_STR_T *ezi_cat(EZI_STR_T *to, EZI_STR_T *from)
{
	// strncpyn does the MEMORY_CONFIDENCE stuff.
	// 'safe' strncpy(), adds a trailing '\0' if original string is too long.
	strncpyn(&to->str[to->len], from->str, to->max - to->len);

#ifndef MEMORY_CONFIDENCE
	if ( to != NULL )
#endif
		to->len = MIN(to->max, to->len + from->len);     // Assumes that 'from' is correctly set up.

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_cat_raw(EZI_STR_T *to, char *from)
{
	// strncpyn does the MEMORY_CONFIDENCE stuff.
	// 'safe' strncpy(), adds a trailing '\0' if original string is too long.
	strncpyn(&to->str[to->len], from, to->max - to->len);

#ifndef MEMORY_CONFIDENCE
	if ( to != NULL )
#endif
		ezi_len(to);

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_ch(EZI_STR_T *to, int ch_integer)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	if ( to->len >= to->max )
	{
		to->len = to->max;               // If too big, truncate the string.
	}                                    // Remember, len is # of bytes, max is the maximum index
	else
	{
		to->str[to->len] = (char)ch_integer;
		to->len++;
	}
	to->str[to->len] = '\0';
	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_clr(EZI_STR_T *to)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	to->str[0] = '\0';
	to->len = 0ul;
	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_cpy(EZI_STR_T *to, EZI_STR_T *from)
{
	// strncpyn does the MEMORY_CONFIDENCE stuff.
	// 'safe' strncpy(), adds a trailing '\0' if original string is too long.
	strncpyn(to->str, from->str, to->max);

#ifndef MEMORY_CONFIDENCE
	if ( to != NULL )
#endif
		to->len = MIN(to->max, from->len);     // Assumes that 'from' is correctly set up.

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_cpy_raw(EZI_STR_T *to, char *from) // non-Ezi-string to Ezi-string
{
	// strncpyn does the MEMORY_CONFIDENCE stuff.
	// 'safe' strncpy(), adds a trailing '\0' if original string is too long.
	strncpyn(to->str, from, to->max);

#ifndef MEMORY_CONFIDENCE
	if ( to != NULL )
#endif
		ezi_len(to);

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_cpy_cat(EZI_STR_T *to, EZI_STR_T *from1, EZI_STR_T *from2)
{
	return ezi_cat(ezi_cpy(to, from1), from2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_cpy_cat_raw(EZI_STR_T *to, char *from1, char *from2)
{
	return ezi_cat_raw(ezi_cpy_raw(to, from1), from2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Fills the whole string with 'ch_integer'.
EZI_STR_T *ezi_fill(EZI_STR_T *to, int ch_integer)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	memset(to->str, ch_integer, to->max);
	to->len = to->max;        // Adjust the length.
	to->str[to->max] = '\0';  // Ensure string is truncated.

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
size_t ezi_len(EZI_STR_T *to)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return 0ul;
#endif

	to->len = strlen(to->str);
	if ( to->len >= to->max )            // 20180208 was '>'
	{
		to->len = to->max;               // If too long, truncate the string.
		to->str[to->max] = '\0';
	}

	return to->len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Removed requested number of characters from the front of the string.
EZI_STR_T *ezi_ltrim(EZI_STR_T *to, size_t left_trim)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	if ( left_trim >= to->len )  // trim more characters than in string? Just clear the string.
	{
		ezi_clr(to);
		return to;;
	}

	if ( left_trim > 0ul )  // only do a trim if actually requested
	{
		// +1 includes trailing '\0'
		memcpy(to->str, &to->str[left_trim], to->len + 1 - left_trim);
//		memmove(to->str, &to->str[left_trim], to->len + 1 - left_trim); // Overlap-safe, but slower.
		to->len -= left_trim;        // Adjust the length.
	}

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Removed requested number of characters from the end of the string.
EZI_STR_T *ezi_rtrim(EZI_STR_T *to, size_t right_trim)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	if ( right_trim >= to->len )  // trim more characters than in string? Just clear the string.
	{
		ezi_clr(to);
		return to;
	}

	if ( right_trim > 0ul )  // only do a trim if actually requested
	{
		to->str[to->len - right_trim] = '\0'; // Trim the string.
		to->len -= right_trim;        // Adjust the length.
	}

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Overlay part of the 'to' string starting at 'index' with the 'from' string.
// If index is outside the current length of 'to', function returns 'to' unchanged.
EZI_STR_T *ezi_overlay(EZI_STR_T *to, size_t index, EZI_STR_T* from)
{
	return ezi_overlay_raw(to, index, from->str, from->len);
}

EZI_STR_T *ezi_overlay_raw(EZI_STR_T *to, size_t index, char *from, size_t from_len)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	if ( index > to->len )
	{
		return to;
	}

	if ( (index + from_len) > to->max )  // only do a partial overlay of 'to's capacity exceeded.
	{
		memcpy(&to->str[index], from, to->max - to->len);
	}
	else
	{
		memcpy(&to->str[index], from, from_len);
	}
	ezi_len(to); // lazy way out

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Similar to ezi_rtrim(), but specify new length instead of number of characters to remove.
EZI_STR_T *ezi_trunc(EZI_STR_T *to, size_t new_len)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	if ( new_len < to->len )  // Only truncate string if new length is shorter than current length.
	{
		to->str[new_len] = '\0';  // Truncate the string.
		to->len = new_len;        // Adjust the length.
	}

	return to;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
EZI_STR_T *ezi_trail(EZI_STR_T *to)
{
#ifndef MEMORY_CONFIDENCE
	if ( to == NULL )
		return NULL;
#endif

	size_t new_len = to->len;

	while ( new_len > 0
	        && ( to->str[new_len - 1] == ' '
	          || to->str[new_len - 1] == '\t' ) )
		new_len--;

	to->len = new_len;
	to->str[new_len] = '\0';

	return to;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
/* reverses string s in place. K.R. p 62.  As an Ezi String fuction, does not need to use strlen(). */
EZI_STR_T *ezi_reverse(EZI_STR_T *s)
{
	reversen(s->str, s->len);
	return s;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// HIDDEN function which does the bulk of the allocation & initialisation of the new Ezi string.
static EZI_STR_T *ezi_alloc_private(size_t mem_size, size_t capacity, char *initstr)
{
	EZI_STR_T *ptr  = NULL;

	if ( capacity > 0ul )
	{
		if ( (ptr = malloc(mem_size)) != NULL )
		{
			ptr->max = capacity;
			ezi_cpy_raw(ptr, initstr);  // Copies string & adjusts 'len'.
		}
	}
	return ptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* Ezi, and safe, strdup-style call.  Obeys strdup rules.  This creates a full-sized Ezi-String
copy of the original Ezi-String.  I.e., if the original capacity is 20 bytes, and 5 bytes are used,
then the duplicate will be 20 bytes in capacity, with 5 bytes used.

RETURN VALUE
Upon successful completion with size not equal to 0, ezi_dup_full() returns a pointer to the
allocated space. If size is 0, either a null pointer or a unique pointer that can be successfully
passed to free() should be returned. Otherwise, it returns a null pointer and should set errno to
indicate the error. */

EZI_STR_T *ezi_dup_full(EZI_STR_T *original)
{
	size_t    capacity = original->max;
	size_t    mem_size = capacity + sizeof(EZI_STR_PRIVATE(ezistr, 1));

	return ezi_alloc_private(mem_size, capacity, original->str);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* Ezi, and safe, strdup-style call.  Obeys strdup rules.  This creates a part-sized Ezi-String
copy of the original Ezi-String.  I.e., if the original capacity is 20 bytes, and 5 bytes are used,
then the duplicate will be 5 bytes in capacity, with 5 bytes used.

RETURN VALUE
Upon successful completion with size not equal to 0, ezi_dup_part() returns a pointer to the
allocated space. If size is 0, either a null pointer or a unique pointer that can be successfully
passed to free() should be returned. Otherwise, it returns a null pointer and should set errno to
indicate the error. */

EZI_STR_T *ezi_dup_part(EZI_STR_T *original)
{
	size_t    capacity = original->len;
	size_t    mem_size = capacity + sizeof(EZI_STR_PRIVATE(ezistr, 1));

	return ezi_alloc_private(mem_size, capacity, original->str);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* The ezi_dup_raw() allocates space for an EZI_STR_T object whose size in bytes is specified by the
length of the raw string, *and* *copies* the raw string.

See "Rules regarding malloc()" above.

RETURN VALUE
Upon successful completion with size not equal to 0, ezi_dup_raw() returns a pointer to the
allocated space. If size is 0, either a null pointer or a unique pointer that can be successfully
passed to free() should be returned. Otherwise, it returns a null pointer and should set errno to
indicate the error. */

EZI_STR_T *ezi_dup_raw(char *initstr)
{
	size_t    capacity = strlen(initstr);
	size_t    mem_size = capacity + sizeof(EZI_STR_PRIVATE(ezistr, 1));

	return ezi_alloc_private(mem_size, capacity, initstr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* The ezi_calloc() (Ezi Clear Alloc) function allocates unused space for an EZI_STR_T object whose
size in bytes is specified by size, *and* *clears" the string to "" and the length to 0.

See "Rules regarding malloc()" above.

RETURN VALUE
Upon successful completion with size not equal to 0, ezi_calloc() returns a pointer to the
allocated space. If size is 0, either a null pointer or a unique pointer that can be successfully
passed to free() should be returned. Otherwise, it returns a null pointer and should set errno to
indicate the error. */

EZI_STR_T *ezi_calloc(size_t capacity)
{
	size_t    mem_size = capacity + sizeof(EZI_STR_PRIVATE(ezistr, 1));

	return ezi_alloc_private(mem_size, capacity, "");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/* The ezi_realloc() function changes the size of the EZI_STR_T object pointed to by ptr to the size
specified by size. If changed to a smaller string, and the old string was bigger, the string is
truncated and '\0'-terminated.

If ptr is a null pointer, ezi_realloc() is equivalent to ezi_calloc() for the specified size.

See "Rules regarding realloc()" above.

RETURN VALUE
Upon successful completion with a size not equal to 0, ezi_realloc() returns a pointer to the
(possibly moved) allocated space. If size is 0, either a null pointer or a unique pointer that can
be successfully passed to free() should be returned. If there is not enough available memory,
ezi_realloc() should return a null pointer and set errno to [ENOMEM]. */

EZI_STR_T *ezi_realloc(EZI_STR_T *old_ptr, size_t capacity)
{
	EZI_STR_T *new_ptr  = old_ptr;
	size_t    mem_size = capacity + sizeof(EZI_STR_PRIVATE(ezistr, 1));

	if ( old_ptr == NULL )
	{
		new_ptr = ezi_calloc(capacity);
	}
	else if ( capacity == 0ul )
	{
		free(old_ptr);
		new_ptr = NULL;
	}
	else
	{
		if ( (new_ptr = realloc(old_ptr, mem_size)) != NULL )
		{
			new_ptr->max = capacity;       // can be bigger or smaller
			if ( new_ptr->len > new_ptr->max )  // if smaller than the current string length, truncate the string.
			{
				new_ptr->len = new_ptr->max;
				new_ptr->str[new_ptr->len] = '\0';
			}
		}
		else
			new_ptr = NULL;
	}
	return new_ptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
/*EZI_STR_T *ezi_EG_helper(EZI_STR_T *p, char *raw_str)
{
	if ( p == NULL )
	{
		p = ezi_dup_raw(raw_str);
	}

	return p;
}*/

//////////////////////////////////////////////////////////////////////////////////////////
int ezi_snprintf ( EZI_STR_T *ezi, const char *fmt, ... )
{
	va_list  args;
	int      possible_overflow;

	//printf("ezi_snprintf:len,max,str=\"%lu,%lu,%s\"\n", ezi->len, ezi->max, ezi->str); fflush(stdout);

	va_start ( args, fmt );
	possible_overflow = ezi_vsnprintf ( ezi, fmt, args );
	va_end ( args );

	//printf("ezi_snprintf:len,max,str=\"%lu,%lu,%s\"\n", ezi->len, ezi->max, ezi->str); fflush(stdout);
	//printf("possible_overflow:=\"%ld\"\n", possible_overflow); fflush(stdout);

	return possible_overflow;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Use this where the va_list pointer has already been set up.
int ezi_vsnprintf ( EZI_STR_T *ezi, const char *fmt, va_list args )
{
	int possible_overflow;

	possible_overflow = vsnprintf ( ezi->str, ezi->max + 1, fmt, args );

	if ( possible_overflow < 0 || possible_overflow >= (int)ezi->max ) // If overflowed...
	{
		ezi_len ( ezi );               // ... recalculate current length
	}
	else
	{
		ezi->len = ( size_t ) possible_overflow;  // Not overflowed: length is OK.
	}

	return possible_overflow;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
#ifdef EZI_ALLOC_TEST_APP

#include "alflb.h"

void p(EZI_STR_T *ezi, char *text)
{
	printf("%p:%3u:%3u:%-20.20s %s\n", ezi, (uint32_t)(ezi ? ezi->max : 0), (uint32_t)(ezi ? ezi->len : 0), ezi ? ezi->str : "<null>", text);
	fflush(stdout);
}

int main(int argc, char *argv[])
{
	EZI_STR_T *a;
	EZI_STR_T *b;
	EZI_STR_T *c;
	EZI_STR_T *d;
	EZI_STR_T *e;
	a = ezi_calloc(10);              p(a, "a = ezi_calloc(10);"              );
	ezi_cpy_raw(a, "testing");       p(a, "    ezi_cpy_raw(a, \"testing\");" );
	b = ezi_dup_raw("hello world");  p(b, "b = ezi_dup_raw(\"hello world\");");
	b = ezi_realloc(b, 500);         p(b, "b = ezi_realloc(b, 500);"         );
	b = ezi_realloc(b, 11);          p(b, "b = ezi_realloc(b, 11);"          );
	b = ezi_realloc(b, 3);           p(b, "b = ezi_realloc(b, 3);"           );
	c = ezi_calloc(0);               p(c, "c = ezi_calloc(0);"               );
	c = ezi_realloc(c, 30);          p(c, "c = ezi_realloc(c, 30);"          );
	ezi_cpy_raw(c, "hello");         p(c, "    ezi_cpy_raw(c,\"hello\");"    );
	c = ezi_realloc(c, 60);          p(c, "c = ezi_realloc(c, 60);"          );
	c = ezi_realloc(c, 0);           p(c, "c = ezi_realloc(c, 0);"           );
	d = ezi_dup_full(a);             p(d, "d = ezi_dup_full(a);"             );
	e = ezi_dup_part(a);             p(e, "e = ezi_dup_part(a);"             );

	return 0;
}
#endif // EZI_ALLOC_TEST_APP

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef TEST_APP_EZI_SNPRINTF

#include "alflb.h"

int main ( int argc, char *argv[] )
{
	EZI_STR ( medium, 20, "" );
	EZI_STR ( small , 10, "" );

	ezi_snprintf ( medium, "%s%d%s", "hello", 20, "world" );
	printf ( "%2lu:%s\n", medium->len, medium->str );

	ezi_snprintf ( small, "%s%d%s", "hello", 20, "world" );
	printf ( "%2lu:%s\n", small->len, small->str );

	return 0;
}
#endif // TEST_APP_EZI_SNPRINTF

////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef EZI_TEST_APP

#include "alflb.h"

int main(int argc, char *argv[])
{
	int       jj;
#define       P printf
#define       TMAX 20
	EZI_STR   (s1, TMAX,    "");
	EZI_STR   (s2, TMAX,    ".123456789.12345678");
	EZI_STR   (s3, TMAX,    ".123456789.123456789");
	EZI_STR   (s4, TMAX + 1,  ".123456789.123456789.");
	EZI_STR   (non, TMAX,    ".\177\37\377456789.123456789");
	char      c1[] = ".123456789.12345678";
	char      c2[] = ".123456789.123456789.12345678";
#ifdef EZI_TEST_APP_OVERFLOW     // should give at least a warning (would prefer an error)
	EZI_STR   (over  , TMAX,    ".123456789.123456789.123456789.123456789");
#endif
	EZI_STR_T *p1;
	EZI_STR_T *p2;

	ezi_cpy(s1, s2);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s2)"  , (uint)s1->len, s1->str);
	ezi_cpy(s1, s3);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s3)"  , (uint)s1->len, s1->str);
	ezi_cpy(s1, s4);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s4)"  , (uint)s1->len, s1->str);
	ezi_len(s1);			P("%24.24s=len=%2u \"%s\"\n", "ezi_len(recalc)"   , (uint)s1->len, s1->str); // recalc length
	s1->str[15] = '\0';
	ezi_len(s1);			P("%24.24s=len=%2u \"%s\"\n", "ezi_len(recalc)"   , (uint)s1->len, s1->str); // recalc length
	for ( jj = 0; jj < 6; jj++ )
	{
	ezi_ch(s1, 'A' + jj);	P("%24.24s=len=%2u \"%s\"\n", "ezi_ch"            , (uint)s1->len, s1->str);
	}
	ezi_cpy(s1, s2);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s2)"  , (uint)s1->len, s1->str);
	ezi_cat(s1, s2);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cat(from s2)"  , (uint)s1->len, s1->str);
	ezi_len(s1);			P("%24.24s=len=%2u \"%s\"\n", "ezi_len"           , (uint)s1->len, s1->str);
	ezi_clr(s1);			P("%24.24s=len=%2u \"%s\"\n", "ezi_clr"           , (uint)s1->len, s1->str);
	ezi_dbg(s1, non);		P("%24.24s=len=%2u \"%s\"\n", "ezi_dbg(from non)" , (uint)s1->len, s1->str); // stops early
	ezi_esc(non, s1);		P("%24.24s=len=%2u \"%s\"\n", "ezi_esc(from s1)"  , (uint)non->len, non->str);
	ezi_sproc(non, SP_8BIT_NONP | SP_ONE_BLANK, ' ');
							P("%24.24s=len=%2u \"%s\"\n", "ezi_sproc(non)"    , (uint)non->len, non->str);
#ifdef EZI_TEST_APP_OVERFLOW
	ezi_cpy(s1, over  );	P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from over)", (uint)s1->len, s1->str);
#endif
	ezi_cpy(s1, s3);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s3)"  , (uint)s1->len, s1->str);
	ezi_ltrim(s1, 2);		P("%24.24s=len=%2u \"%s\"\n", "ezi_ltrim(s1, 2)"  , (uint)s1->len, s1->str);
	ezi_rtrim(s1, 2);		P("%24.24s=len=%2u \"%s\"\n", "ezi_rtrim(s1, 2)"  , (uint)s1->len, s1->str);
	ezi_ltrim(s1, 999);		P("%24.24s=len=%2u \"%s\"\n", "ezi_ltrim(s1,999)" , (uint)s1->len, s1->str);
	ezi_cpy(s1, s3);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s3)"  , (uint)s1->len, s1->str);
	ezi_rtrim(s1, 999);		P("%24.24s=len=%2u \"%s\"\n", "ezi_rtrim(s1,999)" , (uint)s1->len, s1->str);
	ezi_cpy(s1, s3);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s3)"  , (uint)s1->len, s1->str);
	ezi_ltrim(s1, 20);		P("%24.24s=len=%2u \"%s\"\n", "ezi_ltrim(s1,20)"  , (uint)s1->len, s1->str);
	ezi_cpy(s1, s3);		P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy(from s3)"  , (uint)s1->len, s1->str);
	ezi_rtrim(s1, 20);		P("%24.24s=len=%2u \"%s\"\n", "ezi_rtrim(s1,20)"  , (uint)s1->len, s1->str);
	ezi_cpy_raw(s1, c1);	P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy_raw(from c1)"  , (uint)s1->len, s1->str);
	ezi_cpy_raw(s1, c2);	P("%24.24s=len=%2u \"%s\"\n", "ezi_cpy_raw(from c2)"  , (uint)s1->len, s1->str);
	ezi_clr(s1);
	ezi_cat_raw(s1, c1);	P("%24.24s=len=%2u \"%s\"\n", "ezi_cat_raw(from c1)"  , (uint)s1->len, s1->str);
	ezi_cat_raw(s1, c2);	P("%24.24s=len=%2u \"%s\"\n", "ezi_cat_raw(from c2)"  , (uint)s1->len, s1->str);
	p1 = ezi_dup_raw(c1);	P("%24.24s=max=%2u len=%2u \"%s\"\n", "p1=ezi_dup_raw(c1)", (uint)p1->max, (uint)p1->len, p1->str );
	p2 = ezi_calloc(20);	P("%24.24s=max=%2u len=%2u \"%s\"\n", "p2=ezi_calloc(20)", (uint)p2->max, (uint)p2->len, p2->str );
	ezi_cpy(p1, s2);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_cpy(p1,s2)"   , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_cpy(p2, s2);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_cpy(p2,s2)"   , (uint)p2->max, (uint)p2->len, p2->str );
p1 = ezi_realloc(p1, 30);	P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_realloc(p1,30)", (uint)p1->max, (uint)p1->len, p1->str );
	ezi_cpy(p1, s4);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_cpy(p1,s4)"   , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_cat(p1, s2);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_cat(p1,s2)"   , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_cpy(p1, s4);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_cpy(p1,s4)"   , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_reverse(p1);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_reverse(p1)"  , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_trunc(p1, 16);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_trunc(p1,16)" , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_ltrim(p1, 5);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_ltrim(p1,5)"  , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_rtrim(p1, 5);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_rtrim(p1,5)"  , (uint)p1->max, (uint)p1->len, p1->str );
	ezi_reverse(p1);		P("%24.24s=max=%2u len=%2u \"%s\"\n", "ezi_reverse(p1)"  , (uint)p1->max, (uint)p1->len, p1->str );

	NULLIFY(p1);
	NULLIFY(p2);
	return 0;
}
#endif // EZI_TEST_APP

// End-Of-File
