# 2017-01-09: Siva Prasad Meduri <siva.meduri@nxp.com>
# Makefile to compile python package logging 

#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Package/python-logging
$(call Package/python/Default)
  TITLE:=Python $(PYTHON_VERSION) logging module
  DEPENDS:=+python-light
endef

$(eval $(call PyBasePackage,python-logging, \
	/usr/lib/python$(PYTHON_VERSION)/logging \
))
