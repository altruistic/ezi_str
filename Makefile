# Main top-level makefile for ezi_str library build system, Linux & Cygwin version.
# Copyright (C) 2004-2022 Alf Lacis
# mail 1: alfredo4570 at gmail dot com, or 2: lacis_alfredo at yahoo dot com
# http://alfredo4570.net
#
# This library is free software; you can redistribute it and/or modify it under the terms of the GNU
# Lesser General Public License as published by the Free Software Foundation; ONLY version 2.1 of
# the License.
#
# This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License along with this library;
# if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA
#---------------------------------------------------------------------------------------------------
# 20080830 alf Original version
#---------------------------------------------------------------------------------------------------

# name of C source files
CSRC= ezi_str.c
COBJ= ezi_str.o

#####
CFLAGS += -DDEBUG -g

#---------------------------------------------------------------------------------------------------
COBJS = $(patsubst %.c,%.o,$(CSRC)) # Also adds "output/" subdirectory name.
#---------------------------------------------------------------------------------------------------

CFLAGS += -Wall -O2 -MM
CFLAGS += -DEZI_DBG_MAX=150 # Debugger only: sets length to display EZI_STR_T* strings.
CFLAGS += -fsigned-char # ALFLB is written with this requirement in general.
CFLAGS += -ffunction-sections # All functions in a module are placed in separate sections.

# Linker flag used with CFLAGS += -ffunction-sections
#LDFLAGS += --gc-sections

CC = gcc

#---------------------------------------------------------------------------------------------------
$(COBJ): $(CSRC) Makefile
	$(CC) $(CFLAGS) $(CSRC) > $(COBJ)

#---------------------------------------------------------------------------------------------------
