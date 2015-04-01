##===- lib/Transforms/Makefile -----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../..
PARALLEL_DIRS = Ceda Ceda_New CFCSS CFCSS64 CFCSS_MOD CFCSS_XOR CFCSS_XOR_MULT CFCSS_XOR_SBL CFCSS_XOR_SBL64 CFCSS_XOR64 GetFunctionsName IncreaseSize GetBasicBlocksNames CFCSS_NEW_XOR64 CFCSS_NEW_XOR_SBL64

include $(LEVEL)/Makefile.config

# No support for plugins on windows targets
ifeq ($(HOST_OS), $(filter $(HOST_OS), Cygwin MingW Minix))
  PARALLEL_DIRS := $(filter-out Hello, $(PARALLEL_DIRS))
endif

include $(LEVEL)/Makefile.common
