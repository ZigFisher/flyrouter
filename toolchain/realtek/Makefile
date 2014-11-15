include $(TOPDIR)/rules.mk

UTILS_BUILD_DIR:=$(TOOL_BUILD_DIR)/realtek
#CFLAGS=-g -Wall


prepare:
	mkdir -p $(UTILS_BUILD_DIR)
	cp -a src/* $(UTILS_BUILD_DIR)

compile:
	$(MAKE) -C $(UTILS_BUILD_DIR)

install:
	mkdir -p $(STAGING_DIR)/bin
	#$(CP) $(UTILS_BUILD_DIR)/alpha_pack $(STAGING_DIR)/bin/
	$(CP) $(UTILS_BUILD_DIR)/packbin $(STAGING_DIR)/bin/
	$(CP) $(UTILS_BUILD_DIR)/packbin2 $(STAGING_DIR)/bin/
	$(CP) $(UTILS_BUILD_DIR)/packhdr $(STAGING_DIR)/bin/
	$(CP) $(UTILS_BUILD_DIR)/kfspad $(STAGING_DIR)/bin/

clean:
	rm -rf $(UTILS_BUILD_DIR)



