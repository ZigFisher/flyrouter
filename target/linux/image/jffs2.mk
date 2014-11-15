ifneq ($(BR2_BIG_ENDIAN),y)
JFFS2OPTS :=  -l -v -n -q
else
JFFS2OPTS :=  --pad --big-endian --squash
endif

#JFFS2OPTS += -Xlzo -msize -Xlzari

jffs2-prepare:
	$(MAKE) -C jffs2 prepare $(MAKE_TRACE)

jffs2-compile: prepare-targets
	$(MAKE) -C jffs2 compile $(MAKE_TRACE)

jffs2-clean:
	$(MAKE) -C jffs2 clean $(MAKE_TRACE)
	rm -f $(KDIR)/root.jffs2*

$(KDIR)/root.jffs2-4MB: install-prepare
	@rm -rf $(KDIR)/root/jffs
	$(STAGING_DIR)/bin/mkfs.jffs2 $(JFFS2OPTS) -e 0x10000 -o $@ -d $(KDIR)/root $(MAKE_TRACE)

$(KDIR)/root.jffs2-8MB: install-prepare
	@rm -rf $(KDIR)/root/jffs
	$(STAGING_DIR)/bin/mkfs.jffs2 $(JFFS2OPTS) -e 0x20000 -o $@ -d $(KDIR)/root $(MAKE_TRACE)

$(KDIR)/root.jffs2-32MB: install-prepare $(BOARD)-prepare
	@rm -rf $(KDIR)/root/jffs
	$(STAGING_DIR)/bin/mkfs.jffs2 $(JFFS2OPTS) -e 0x4000 -o $@ -d $(KDIR)/root $(MAKE_TRACE)

ifeq ($(IB),)
jffs2-install: compile-targets $(BOARD)-compile
endif

jffs2-install: $(KDIR)/root.jffs2-32MB
	$(TRACE) target/linux/image/$(BOARD)/install
	$(MAKE) -C $(BOARD) install KERNEL="$(KERNEL)" FS="jffs2-32MB" BOARD="$(BOARD)"

jffs2-install-ib: compile-targets
	mkdir -p $(IB_DIR)/staging_dir_$(ARCH)/bin
	$(CP) $(STAGING_DIR)/bin/mkfs.jffs2 $(IB_DIR)/staging_dir_$(ARCH)/bin

prepare-targets: jffs2-prepare
compile-targets: jffs2-compile
install-targets: jffs2-install
install-ib: jffs2-install-ib
clean-targets: jffs2-clean

