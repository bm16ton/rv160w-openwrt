# 2017-07-03: Siva Prasad Meduri <siva.meduri@nxp.com>
# Makefile to compile complete python package
#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

PYTHON_VERSION:=2.7
PYTHON_VERSION_MICRO:=12

PYTHON_DIR:=$(STAGING_DIR)/usr
PYTHON_BIN_DIR:=$(PYTHON_DIR)/bin
PYTHON_INC_DIR:=$(PYTHON_DIR)/include/python$(PYTHON_VERSION)
PYTHON_LIB_DIR:=$(PYTHON_DIR)/lib/python$(PYTHON_VERSION)

PYTHON_PKG_DIR:=/usr/lib/python$(PYTHON_VERSION)/site-packages

PYTHON:=python$(PYTHON_VERSION)

HOST_PYTHON_LIB_DIR:=$(STAGING_DIR_HOST)/lib/python$(PYTHON_VERSION)
HOST_PYTHON_BIN:=$(STAGING_DIR_HOST)/bin/python2

PYTHONPATH:=$(PYTHON_LIB_DIR):$(STAGING_DIR)/$(PYTHON_PKG_DIR):$(PKG_INSTALL_DIR)/$(PYTHON_PKG_DIR)
define HostPython
	(	export PYTHONPATH="$(PYTHONPATH)"; \
		export PYTHONOPTIMIZE=""; \
		export PYTHONDONTWRITEBYTECODE=1; \
		export _python_sysroot="$(STAGING_DIR)"; \
		export _python_prefix="/usr"; \
		export _python_exec_prefix="/usr"; \
		$(1) \
		$(HOST_PYTHON_BIN) $(2); \
	)
endef

# These configure args are needed in detection of path to Python header files
# using autotools.
CONFIGURE_ARGS += \
	_python_sysroot="$(STAGING_DIR)" \
	_python_prefix="/usr" \
	_python_exec_prefix="/usr"

PKG_USE_MIPS16:=0
# This is required in addition to PKG_USE_MIPS16:=0 because otherwise MIPS16
# flags are inherited from the Python base package (via sysconfig module)
ifdef CONFIG_USE_MIPS16
  TARGET_CFLAGS += -mno-mips16 -mno-interlink-mips16
endif

define PyPackage

  # Add default PyPackage filespec none defined
  ifndef PyPackage/$(1)/filespec
    define PyPackage/$(1)/filespec
      +|$(PYTHON_PKG_DIR)
    endef
  endif

  $(call shexport,PyPackage/$(1)/filespec)

  define Package/$(1)/install
	find $(PKG_INSTALL_DIR) -name "*\.pyc" -o -name "*\.pyo" | xargs rm -f
	@echo "$$$$$$$$$$(call shvar,PyPackage/$(1)/filespec)" | ( \
		IFS='|'; \
		while read fop fspec fperm; do \
		  fop=`echo "$$$$$$$$fop" | tr -d ' \t\n'`; \
		  if [ "$$$$$$$$fop" = "+" ]; then \
			if [ ! -e "$(PKG_INSTALL_DIR)$$$$$$$$fspec" ]; then \
			  echo "File not found '$(PKG_INSTALL_DIR)$$$$$$$$fspec'"; \
			  exit 1; \
			fi; \
			dpath=`dirname "$$$$$$$$fspec"`; \
			if [ -n "$$$$$$$$fperm" ]; then \
			  dperm="-m$$$$$$$$fperm"; \
			else \
			  dperm=`stat -c "%a" $(PKG_INSTALL_DIR)$$$$$$$$dpath`; \
			fi; \
			mkdir -p $$$$$$$$$dperm $$(1)$$$$$$$$dpath; \
			echo "copying: '$$$$$$$$fspec'"; \
			cp -fpR $(PKG_INSTALL_DIR)$$$$$$$$fspec $$(1)$$$$$$$$dpath/; \
			if [ -n "$$$$$$$$fperm" ]; then \
			  chmod -R $$$$$$$$fperm $$(1)$$$$$$$$fspec; \
			fi; \
		  elif [ "$$$$$$$$fop" = "-" ]; then \
			echo "removing: '$$$$$$$$fspec'"; \
			rm -fR $$(1)$$$$$$$$fspec; \
		  elif [ "$$$$$$$$fop" = "=" ]; then \
			echo "setting permissions: '$$$$$$$$fperm' on '$$$$$$$$fspec'"; \
			chmod -R $$$$$$$$fperm $$(1)$$$$$$$$fspec; \
		  fi; \
		done; \
	)
	$(call PyPackage/$(1)/install,$$(1))
  endef
endef

# $(1) => build subdir
# $(2) => additional arguments to setup.py
# $(3) => additional variables
define Build/Compile/PyMod
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/$(PYTHON_PKG_DIR)
	$(call HostPython, \
		cd $(PKG_BUILD_DIR)/$(strip $(1)); \
		CC="$(TARGET_CC)" \
		CCSHARED="$(TARGET_CC) $(FPIC)" \
		CXX="$(TARGET_CXX)" \
		LD="$(TARGET_CC)" \
		LDSHARED="$(TARGET_CC) -shared" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -I$(PYTHON_INC_DIR)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lpython$(PYTHON_VERSION)" \
		_PYTHON_HOST_PLATFORM=linux2 \
		__PYVENV_LAUNCHER__="/usr/bin/$(PYTHON)" \
		$(3) \
		, \
		./setup.py $(2) \
	)
	find $(PKG_INSTALL_DIR) -name "*\.pyc" -o -name "*\.pyo" | xargs rm -f
endef

