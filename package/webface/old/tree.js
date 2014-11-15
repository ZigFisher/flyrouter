var tree,a,b,c,d,e;
function l0(sText, sAction){
	tree = new WebFXTree(sText, sAction);
	tree.setBehavior(null);
}
function l1(sText, sAction){
	a = new WebFXTreeItem(sText, sAction);
    a.target='content';
	tree.add(a);
}
function l2(sText, sAction){
	b = new WebFXTreeItem(sText, sAction);
    b.target='content';
	a.add(b);
}
function l3(sText, sAction){
	c = new WebFXTreeItem(sText, sAction);
    c.target='content';
	b.add(c);
}
function l4(sText, sAction){
	d = new WebFXTreeItem(sText, sAction);
    d.target='content';
	c.add(d);
}
function l5(sText, sAction){
	e = new WebFXTreeItem(sText, sAction);
    e.target='content';
	d.add(e);
}

function writeTree(){
	document.write(tree);
}

if (document.getElementById) {
	l0('Settings');
		l1('General', 'settings/general.asp');
		l1('Date/Time', 'settings/time.asp');
		l1('Modem', 'settings/modem.asp');
		l1('Networking', 'settings/network.asp');
			l2('DNS', 'settings/dns.asp');
			l2('Static routes', 'settings/routes.asp');
			l2('DHCP', 'settings/dhcp.asp');
			l2('Firewall', 'settings/fw.asp');
			l2('DMZ', 'settings/dmz.asp');
	writeTree();


	l0('Status');
		l1('General', 'status/general.asp');
		l1('Modem', 'status/modem.asp');
		l1('Interfaces', 'status/ifaces.asp');
		l1('Routes', 'status/routes.asp');
		l1('ARP cache', 'status/arp.asp');
		l1('DHCP', 'status/dhcp.asp');
		l1('Disk usage', 'status/df.asp');
		l1('System log', 'status/syslog.asp');
	writeTree();

	l0('Tools', "a");
		l1('Ping');
		l1('Traceroute');
		l1('Upgrade');
		l1('Reset to defaults');
	writeTree();

	l0('Services');
		l1('SSH');
		l1('WWW Server');
	writeTree();

	l0('Security');
		l1('Users');
	writeTree();

	l0('Debug');
		l1('kdb', 'debug/kdb.asp');
		l1('configs', 'debug/configs.asp');
	writeTree();

}
