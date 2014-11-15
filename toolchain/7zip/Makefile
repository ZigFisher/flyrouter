include $(TOPDIR)/rules.mk

UTILS_BUILD_DIR:=$(TOOL_BUILD_DIR)/7zip

#CFLAGS=-g -Wall


prepare:
	mkdir -p $(UTILS_BUILD_DIR)
	cp -a src/* $(UTILS_BUILD_DIR)

compile:
	$(MAKE) -C $(UTILS_BUILD_DIR)

install:
	mkdir -p $(STAGING_DIR)/bin
	$(CP) $(UTILS_BUILD_DIR)/7zip $(STAGING_DIR)/bin/

clean:
	rm -rf $(UTILS_BUILD_DIR)



