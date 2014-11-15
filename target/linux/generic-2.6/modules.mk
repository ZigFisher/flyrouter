include ../netfilter.mk

# Networking

$(eval $(call KMOD_template,GRE,gre,\
	$(MODULES_DIR)/kernel/net/ipv4/ip_gre.ko \
,CONFIG_NET_IPGRE,,54,ip_gre))

$(eval $(call KMOD_template,BONDING,bonding,\
	$(MODULES_DIR)/kernel/drivers/net/bonding/bonding.ko \
,CONFIG_BONDING,,20,bonding))

$(eval $(call KMOD_template,IMQ,imq,\
	$(MODULES_DIR)/kernel/net/ipv4/netfilter/*IMQ*.ko \
	$(MODULES_DIR)/kernel/drivers/net/imq.ko \
))

$(eval $(call KMOD_template,IPV6,ipv6,\
	$(MODULES_DIR)/kernel/net/ipv6/ipv6.ko \
,CONFIG_IPV6,,20,ipv6))

$(eval $(call KMOD_template,PPP,ppp,\
	$(MODULES_DIR)/kernel/drivers/net/ppp_async.ko \
	$(MODULES_DIR)/kernel/drivers/net/ppp_generic.ko \
	$(MODULES_DIR)/kernel/drivers/net/slhc.ko \
	$(MODULES_DIR)/kernel/drivers/net/pppox.ko \
,CONFIG_PPP,,55,slhc ppp_generic ppp_async pppox))

$(eval $(call KMOD_template,MPPE,mppe,\
	$(MODULES_DIR)/kernel/drivers/net/ppp_mppe_mppc.ko \
,CONFIG_PPP_MPPE_MPPC))

$(eval $(call KMOD_template,PPPOE,pppoe,\
	$(MODULES_DIR)/kernel/drivers/net/pppoe.ko \
,CONFIG_PPPOE))

$(eval $(call KMOD_template,SCHED,sched,\
	$(MODULES_DIR)/kernel/net/sched/*.ko \
,,,80,act_gact act_ipt act_mirred act_pedit act_police cls_fw cls_route cls_rsvp cls_rsvp6 cls_tcindex cls_u32 em_cmp em_meta em_nbyte \
	  em_text em_u32 sch_cbq sch_dsmark sch_esfq sch_gred sch_hfsc sch_htb sch_netem sch_prio sch_red sch_sfq sch_tbf sch_teql))

$(eval $(call KMOD_template,TUN,tun,\
	$(MODULES_DIR)/kernel/drivers/net/tun.ko \
,CONFIG_TUN,,20,tun))


# Filtering / Firewalling

$(eval $(call KMOD_template,ARPTABLES,arptables,\
	$(MODULES_DIR)/kernel/net/ipv4/netfilter/arp*.ko \
,CONFIG_IP_NF_ARPTABLES,,55,arp_tables arpt_mangle arptable_filter))

$(eval $(call KMOD_template,EBTABLES,ebtables,\
	$(MODULES_DIR)/kernel/net/bridge/netfilter/*.ko \
,CONFIG_BRIDGE_NF_EBTABLES,,70,ebtables ebtable_filter ebt_ip ebt_pkttype))

# metapackage for compatibility ...
$(eval $(call KMOD_template,IPTABLES_EXTRA,iptables-extra,\
,,kmod-ipt-conntrack kmod-ipt-extra kmod-ipt-filter kmod-ipt-ipopt kmod-ipt-ipsec kmod-ipt-nat kmod-ipt-nat-extra kmod-ipt-queue kmod-ipt-ulogd))

$(eval $(call KMOD_template,IPT_CONNTRACK,ipt-conntrack,\
	$(foreach mod,$(IPT_CONNTRACK-m),$(MODULES_DIR)/kernel/net/$(mod).ko) \
	$(MODULES_DIR)/kernel/net/ipv4/netfilter/ip_conntrack.ko \
	$(MODULES_DIR)/kernel/net/ipv4/netfilter/iptable_nat.ko \
,,,40,ip_conntrack iptable_nat ipt_state))

$(eval $(call KMOD_template,IPT_EXTRA,ipt-extra,\
	$(foreach mod, xt_limit xt_pkttype xt_string, $(MODULES_DIR)/kernel/net/netfilter/$(mod).ko) \
	$(foreach mod, ipt_LOG ipt_multiport ipt_owner ipt_recent ipt_REJECT, $(MODULES_DIR)/kernel/net/ipv4/netfilter/$(mod).ko) \
,,,80,$(IPT_IPOPT-m)))

$(eval $(call KMOD_template,IPT_FILTER,ipt-filter,\
	$(foreach mod,$(IPT_FILTER-m),$(MODULES_DIR)/kernel/net/$(mod).ko) \
,,,80,$(IPT_FILTER-m)))

$(eval $(call KMOD_template,IPT_IPOPT,ipt-ipopt,\
	$(foreach mod, xt_length xt_mac xt_mark xt_MARK xt_tcpmss, $(MODULES_DIR)/kernel/net/netfilter/$(mod).ko) \
	$(foreach mod, ipt_dscp ipt_DSCP ipt_ecn ipt_ECN ipt_TCPMSS ipt_tos ipt_time ipt_TOS ipt_ttl ipt_TTL, $(MODULES_DIR)/kernel/net/ipv4/netfilter/$(mod).ko) \
,,,80,$(IPT_IPOPT-m)))

$(eval $(call KMOD_template,IPT_IPSEC,ipt-ipsec,\
	$(foreach mod,$(IPT_IPSEC-m),$(MODULES_DIR)/kernel/net/$(mod).ko) \
))

$(eval $(call KMOD_template,IPT_NAT,ipt-nat,\
	$(foreach mod,$(IPT_NAT-m),$(MODULES_DIR)/kernel/net/$(mod).ko) \
	$(shell [ -d $(MODULES_DIR) ] && cd $(MODULES_DIR) && find $(MODULES_DIR) -name ip*nat*.ko) \
))

$(eval $(call KMOD_template,IPT_NAT_EXTRA,ipt-nat-extra,\
	$(foreach mod,$(IPT_NAT_EXTRA-m),$(MODULES_DIR)/kernel/net/$(mod).ko) \
,,,40,ip_conntrack_proto_gre ip_conntrack_pptp ip_conntrack_ftp ip_nat_proto_gre ip_nat_pptp ip_nat_ftp))
# Old string!  ,,,40,$(IPT_NAT_EXTRA-m)))

$(eval $(call KMOD_template,IPT_QUEUE,ipt-queue,\
	$(foreach mod,$(IPT_QUEUE-m),$(MODULES_DIR)/kernel/net/ipv4/netfilter/$(mod).ko) \
))

$(eval $(call KMOD_template,IPT_ULOG,ipt-ulog,\
	$(foreach mod,$(IPT_ULOG-m),$(MODULES_DIR)/kernel/net/$(mod).ko) \
))

$(eval $(call KMOD_template,IP6TABLES,ip6tables,\
	$(MODULES_DIR)/kernel/net/ipv6/netfilter/ip*.ko \
,CONFIG_IP6_NF_IPTABLES,kmod-ipv6))


# All modules
$(eval $(call KMOD_template,ALLMODULES,allmodules,\
	$(shell [ -d $(MODULES_DIR) ] && find $(MODULES_DIR) -name *.ko) \
,,,,))
