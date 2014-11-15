//\/////
//\  overLIB Speech Bubble Plugin
//\
//\  You may not remove or change this notice.
//\  Copyright Erik Bosrup 1998-2003. All rights reserved.
//\  Contributors are listed on the homepage.
//\  See http://www.bosrup.com/web/overlib/ for details.
//\/////
////////
// PRE-INIT
// Ignore these lines, configuration is below.
////////
if (typeof olInfo == 'undefined' || typeof olInfo.meets == 'undefined' || !olInfo.meets(4.14)) alert('overLIB 4.14 or later is required for the Speech Bubble Plugin.');
else {
registerCommands('bubble,bubbletype,adjbubble');
var imgWidth=[250,330,144,202,200];						    // image width in pixels
var imgHeight=[150,160,190,221,66];							  // image height in pixels
var contentWidth=[200,250,130,184,190];						// image content width in pixels
var contentHeight=[80,85,150,176,46];						  // image content height in pixels
var padLeft=[30,40,7,9,5];										  // X displacement of content from image upper left corner
var padTop=[25,48,10,34,4];										  // Y displacement of content from image upper left corner
var arwTipX=[180,50,51,9,19];                    // x location of tip from image upper left corner
var arwTipY=[148,5,180,221,64];                  // y location of tip from image upper left corner
////////
// DEFAULT CONFIGURATION
// Settings you want everywhere are set here. All of this can also be
// changed on your html page or through an overLIB call.
////////
if (typeof ol_bubble=='undefined') var ol_bubble=0;
if (typeof ol_bubbletype=='undefined') var ol_bubbletype='';
if (typeof ol_adjbubble=='undefined') var ol_adjbubble=0;
var olBId, olBContentWd=contentWidth;
////////
// END OF CONFIGURATION
// Don't change anything below this line, all configuration is above.
////////
////////
// INIT
////////
// Runtime variables init. Don't change for config!
var o3_bubble=0;
var o3_bubbletype='';
var o3_adjbubble=0;
////////
// PLUGIN FUNCTIONS
////////
function setBubbleVariables() {
	o3_bubble=ol_bubble;
	o3_bubbletype=ol_bubbletype;
	o3_adjbubble=ol_adjbubble;
}
// Parses Speech Bubble commands
function parseBubbleExtras(pf,i,ar) {
	var k=i;
	if (k < ar.length) {
		if (ar[k]==BUBBLE) { eval(pf +'bubble=('+pf+'bubble==0) ? 1 : 0'); return k; }
		if (ar[k]==BUBBLETYPE) { eval(pf+'bubbletype="'+ar[++k]+'"'); return k; }
		if (ar[k]==ADJBUBBLE) { eval(pf +'adjbubble=('+pf+'adjbubble==0) ? 1 : 0'); return k; }
	}
	return -1;
}
////////
// CHECKS FOR BUBBLE EFFECT
////////
function chkForBubbleEffect() {
	if(o3_bubble) {
		o3_bubbletype=(o3_bubbletype) ? o3_bubbletype : 'flower';
		for (var i=0; i<olBTypes.length; i++) {
			if(olBTypes[i]==o3_bubbletype) {
				olBId=i;
				break;
			}
		}
		// disable inappropriate parameters
		o3_bgcolor=o3_fgcolor='';
		o3_border=0;
		if(o3_fgbackground||o3_bgbackground) o3_fgbackground=o3_bgbackground='';
		if(o3_cap) o3_cap='';
		if(o3_sticky) opt_NOCLOSE();
		if(o3_fullhtml) o3_fullhtml=0;
		if(o3_fixx>0||o3_fixy>0) o3_fixx=o3_fixy=-1;
		if(o3_relx||o3_rely) o3_relx=o3_rely=null;
		if(typeof o3_shadow!='undefined'&&o3_shadow) o3_shadow=0;
		if(o3_bubbletype!='roundcorners') {
			o3_width=olBContentWd[olBId];
			if(o3_hpos==CENTER||o3_hpos==LEFT) o3_hpos=RIGHT;
			if(o3_vpos==ABOVE) o3_vpos=BELOW;
			if(o3_vauto) o3_vauto=0;
			if(o3_hauto) o3_hauto=0;
			if(o3_wrap) o3_wrap=0;
		}
	}
	return true;
}
////////
// BUBBLE EFFECT SUPPORTING ROUTINES
////////
function registerImages(imgStr,path) {
	if(typeof imgStr!='string') return;
	path=(path&&typeof path=='string') ? path : '/img';
	if (path.charAt(path.length-1)=='/') path=path.substring(0,path.length-1);
	if(typeof olBTypes=='undefined') olBTypes=imgStr.split(',');
	if(typeof olBubbleImg=='undefined') {
		olBubbleImg=new Array();
		for (var i=0; i<olBTypes.length; i++) {
			if(olBTypes[i]=='roundcorners') {
				olBubbleImg[i]=new Array();
				var o=olBubbleImg[i];
				o[0]=new Image();
				o[0].src=path+'/cornerTL.gif';
				o[1]=new Image();
				o[1].src=path+'/edgeT.gif';
				o[2]=new Image();
				o[2].src=path+'/cornerTR.gif';
				o[3]=new Image();
				o[3].src=path+'/edgeL.gif';
				o[4]=new Image();
				o[4].src=path+'/edgeR.gif';
				o[5]=new Image();
				o[5].src=path+'/cornerBL.gif';
				o[6]=new Image();
				o[6].src=path+'/edgeB.gif';
				o[7]=new Image();
				o[7].src=path+'/cornerBR.gif';
			} else {
				olBubbleImg[i]=new Image();
				olBubbleImg[i].src=path+'/'+olBTypes[i]+'.gif';
			}
		}
	}
}
function generateBubble(content) {
	if(!o3_bubble) return;
	if(o3_bubbletype=='roundcorners') return doRoundCorners(content);
	var ar,X,Y,fc=1.0,txt,sY,bHtDiff, bPadDiff=0,zIdx=0;
	var bTopPad=padTop,bLeftPad=padLeft;
	var bContentHt=contentHeight, bHt=imgHeight;
	var bWd=imgWidth, bArwTipX=arwTipX, bArwTipY=arwTipY;
//
	bHtDiff=fc*bContentHt[olBId]-(olNs4 ? over.clip.height : over.offsetHeight);
	if (o3_adjbubble) {
		fc=resizeBubble(bHtDiff, 0.5, fc);
		ar=getHeightDiff(fc);
		bHtDiff=ar[0];
		content=ar[1];
	}
	if(bHtDiff>0) bPadDiff=parseInt(0.5*bHtDiff);
	Y=(bHtDiff<0) ? fc*bTopPad[olBId] : fc*bTopPad[olBId]+bPadDiff;
	X=fc*bLeftPad[olBId];
	Y=Math.round(Y);
	X=Math.round(X);
//
	txt=(olNs4) ? '<div id="bLayer">' : ((olIe55&&olHideForm) ? backDropSource(Math.round(fc*bWd[olBId]),Math.round((bHtDiff<0 ? fc*bHt[olBId]-bHtDiff : fc*bHt[olBId])),zIdx++) : '') + '<div id="bLayer" style="position: absolute; top:0; left:0; width: '+ Math.round(fc*bWd[olBId]) +'px; z-index: '+(zIdx++)+';">';
	txt += '<img src="'+olBubbleImg[olBId].src+'" width="'+Math.round(fc*bWd[olBId])+'" height="'+Math.round((bHtDiff<0 ? fc*bHt[olBId]-bHtDiff : fc*bHt[olBId]))+'" /></div>';
	txt += (olNs4 ? '<div id="bContent">' : '<div id="bContent" style="position: absolute; top:'+Y+'px; left:'+X+'px; width: '+ Math.round(fc*olBContentWd[olBId])+'px; z-index: '+(zIdx++)+';">')+content+'</div>'
//
	layerWrite(txt);
	if(olNs4) {
		var imgLyr=over.document.layers['bLayer'];
		var cLyr=over.document.layers['bContent'];
		imgLyr.zIndex=0;
		cLyr.zIndex=1;
		cLyr.top=Y;
		cLyr.left=X;
	}
	// Set popup's new width and height values here so they are available in placeLayer()
	o3_width=Math.round(fc*bWd[olBId]);
	o3_aboveheight=Math.round(fc*bHt[olBId]);
	if(fc*bArwTipY[olBId] < 0.5*fc*bHt[olBId]) 
		sY = fc*bArwTipY[olBId]; 
	else
		sY = -(fc*bHt[olBId]+20)
	o3_offsetx-=Math.round(fc*bArwTipX[olBId]);
	o3_offsety+=Math.round(sY);
}
function doRoundCorners(content) {
	var txt='', wd,ht, zIdx=0, o=olBubbleImg[olBId];
	wd=(olNs4) ? over.clip.width : over.offsetWidth;
	ht=(olNs4) ? over.clip.height : over.offsetHeight;
	txt = (olIe55&&olHideForm) ? backDropSource(wd+28,ht+28,zIdx++) + '<div style="position:absolute; top: 0; left: 0; z-index: '+(zIdx++)+';">' : '';
	txt+= '<div class=tooltip><table cellpadding="0" cellspacing="0" border="0">'+
				'<tr><td align="right" valign="bottom"><img src="'+o[0].src+'" width="14" height="14"'+(olNs6 ? ' style="display: block;"' : '')+' /></td><td valign="bottom"><img src="'+o[1].src+'" height="14" width="'+wd+'"'+(olNs6 ? ' style="display: block;"' : '')+' /></td><td align="left" valign="bottom"><img src="'+o[2].src+'" width="14" height="14"'+(olNs6 ? ' style="display: block;"' : '')+' /></td></tr>'+
				'<tr><td align="right"><img src="'+o[3].src+'" width="14" height="'+ht+'"'+(olNs6 ? ' style="display: block;"' : '')+' /></td><td bgcolor="#ffffcc">'+content+'</td><td align="left"><img src="'+o[4].src+'" width="14" height="'+ht+'"'+(olNs6 ? ' style="display: block;"' : '')+' /></td></tr>'+
				'<tr><td align="right" valign="top"><img src="'+o[5].src+'" width="14" height="14" /></td><td valign="top"><img src="'+o[6].src+'" height="14" width="'+wd+'" /></td><td align="left" valign="top"><img src="'+o[7].src+'" width="14" height="14" /></td></tr></table></div>';
	txt += (olIe55&&olHideForm) ? '</div>' : '';
	layerWrite(txt);
	o3_width=wd+28;
	o3_aboveheight=ht+28;
}
function resizeBubble(h1, dF, fold) {
	var df,h2,fnew,alpha,cnt=0;
	while(cnt<2) {
		df=-signOf(h1)*dF;
		fnew=fold+df;
		h2=getHeightDiff(fnew)[0];
		if(Math.abs(h2)<o3_textsize) break;
		if (signOf(h1)!=signOf(h2)) {
			alpha=Math.abs(h1)/(Math.abs(h1)+Math.abs(h2));
			if(h1<0) fnew=alpha*fnew+(1.0-alpha)*fold;
			else fnew=(1.0-alpha)*fnew+alpha*fold;
		} else {
			alpha=Math.abs(h1)/(Math.abs(h2)-Math.abs(h1));
			if(h1<0) fnew=(1.0+alpha)*fold-alpha*fnew;
			else fnew=(1.0+alpha)*fnew-alpha*fold;
		}
		fold=fnew;
		h1=h2;
		dF*=0.5;
		cnt++;
	}
	return fnew;
}
function getHeightDiff(f) {
	var lyrhtml;
	o3_width=Math.round(f*contentWidth[olBId]);
	lyrhtml=runHook('ol_content_simple',FALTERNATE,o3_css,o3_text);
	layerWrite(lyrhtml)
	return [Math.round(f*contentHeight[olBId])-((olNs4) ? over.clip.height : over.offsetHeight),lyrhtml];
}
function signOf(x) {
	return (x<0) ? -1 : 1;
}
////////
// PLUGIN REGISTRATIONS
////////
registerRunTimeFunction(setBubbleVariables);
registerCmdLineFunction(parseBubbleExtras);
registerPostParseFunction(chkForBubbleEffect);
registerHook("createPopup",generateBubble,FAFTER);
registerImages('roundcorners'); // root names given to images (should be gif file)
//registerImages('roundcorners'); // root names given to images (should be gif file)
//registerImages('flower,oval,square,pushpin,quotation,roundcorners'); // root names given to images (should be gif file)
if(olInfo.meets(4.14)) registerNoParameterCommands('bubble,adjbubble');
if(olNs4) document.write('<style type="text/css">\n<!--\n#bLayer, #bContent {position: absolute;}\n-->\n<'+'\/style>');
}
//end 
