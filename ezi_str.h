/*  Start-Of-File: ezi_str.h
Copyright (c) 2005-2022 Alf Lacis.
mail 1: alfredo4570 at gmail dot com, or 2: lacis_alfredo at yahoo dot com
http://alfredo4570.net

This header as of 20080422 has ezitypes.h and alf_defines.h merged in it: the copyright from those
files is identical with this one.

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

Refer to alflb documentation which contains usage notes for these and other functions.

----------------------------------------------------------------------------------------------------

20220507 abb Extracted ezi_str definitions to this header file.
20180208 abb Renamed max in the ezi strings to max_index, although the public name 'max' is unchanged.
20161120 abb Used stdint types (e.g. uint32_t) instead of ULONG, etc.
20120827 alf Added ezi_overlay() and ezi_overlay_raw().
20120508 alf Added ezi_trail() as part of the ezi_sproc() group.
20120401 alf Added ezi_dup_full() & ezi_dup_part().  Renamed ezi_salloc() to ezi_dup_raw().
20120314 alf Added ezi_reverse() which calls reversen(). Added some separator dashed lines.
20100708 alf Added ezi_trunc().
20090727 alf Changed return from size_t to int for ezi_snprintf() & ezi_vsnprintf() to fit in with
             standard definitions for snprintf() & vsnprintf().
20090718 alf Added ezi_vsnprintf().
20090715 alf Added ezi_cpy_cat() & ezi_cpy_cat_raw().
20080828 alf Added __cplusplus fences; added template <class TYPE> class ezi_index; added ezi_min()
             template family.
20071130 alf Renamed ezi_set() to ezi_cpy_raw(), but added ezi_set() macro for old code. Added
             ezi_cat_raw().
20070722 alf Changed some returns for Ezi-String funtions: ezi_ch(), ezi_set(), ezi_cpy(),
             ezi_cat(), ezi_dbg(), ezi_esc(), ezi_sproc().
20070714 alf Reinstated EZI_STR_EMPTY() with new functionality.
20061221 alf Removed EZI_STR_EMPTY() since it was too easy to abuse.
20061219 alf Added ezi_ltrim() & ezi_rtrim().
20061212 alf Added EZI_STR_BASIC().
20061201 alf Added ezi_calloc, ezi_salloc & ezi_realloc prototypes.
20061108 alf Changed EZI_STR() to set up a pointer.  Added EZI_STR_T for prototypes.
20050517 alf Added EZI_STR(), which declares a simple string structure with an in-built length
             variable. If you've got a better solution, please tell me!
--------------------------------------------------------------------------------------------------*/

#ifndef __ezi_str_h__
#define __ezi_str_h__ 1

#include <stddef.h>     // for size_t
#include <stdarg.h>     // for va_list
#include <stdio.h>      // for FILE*
#include <stdint.h>     // for various types                                              //20160129

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************************
	E Z I    S T R I N G S   :    P R I V A T E   T Y P E :

The outer name "EZI_STR_PRIVATE" is typically not used by applications.

The main elements of the structure are the manipulated items:

	size_t max;              // Maximum index of buffer: effectively a 'const'
	size_t len;              // Current length of string
	char   str[max_index+1]; // The string data itself. See CLARIFICATION below regarding index Vs size.

*/
#define EZI_STR_PRIVATE(name,max_index) \
	struct name ## _struct_tag \
	{ \
		size_t max; \
		size_t len; \
		char   str[max_index+1]; \
	}

/***************************************************************************************************
    E Z I    S T R I N G S    :    P U B L I C    I N T E R F A C E S

EZI_STR(name,max_index,init_str)  Initialised ezi string. The created object is always a pointer.

                 CLARIFICATION:  max_index is the maximum INDEX of the string: so for "Pea",
                                 max_index is 3.  Indexes are "0,1,2,3".
                                 The sizeof() is 4 (including terminating NUL).
                                 For example, to terminate any overflowed string:

                                                 to->str[to->max] = '\0';
                                 EQUIVALENT TO:  to->str[     3 ] = '\0';

                                 So when we set len to max, like this:
                                   to->len = to->max;
                                 we really mean that the length, eg, as reported by strlen(), is 3 bytes.

  EZI_STR_BASIC(name,init_str)     Initialised ezi string where the max length is the initialised
                                 string length, e.g. EZI_STR_BASIC(hi,"hello") sets max_index & len = 5,
                                 and str[] to 5+1.

  EZI_STR_T                       typedef for function prototype parameters.
                                 * DON'T USE FOR INITIALISATIONS: USE ONLY FOR  *
                                 * PROTOTYPES, 'extern' REFERENCES OR POINTERS. *

  EZI_STR_EMPTY(name,max_index)    Shortcut for EZI_STR(name, max_index, "");

  EZI_DBG_MAX 1                  Used to initialise the typedef (but not the actual memory) so
                                 that debugger can report strings longer than the default of 1.

***************************************************************************************************/

// The original name is 'hidden' as 'name_private'.
#define EZI_STR(name,max_index,init_str) \
	EZI_STR_PRIVATE(name,max_index) name ## _private = { max_index, sizeof(init_str)-1, init_str }; \
	EZI_STR_T *name = (EZI_STR_T*)&name ## _private

#define EZI_STR_BASIC(name,init_str) \
	EZI_STR_PRIVATE(name,sizeof(init_str)-1) name ## _private = { sizeof(init_str)-1, sizeof(init_str)-1, init_str }; \
	EZI_STR_T *name = (EZI_STR_T*)&name ## _private

#define EZI_STR_EMPTY(name,max_index) \
	EZI_STR(name,max_index,"")

#ifndef EZI_DBG_MAX
#define EZI_DBG_MAX (1)
#endif

	typedef         EZI_STR_PRIVATE(ezistr, EZI_DBG_MAX) EZI_STR_T; // TYPICALLY USED ONLY FOR PROTOTYPES, 'extern' REFERENCES OR POINTERS.

//--------------------------------------------------------------------------------------------------
//
//   E Z I   S T R I N G S   :   P U B L I C   F U N C T I O N   P R O T O T Y P E S
//
//  Instantiated in ezi_str.c:
	EZI_STR_T      *ezi_clr         (EZI_STR_T *buf);                // Clears an Ezi string to an empty string.
	EZI_STR_T      *ezi_ch          (EZI_STR_T *buf, int ch_integer);// Adds character ch_integer to the end of an Ezi string. Note 'int' character.
	EZI_STR_T      *ezi_cpy         (EZI_STR_T *to, EZI_STR_T *from); // Ezi, and safe, string copy.
	EZI_STR_T      *ezi_cpy_raw     (EZI_STR_T *to, char     *from); // Non-Ezi-string to Ezi-string
	EZI_STR_T      *ezi_cat         (EZI_STR_T *to, EZI_STR_T *from); // Ezi, and safe, string concatenation.
	EZI_STR_T      *ezi_cat_raw     (EZI_STR_T *to, char     *from); // Non-Ezi-string to Ezi-string
	EZI_STR_T      *ezi_cpy_cat     (EZI_STR_T *to, EZI_STR_T *from1, EZI_STR_T *from2); // "Copy cat"
	EZI_STR_T	   *ezi_cpy_cat_raw (EZI_STR_T *to, char 	*from1, char     *from2); // "Copy cat raw"
	EZI_STR_T      *ezi_fill        (EZI_STR_T *buf, int ch_integer); // Fills the whole string with 'ch_integer'.
	size_t          ezi_len         (EZI_STR_T *buf); // Recalculates Ezi string length (especially if string populated by a non-Ezi function).
	EZI_STR_T      *ezi_ltrim       (EZI_STR_T *buf, size_t left_trim);  // Removed requested number of characters from the front of the string.
	EZI_STR_T      *ezi_rtrim       (EZI_STR_T *buf, size_t right_trim); // Removed requested number of characters from the end of the string.
	EZI_STR_T      *ezi_trunc       (EZI_STR_T *buf, size_t new_len);    // Truncates string to new length.
	EZI_STR_T      *ezi_reverse     (EZI_STR_T *buf); // Reverses the string in-place.
	EZI_STR_T      *ezi_trail       (EZI_STR_T *str);
	EZI_STR_T      *ezi_overlay     (EZI_STR_T *str, size_t index, EZI_STR_T *from);
	EZI_STR_T      *ezi_overlay_raw (EZI_STR_T *str, size_t index, char     *from, size_t from_len);

	EZI_STR_T      *ezi_dup_raw     (char *initstr);      // Ezi, and safe, strdup() of a raw string. Obeys strdup rules.
	EZI_STR_T      *ezi_dup_full    (EZI_STR_T *original); // Ezi, and safe, strdup() of a full Ezi-String. Obeys strdup rules.
	EZI_STR_T      *ezi_dup_part    (EZI_STR_T *original); // Ezi, and safe, strdup() of the used part of an Ezi-String. Obeys strdup rules.
#define             ezi_salloc       ezi_dup_raw          // Deprecated.
	EZI_STR_T      *ezi_calloc      (size_t capacity);    // Ezi string calloc: obeys calloc rules
	EZI_STR_T      *ezi_realloc     (EZI_STR_T *old_ptr, size_t capacity); // Ezi string realloc: obeys realloc rules

	int             ezi_snprintf    (EZI_STR_T *ezi, const char *format, ... ); // Instantiated in ezi_snprintf.c
	int             ezi_vsnprintf   (EZI_STR_T *ezi, const char *format, va_list args); // Instantiated in ezi_snprintf.c


// deprecated
#define             ezi_set(to,from)  ezi_cpy_raw(to, from) // ezi_set() is deprecated: use ezi_cpy_raw(to, from)

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // #ifndef __ezi_str_h__
//  End-Of-File: ezi_str.h
