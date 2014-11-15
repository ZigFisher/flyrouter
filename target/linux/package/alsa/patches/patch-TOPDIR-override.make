diff -Nur alsa-driver-1.0.11.orig/Rules.make alsa-driver-1.0.11/Rules.make
--- alsa-driver-1.0.11.orig/Rules.make	2006-01-04 16:52:31.000000000 +0100
+++ alsa-driver-1.0.11/Rules.make	2006-08-03 16:29:19.000000000 +0200
@@ -58,7 +58,7 @@
 
 else # ! NEW_KBUILD
 
-TOPDIR = $(MAINSRCDIR)
+override TOPDIR        := $(MAINSRCDIR)
 
 comma = ,
 
diff -Nur alsa-driver-1.0.11.orig/support/Makefile alsa-driver-1.0.11/support/Makefile
--- alsa-driver-1.0.11.orig/support/Makefile	2003-04-07 11:51:24.000000000 +0200
+++ alsa-driver-1.0.11/support/Makefile	2006-08-03 17:21:42.000000000 +0200
@@ -3,7 +3,7 @@
 # Copyright (c) 2001 by Jaroslav Kysela <perex@suse.cz>
 #
 
-TOPDIR = ..
+override TOPDIR        := ..
 
 include $(TOPDIR)/toplevel.config
 include $(TOPDIR)/Makefile.conf
