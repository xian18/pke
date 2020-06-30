# See LICENSE.Batten for license details.
#
#=========================================================================
# Toplevel Makefile for the Modular C++ Build System
#=========================================================================
# Please read the documenation in 'mcppbs-doc.txt' for more details on
# how the Modular C++ Build System works. For most projects, a developer
# will not need to make any changes to this makefile. The key targets
# are as follows:
#
#  - default   : build all libraries and programs
#  - clean     : remove all generated content (except autoconf files)


#-------------------------------------------------------------------------
# Basic setup
#-------------------------------------------------------------------------

# Remove all default implicit rules since they can cause subtle bugs
# and they just make things run slower
.SUFFIXES:
% : %,v
% : RCS/%,v
% : RCS/%
% : s.%
% : SCCS/s.%

# Default is to build the prereqs of the all target (defined at bottom)
default : all
.PHONY : default

project_name := riscv-pk
src_dir      := .
obj_dir      := obj
# scripts_dir  := $(src_dir)/scripts

# If the version information is not in the configure script, then we
# assume that we are in a working directory. We use the vcs-version.sh
# script in the scripts directory to generate an appropriate version
# string. Currently the way things are setup we have to run this script
# everytime we run make so the script needs to be as fast as possible.

# ifeq (?,?)
#   project_ver:=$(shell $(scripts_dir)/vcs-version.sh $(src_dir))
# else
#   project_ver:=?
# endif

# If --with-arch is not specified, it defaults to whatever the compiler's
# default is. The -with-abi is not necessary for this project. Unconditionally
# compile it with a no-float ABI. i.e., ilp32 for 32-bit and lp64 for 64-bit.

ifneq (,)
  march := -march=
  is_32bit := $(findstring 32,$(march))
  mabi := -mabi=$(if $(is_32bit),ilp32,lp64)
endif


#-------------------------------------------------------------------------
# List of subprojects
#-------------------------------------------------------------------------

sprojs         :=  pk machine util
sprojs_enabled :=  pk machine util

sprojs_include := -I. $(addprefix -I$(src_dir)/, $(sprojs_enabled))
VPATH := $(addprefix $(src_dir)/, $(sprojs_enabled))

#-------------------------------------------------------------------------
# Programs and flags 
#-------------------------------------------------------------------------

# C++ compiler
#  - CPPFLAGS : flags for the preprocessor (eg. -I,-D)
#  - CXXFLAGS : flags for C++ compiler (eg. -Wall,-g,-O3)

CC            := riscv64-unknown-elf-gcc
READELF       := riscv64-unknown-elf-readelf
OBJCOPY       := riscv64-unknown-elf-objcopy
CFLAGS        := -Wall -Werror -D__NO_INLINE__ -mcmodel=medany -O2 -std=gnu99 -Wno-unused -Wno-attributes -fno-delete-null-pointer-checks -fno-PIE $(CFLAGS) $(march) $(mabi) -DBBL_PAYLOAD=\"bbl_payload\" -DBBL_LOGO_FILE=\"bbl_logo_file\"
BBL_PAYLOAD   := 
COMPILE       := $(CC) -MMD -MP $(CFLAGS) \
                 $(sprojs_include)
# Linker
#  - LDFLAGS : Flags for the linker (eg. -L)
#  - LIBS    : Library flags (eg. -l)

LD            := $(CC)
LDFLAGS       :=  -Wl,--build-id=none -nostartfiles -nostdlib -static $(LDFLAGS) $(march) $(mabi)
LIBS          := -lgcc -L $(obj_dir)/ 
LINK          := $(LD) $(LDFLAGS)

# Library creation

AR            := riscv64-unknown-elf-ar
RANLIB        := riscv64-unknown-elf-ranlib

# Host simulator

RUN           := @RUN@
RUNFLAGS      := @RUNFLAGS@



#-------------------------------------------------------------------------
# Default
#-------------------------------------------------------------------------

all : mkdir_obj $(obj_dir)/pke
.PHONY : all

mkdir_obj :
	@test -d $(obj_dir) || mkdir $(obj_dir)


pk_objs := file.o syscall.o handlers.o frontend.o elf.o console.o mmap.o
pk_asm_objs := entry.o
obj_pk_objs := $(addprefix $(obj_dir)/, $(pk_objs))
obj_pk_asm_objs := $(addprefix $(obj_dir)/, $(pk_asm_objs))

machine_objs := fdt.o mtrap.o minit.o htif.o emulation.o muldiv_emulation.o  uart.o uart16550.o finisher.o misaligned_ldst.o flush_icache.o
machine_asm_objs := mentry.o 
obj_machine_objs := $(addprefix $(obj_dir)/, $(machine_objs))
obj_machine_asm_objs := $(addprefix $(obj_dir)/, $(machine_asm_objs))

util_objs := snprintf.o string.o
util_asm_objs := 
obj_util_objs := $(addprefix $(obj_dir)/, $(util_objs))
obj_util_asm_objs := $(addprefix $(obj_dir)/, $(util_asm_objs))

link_libs = -lpk -lmachine -lutil

$(obj_dir)/pke : $(obj_dir)/pk.o $(obj_dir)/libpk.a $(obj_dir)/libmachine.a $(obj_dir)/libutil.a 
	$(LINK) -o $@ $<  $(LIBS) $(link_libs) -T $(src_dir)/pke.lds

#pk
$(obj_dir)/libpk.a: $(obj_pk_objs) $(obj_pk_asm_objs)
	$(AR) rcv -o $@ $^
	$(RANLIB) $@

$(obj_pk_objs): $(obj_dir)/%.o : %.c
	$(COMPILE) -c $< -o $@

$(obj_pk_asm_objs): $(obj_dir)/%.o : %.S
	$(COMPILE) -c $< -o $@

#machine
$(obj_dir)/libmachine.a: $(obj_machine_objs) $(obj_machine_asm_objs)
	$(AR) rcv -o $@ $^
	$(RANLIB) $@

$(obj_machine_objs): $(obj_dir)/%.o : %.c
	$(COMPILE) -c $< -o $@

$(obj_machine_asm_objs): $(obj_dir)/%.o : %.S
	$(COMPILE) -c $< -o $@

#util
$(obj_dir)/libutil.a: $(obj_util_objs)
	$(AR) rcv -o $@ $^
	$(RANLIB) $@

$(obj_util_objs): $(obj_dir)/%.o : %.c
	$(COMPILE) -c $< -o $@

#pk.o
$(obj_dir)/pk.o : pk.c
	$(COMPILE) -c $< -o $@

clean :
	rm -rf $(obj_dir)

