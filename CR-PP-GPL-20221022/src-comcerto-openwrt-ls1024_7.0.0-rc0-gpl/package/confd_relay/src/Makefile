######################################################################
#
#  A minimal 'Makefile', by Alan DeKok <aland@freeradius.org>
#
# $Id: Makefile,v 1.13 2007/03/26 04:22:11 fcusack Exp $
#
#############################################################################
VERSION=1.0

######################################################################
#
# If we're really paranoid, use these flags
#CFLAGS = -Wall -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Waggregate-return
#
#  If you're not using GCC, then you'll have to change the CFLAGS.
#
CFLAGS += -Wall
#
# On Irix, use this with MIPSPRo C Compiler, and don't forget to export CC=cc
# gcc on Irix does not work yet for pam_radius
# Also, use gmake instead of make
# Then copy pam_radius_auth.so to /usr/freeware/lib32/security (PAM dir)
# CFLAGS =

#LDFLAGS += -shared -Wl,--version-script=pamsymbols.ver
LDFLAGS += -shared
USER_AUTH = user_auth.o
NOTIFIER = notifier
N_SRC  = notifier.c
######################################################################
#
#
#  The default rule to build everything.
#
all: userauth $(NOTIFIER)

######################################################################
#
#  Build the object file from the C source.
#

#
# This is what should work on Irix:
#pam_radius_auth.so: pam_radius_auth.o md5.o
#	ld -shared pam_radius_auth.o md5.o -L/usr/freeware/lib32 -lpam -lc -o pam_radius_auth.so


######################################################################
#
#  Build the shared library.
#
#  The -Bshareable flag *should* work on *most* operating systems.
#
#  On Solaris, you might try using '-G', instead.
#
#  On systems with a newer GCC, you will need to do:
#
#	gcc -shared pam_radius_auth.o md5.o -lpam -lc -o pam_radius_auth.so
#

userauth: $(USER_AUTH)
	$(CC) $(CFLAGS) -o $@ $(USER_AUTH) -L$(STAGING_DIR)/lib -lconfd -lpthread

$(NOTIFIER):
	$(CC) $(CFLAGS) $(N_SRC) -o $@ -lpthread -lconfd

install:
######################################################################
#
#  Check a distribution out of the source tree, and make a tar file.
#
#.PHONY: dist
#dist:
#	git archive --format=tar --prefix=pam_radius-$(VERSION)/ master | gzip > pam_radius-$(VERSION).tar.gz
#	gpg --default-key aland@freeradius.org -b pam_radius-$(VERSION).tar.gz


######################################################################
#
#  Clean up everything
#
clean:
	@rm -f *~ *.so *.o $(NOTIFIER)
