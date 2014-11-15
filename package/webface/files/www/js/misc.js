var tri_open = "";
var tri_closed = "";

function addLoadEvent(func){
	var oldonload = window.onload;
	if(typeof window.onload != "function"){
		window.onload = func;
	} 
	else{
		window.onload = function(){
			oldonload();
			func();
		}
	}
}

function preload() {
	if (document.images) {
		tri_open = new Image(14,10);
		tri_closed = new Image(14,10);
		tri_open.src = "img/tri_o.gif";
		tri_closed.src = "img/tri_c.gif";
	}
}

addLoadEvent(preload);

function showhide(tspan, tri) {
	tspanel = document.getElementById(tspan);
	triel = document.getElementById(tri);
	if (tspanel.style.display == 'none') {
		tspanel.style.display = '';
		triel.src = "img/tri_o.gif";
	} else {
		tspanel.style.display = 'none';
		triel.src = "img/tri_c.gif";
	}
}

function openPopup(win, controller, item, exturl) {
    var exturl = (exturl == null) ? "" : "&"+exturl;
	var w = 500;
	var h = 600;
	var winl = (screen.width-w)/2;
	var wint = (screen.height-h)/2;
	if (winl < 0) winl = 0;
	if (wint < 0) wint = 0;
	popup=win.open("/?controller=" + controller + "&item=" + item + "&popup=1"+exturl, "popup", "width="+w+",height="+h+",top="+wint+",left="+winl+",modal=1,dialog=1,centerscreen=1,scrollbars=0,menubar=0,location=0,toolbar=0,dependent=1,status=0"); 
	popup.focus();
}

function openHelp(file, topic) {
	var w = 600;
	var h = 800;
	var winl = (screen.width+300-w)/2;
	var wint = (screen.height-h)/2;
	if (winl < 0) winl = 0;
	if (wint < 0) wint = 0;
	popup=window.open("/help/" + file + ".html#" + topic, "help", "width="+w+",height="+h+",top="+wint+",left="+winl+",menubar=0,location=0,toolbar=0,dependent=1,status=0,scrollbars=yes"); 
	popup.focus();
}

function confirmSubmit() {
	if (confirm("Are you sure?"))  { 
		return true; 
	};
		return false;
}

