/** 
* Copyright 2005-2006 massimocorner.com
* @author      Massimo Foti (massimo@massimocorner.com)
* @version     1.3.1, 2006-07-22
* modified by vlad
 */

// Create all the validator objects required inside the document
function tmt_validatorInit(){
	var formNodes = document.getElementsByTagName("form");
	for(var i=0; i<formNodes.length; i++){
		if(formNodes[i].getAttribute("tmt:validate") == "true"){
			// Attach a validator object to each form that requires it
			formNodes[i].tmt_validator = new tmt_formValidator(formNodes[i]);
			// Set the form node's onsubmit event 
			// We use a gigantic hack to preserve exiting calls attached to the onsubmit event (most likely validation routines)
			if(typeof formNodes[i].onsubmit != "function"){
				formNodes[i].onsubmit = function(){
					return tmt_validateForm(this);
				}
			}
			else{
				// Store a reference to the old function
				formNodes[i].tmt_oldSubmit = formNodes[i].onsubmit;
				formNodes[i].onsubmit = function(){
					// If the existing function return true, send the form
					if(this.tmt_oldSubmit()){
						return tmt_validateForm(this);
					}
					return false;
				}
			}
		}
	}
}

// Perform the validation
function tmt_validateForm(formNode){
	var errorMsg = "";
	var formValidator = formNode.tmt_validator;
	// Be sure the form contains a validator object
	if(formValidator){
		var focusGiven = false;
		// This array will store all the field validators that contains errors
		// They may be required by the callback
		var invalidFields = new Array();
		// Validate all the fields
		for(var i=0; i<formValidator.validators.length; i++){
			if(formValidator.validators[i].validate()){
				// Append to the global error string
				errorMsg += formValidator.validators[i].message + "\n";
				invalidFields[invalidFields.length] = formValidator.validators[i];
				// Give focus to the first invalid text/textarea field
				if(!focusGiven && (formValidator.validators[i].type == "text")){
					formValidator.validators[i].getFocus();
					focusGiven = true;
				}
			}
		}
		if(errorMsg != ""){
			// We have errors, display them
			if(!formValidator.callback){
				// We don't have callbacks, just display an alert
				alert(errorMsg);
			}
			else{
				// Invoke the callbak, it will take care of displaying the errors
				eval(formValidator.callback + "(formNode, invalidFields)");
			}
		}
		else{
			// Everything is fine, disable form submission to avoid multiple submits
			formValidator.blockSubmit();
		}
	}
	return errorMsg.length == 0; 
}

/* Object constructors */

// Form validator
function tmt_formValidator(formNode){
	// Store all the validator objects inside an array
	this.validators = new Array();
	// Add the specified callback only if the function is currently defined
	if(formNode.getAttribute("tmt:callback") && window[formNode.getAttribute("tmt:callback")]){
		this.callback = formNode.getAttribute("tmt:callback");
	}
	var fieldsArray = tmt_getTextfieldNodes(formNode);
	for(var i=0; i<fieldsArray.length; i++){
		// Create a validator for each text field
		this.validators[this.validators.length] = tmt_textValidatorFactory(fieldsArray[i]);
		
		if(fieldsArray[i].getAttribute("type")){
			// Set the onchange event for each image upload validation
			if((fieldsArray[i].getAttribute("type").toLowerCase() == "file") &&	(fieldsArray[i].getAttribute("tmt:image") == "true")){
				fieldsArray[i].onchange = function(){
					tmt_validateImg(this);
				}
			}
		}
		if(fieldsArray[i].getAttribute("tmt:filters")){
			// Call the filters on the onkeyup and onblur events
			addEvent(fieldsArray[i], "keyup", function(){tmt_filterField(this);});
			addEvent(fieldsArray[i], "blur", function(){tmt_filterField(this);});
		}
	}
	var selectNodes = formNode.getElementsByTagName("select");
	for(var j=0; j<selectNodes.length; j++){
		// Create a validator for each select element
		this.validators[this.validators.length] = tmt_selectValidatorFactory(selectNodes[j]);
	}
	var boxTable = tmt_getNodesTable(formNode, "checkbox");
	for(var boxName in boxTable){
		// Create a validator for each group of checkboxes
		this.validators[this.validators.length] = tmt_boxValidatorFactory(boxTable[boxName]);
	}
	var radioTable = tmt_getNodesTable(formNode, "radio");
	for(var radioName in radioTable){
		// Create a validator for each group of radios
		this.validators[this.validators.length] = tmt_radioValidatorFactory(radioTable[radioName]);
	}
	// Store all the submit buttons
	this.buttons = tmt_getSubmitNodes(formNode);
	// Define a method that can block multiple submits
	this.blockSubmit = function(){
		// Check to see if we want to disable submit buttons
		if(!formNode.getAttribute("tmt:blocksubmit") && !(formNode.getAttribute("tmt:blocksubmit") == "false")){
			// Disable each submit button
			for(var i=0; i<this.buttons.length; i++){
				if(this.buttons[i].getAttribute("tmt:waitmessage")){
					this.buttons[i].value = this.buttons[i].getAttribute("tmt:waitmessage");
				}
				this.buttons[i].disabled = true;
			}
		}
	}
}

// Abstract field validator constructor
function tmt_abstractValidator(fieldNode){
	this.message = "";
	this.name = fieldNode.name;
	if(fieldNode.getAttribute("tmt:message")){
		this.message = fieldNode.getAttribute("tmt:message");
	}
	var errorClass = "";
	if(fieldNode.getAttribute("tmt:errorclass")){
		errorClass = fieldNode.getAttribute("tmt:errorclass");
	}
	this.flagInvalid = function(){
		// Append the CSS class to the existing one
		if(errorClass){
			// Flag only if it's not already flagged
			if(fieldNode.className.indexOf(errorClass) == -1){
				fieldNode.className = fieldNode.className + " " + errorClass;
			}
		}
		// Set the title attribute in order to show a tootip
		fieldNode.setAttribute("title", this.message);
	}
	this.flagValid = function(){
		// Remove the CSS class
		if(errorClass){
			var regClass = new RegExp("\\b" + errorClass);
			fieldNode.className = fieldNode.className.replace(regClass, "");
		}
		fieldNode.removeAttribute("title");
	}
	this.validate = function(){
		// If the field contains error, flag it as invalid and return the error message
		// Be careful, this method contains multiple exit points!!!
		if(fieldNode.disabled){
			// Disabled fields are always valid
			this.flagValid();
			return false;
		}
		if(!this.isValid()){
			this.flagInvalid();
			return true;
		}
		else{
			this.flagValid();
			return false;
		}
	}
}

// Create a validator for text and texarea fields
function tmt_textValidatorFactory(fieldNode){
	// Create a generic validator, than add specific properties and methods as needed
	var obj = new tmt_abstractValidator(fieldNode);
	obj.type = "text";
	var required = false;
	if(fieldNode.getAttribute("tmt:required")){
		required = fieldNode.getAttribute("tmt:required");
	}
	// Put focus and cursor inside the field
	obj.getFocus = function(){
		// This try block is required to solve an obscure issue with IE and hidden fields
		try{
			fieldNode.focus();
			fieldNode.select();
		}
		catch(exception){
		}
	}
	// Check if the field is empty
	obj.isEmpty = function(){
		return fieldNode.value == "";
	}
	// Check if the field is required
	obj.isRequired = function(){
		return required;
	}
	// Check if the field satisfy the rules associated with it
	// Be careful, this method contains multiple exit points!!!
	obj.isValid = function(){
		// The tmt:required="conditional" attribute has a special meaning. 
		// The field isn't strictly required, so it may sometimes be empty, 
		// but before we let it go, we need to check any rule that may apply to it
		if(obj.isEmpty() && (required != "conditional")){
			if(obj.isRequired()){
				return false;
			}
			else{
				return true;
			}
		}
		else{
			// Loop over all the available rules
			for(var rule in tmt_globalRules){
				// Check if the current rule is required for the field
				if(fieldNode.getAttribute("tmt:" + rule)){
					// Invoke the rule
					if(!eval("tmt_globalRules." + rule + "(fieldNode)")){
						return false;
					}
				}
			}
		}
		return true;
	}
	return obj;
}

// Create a validator for <select> fields
function tmt_selectValidatorFactory(selectNode){
	// Create a generic validator, than add specific properties and methods as needed
	var obj = new tmt_abstractValidator(selectNode);
	obj.type = "select";
	var required = false;
	var invalidIndex;
	if(selectNode.getAttribute("tmt:invalidindex")){
		invalidIndex = selectNode.getAttribute("tmt:invalidindex");
	}
	var invalidValue;
	if(selectNode.getAttribute("tmt:invalidvalue") != null){
		invalidValue = selectNode.getAttribute("tmt:invalidvalue");
	}
	// Check if the field is required
	obj.isRequired = function(){
		return required;
	}
	// Check if the field satisfy the rules associated with it
	// Be careful, this method contains multiple exit points!!!	// Check if the select validate
	obj.isValid = function(){
		// Check for index
		if(selectNode.selectedIndex == invalidIndex){
			return false;
		}
		// Check for value
		if(selectNode.value == invalidValue){
			return false;
		}
		// Loop over all the available rules
		for(var rule in tmt_globalRules){
			// Check if the current rule is required for the field
			if(selectNode.getAttribute("tmt:" + rule)){
				// Invoke the rule
				if(!eval("tmt_globalRules." + rule + "(selectNode)")){
					return false;
				}
			}
		}
		return true;
	}
	return obj;
}

// Generic validator for grouped fields (radio and checkboxes)
function tmt_groupValidatorFactory(buttonGroup){
	this.name = buttonGroup.name;
	this.message = "";
	this.errorClass = "";
	// Since fields from the same group can have conflicting attribute values, the last one win
	for(var i=0; i<buttonGroup.elements.length; i++){
		if(buttonGroup.elements[i].getAttribute("tmt:message")){
			this.message = buttonGroup.elements[i].getAttribute("tmt:message");
		}
		if(buttonGroup.elements[i].getAttribute("tmt:errorclass")){
			this.errorClass = buttonGroup.elements[i].getAttribute("tmt:errorclass");
		}
	}
	this.flagInvalid = function(){
		// Append the CSS class to the existing one
		if(this.errorClass){
			for(var i=0; i<buttonGroup.elements.length; i++){
				// Flag only if it's not already flagged
				if(buttonGroup.elements[i].className.indexOf(this.errorClass) == -1){
					buttonGroup.elements[i].className = buttonGroup.elements[i].className + " " + this.errorClass;
				}
				buttonGroup.elements[i].setAttribute("title", this.message);
			}
		}
	}
	this.flagValid = function(){
		// Remove the CSS class
		if(this.errorClass){
			var regClass = new RegExp("\\b" + this.errorClass);
			for(var i=0; i<buttonGroup.elements.length; i++){
				buttonGroup.elements[i].className = buttonGroup.elements[i].className.replace(regClass, "");
				buttonGroup.elements[i].removeAttribute("title");
			}
		}
	}
	this.validate = function(){
		var errorMsg = "";
		// If the field group contains error, flag it as invalid and return the error message
		if(!this.isValid()){
			errorMsg += this.message;
			this.flagInvalid();
		}
		else{
			this.flagValid();
		}
		return errorMsg;
	}
}

// Checkbox validator (one for each group of boxes sharing the same name)
function tmt_boxValidatorFactory(boxGroup){
	var obj = new tmt_groupValidatorFactory(boxGroup);
	obj.type = "box";
	var minchecked = 0;
	var maxchecked = boxGroup.elements.length;
	// Since checkboxes from the same group can have conflicting attribute values, the last one win
	for(var i=0; i<boxGroup.elements.length; i++){
		if(boxGroup.elements[i].getAttribute("tmt:minchecked")){
			minchecked = boxGroup.elements[i].getAttribute("tmt:minchecked");
		}
		if(boxGroup.elements[i].getAttribute("tmt:maxchecked")){
			maxchecked = boxGroup.elements[i].getAttribute("tmt:maxchecked");
		}
	}
	// Check if the boxes validate
	obj.isValid = function(){
		var checkCounter = 0;
		for(var i=0; i<boxGroup.elements.length; i++){
		    // For each checked box, increase the counter
			if(boxGroup.elements[i].checked){
				checkCounter++;
			}
		}
		return (checkCounter >=  minchecked) && (checkCounter <= maxchecked);
	}
	return obj;
}

// Radio validator (one for each group of radios sharing the same name)
function tmt_radioValidatorFactory(radioGroup){
	var obj = new tmt_groupValidatorFactory(radioGroup);
	obj.type = "radio";

	obj.isRequired = function(){
		var requiredFlag = false;
		// Since radios from the same group can have conflicting attribute values, the last one win
		for(var i=0; i<radioGroup.elements.length; i++){
			if(radioGroup.elements[i].disabled == false){
				if(radioGroup.elements[i].getAttribute("tmt:required")){
					requiredFlag = radioGroup.elements[i].getAttribute("tmt:required");
				}
			}
		}
		return requiredFlag;
	}
	
	// Check if the radio validate
	obj.isValid = function(){
		if(obj.isRequired()){
			for(var i=0; i<radioGroup.elements.length; i++){
				// As soon as one is checked, we are fine
				if(radioGroup.elements[i].checked){
					return true;
				}
			}
			return false;
		}
		// It's not required, so it's fine
		else{
			return true;
		}	
	}
	return obj;
}

// This global objects store all the validation rules
// Every rule is stored as a method that accepts the field node as argument and return a boolean
var tmt_globalRules = new Object;
tmt_globalRules.datepattern = function(fieldNode){
	var globalObj = tmt_globalDatePatterns[fieldNode.getAttribute("tmt:datepattern")];
	if(globalObj){
		// Split the date into 3 different bits using the separator
		var dateBits = fieldNode.value.split(globalObj.s);
		// First try to create a new date out of the bits
		var testDate = new Date(dateBits[globalObj.y], (dateBits[globalObj.m]-1), dateBits[globalObj.d]);
		// Make sure values match after conversion
		var isDate = (testDate.getFullYear() == dateBits[globalObj.y])
				 && (testDate.getMonth() == dateBits[globalObj.m]-1)
				 && (testDate.getDate() == dateBits[globalObj.d]);
		// If it's a date and it matches the RegExp, it's a go
		return isDate && globalObj.rex.test(fieldNode.value);
	}
}
tmt_globalRules.equalto = function(fieldNode){
	var twinNode = document.getElementById(fieldNode.getAttribute("tmt:equalto"));
	return twinNode.value == fieldNode.value;
}
tmt_globalRules.maxlength = function(fieldNode){
	if(fieldNode.value.length > fieldNode.getAttribute("tmt:maxlength")){
		return false;
	}
	return true;
}
tmt_globalRules.maxnumber = function(fieldNode){
	if(parseFloat(fieldNode.value) > fieldNode.getAttribute("tmt:maxnumber")){
		return false;
	}
	return true;
}
tmt_globalRules.minlength = function(fieldNode){
	if(fieldNode.value.length < fieldNode.getAttribute("tmt:minlength")){
		return false;
	}
	return true;
}
tmt_globalRules.minnumber = function(fieldNode){
	if(parseFloat(fieldNode.value) < fieldNode.getAttribute("tmt:minnumber")){
		return false;
	}
	return true;
}
tmt_globalRules.pattern = function(fieldNode){
	var reg = tmt_globalPatterns[fieldNode.getAttribute("tmt:pattern")];
	if(reg){
		return reg.test(fieldNode.value);
	}
	else{
		// If the pattern is missing, skip it
		return true;	
	}
}

/* Image upload validation */

tmt_globalRules.image = function(fieldNode){
	// If the flag isn't defined we assume things are fine
	if(!fieldNode.isValidImg){
		fieldNode.isValidImg = "true";
	}
	return fieldNode.isValidImg == "true";
}

// Check the currently selected image and set a validity flag
function tmt_validateImg(fieldNode){
	var imgURL = "file:///" + fieldNode.value;
	var img = new Image();
	img.maxSize =  fieldNode.getAttribute("tmt:imagemaxsize");
	img.maxWidth = fieldNode.getAttribute("tmt:imagemaxwidth");
	img.minWidth = fieldNode.getAttribute("tmt:imageminwidth");
	img.maxHeight = fieldNode.getAttribute("tmt:imagemaxheight");
	img.minHeight = fieldNode.getAttribute("tmt:imageminheight");
	// Store a reference to the input field
	img.fieldNode = fieldNode;
	// The image's data can be read only after loading. That's why we need a callback
	img.onload = tmt_validateImgCallback;
	img.src = imgURL;
}

function tmt_validateImgCallback(){
	var errorsCount = 0;
	// Check every constrain and increment the error counter accordingly
	if(this.fileSize && this.maxSize && (this.fileSize/1024) > this.maxSize){
		errorsCount ++;
	}
	if(this.maxWidth && (this.width > this.maxWidth)){
		errorsCount ++;
	}
	if(this.minWidth && (this.width < this.minWidth)){
		errorsCount ++;
	}
	if(this.maxHeight && (this.height > this.maxHeight)){
		errorsCount ++;
	}
	if(this.minHeight && (this.height < this.minHeight)){
		errorsCount ++;
	}
	// Store the valid flag inside the DOM node itself
	this.fieldNode.isValidImg = (errorsCount != 0) ? "false" : "true";
}

// This global objects store all the RegExp patterns for strings
var tmt_globalPatterns = new Object;
tmt_globalPatterns.email = new RegExp("^[\\w\\.=-]+@[\\w\\.-]+\\.[\\w\\.-]{2,4}$");
tmt_globalPatterns.lettersonly = new RegExp("^[a-zA-Z]*$");
tmt_globalPatterns.alphanumeric = new RegExp("^\\w*$");
tmt_globalPatterns.integer = new RegExp("^-?\\d\\d*$");
tmt_globalPatterns.positiveinteger = new RegExp("^\\d\\d*$");
tmt_globalPatterns.number = new RegExp("^-?(\\d\\d*\\.\\d*$)|(^-?\\d\\d*$)|(^-?\\.\\d\\d*$)");
tmt_globalPatterns.filepath_pdf = new RegExp("\\\\[\\w_]*\\.([pP][dD][fF])$");
tmt_globalPatterns.filepath_jpg_gif = new RegExp("\\\\[\\w_]*\\.([gG][iI][fF])|([jJ][pP][eE]?[gG])$");
tmt_globalPatterns.filepath_jpg = new RegExp("\\\\[\\w_]*\\.([jJ][pP][eE]?[gG])$");
tmt_globalPatterns.filepath_zip = new RegExp("\\\\[\\w_]*\\.([zZ][iI][pP])$");
tmt_globalPatterns.filepath = new RegExp("\\\\[\\w_]*\\.\\w{3}$");

// This global objects store all the info required for date validation
var tmt_globalDatePatterns = new Object;
tmt_globalDatePatterns["YYYY-MM-DD"] = tmt_dateInfo("^\([0-9]{4}\)\\-\([0-1][0-9]\)\\-\([0-3][0-9]\)$", 0, 1, 2, "-");
tmt_globalDatePatterns["YYYY-M-D"] = tmt_dateInfo("^\([0-9]{4}\)\\-\([0-1]?[0-9]\)\\-\([0-3]?[0-9]\)$", 0, 1, 2, "-");
tmt_globalDatePatterns["MM.DD.YYYY"] = tmt_dateInfo("^\([0-1][0-9]\)\\.\([0-3][0-9]\)\\.\([0-9]{4}\)$", 2, 0, 1, ".");
tmt_globalDatePatterns["M.D.YYYY"] = tmt_dateInfo("^\([0-1]?[0-9]\)\\.\([0-3]?[0-9]\)\\.\([0-9]{4}\)$", 2, 0, 1, ".");
tmt_globalDatePatterns["MM/DD/YYYY"] = tmt_dateInfo("^\([0-1][0-9]\)\/\([0-3][0-9]\)\/\([0-9]{4}\)$", 2, 0, 1, "/");
tmt_globalDatePatterns["M/D/YYYY"] = tmt_dateInfo("^\([0-1]?[0-9]\)\/\([0-3]?[0-9]\)\/\([0-9]{4}\)$", 2, 0, 1, "/");
tmt_globalDatePatterns["MM-DD-YYYY"] = tmt_dateInfo("^\([0-21][0-9]\)\\-\([0-3][0-9]\)\\-\([0-9]{4}\)$", 2, 0, 1, "-");
tmt_globalDatePatterns["M-D-YYYY"] = tmt_dateInfo("^\([0-1]?[0-9]\)\\-\([0-3]?[0-9]\)\\-\([0-9]{4}\)$", 2, 0, 1, "-");
tmt_globalDatePatterns["DD.MM.YYYY"] = tmt_dateInfo("^\([0-3][0-9]\)\\.\([0-1][0-9]\)\\.\([0-9]{4}\)$", 2, 1, 0, ".");
tmt_globalDatePatterns["D.M.YYYY"] = tmt_dateInfo("^\([0-3]?[0-9]\)\\.\([0-1]?[0-9]\)\\.\([0-9]{4}\)$", 2, 1, 0, ".");
tmt_globalDatePatterns["DD/MM/YYYY"] = tmt_dateInfo("^\([0-3][0-9]\)\/\([0-1][0-9]\)\/\([0-9]{4}\)$", 2, 1, 0, "/");
tmt_globalDatePatterns["D/M/YYYY"] = tmt_dateInfo("^\([0-3]?[0-9]\)\/\([0-1]?[0-9]\)\/\([0-9]{4}\)$", 2, 1, 0, "/");
tmt_globalDatePatterns["DD-MM-YYYY"] = tmt_dateInfo("^\([0-3][0-9]\)\\-\([0-1][0-9]\)\\-\([0-9]{4}\)$", 2, 1, 0, "-");
tmt_globalDatePatterns["D-M-YYYY"] = tmt_dateInfo("^\([0-3]?[0-9]\)\\-\([0-1]?[0-9]\)\\-\([0-9]{4}\)$", 2, 1, 0, "-");

// Create an object that stores date validation's info
function tmt_dateInfo(rex, year, month, day, separator){
	var infoObj = new Object;
	infoObj.rex = new RegExp(rex);
	infoObj.y = year;
	infoObj.m = month;
	infoObj.d = day;
	infoObj.s = separator;
	return infoObj;
}

/* Filters */

// This global objects store all the info required for filters
var tmt_globalFilters = new Object;
tmt_globalFilters.ltrim = tmt_filterInfo("^(\\s*)(\\b[\\w\\W]*)$", "$2");
tmt_globalFilters.rtrim = tmt_filterInfo("^([\\w\\W]*)(\\b\\s*)$", "$1");
tmt_globalFilters.nospaces = tmt_filterInfo("\\s*", "");
tmt_globalFilters.nocommas = tmt_filterInfo(",", "");
tmt_globalFilters.nodots = tmt_filterInfo("\\.", "");
tmt_globalFilters.noquotes = tmt_filterInfo("'", "");
tmt_globalFilters.nodoublequotes = tmt_filterInfo('"', "");
tmt_globalFilters.nohtml = tmt_filterInfo("<[^>]*>", "");
tmt_globalFilters.alphanumericonly = tmt_filterInfo("[^\\w]", "");
tmt_globalFilters.numbersonly = tmt_filterInfo("[^\\d]", "");
tmt_globalFilters.lettersonly = tmt_filterInfo("[^a-zA-Z]", "");
tmt_globalFilters.commastodots = tmt_filterInfo(",", ".");
tmt_globalFilters.dotstocommas = tmt_filterInfo("\\.", ",");
tmt_globalFilters.numberscommas = tmt_filterInfo("[^\\d,]", "");
tmt_globalFilters.numbersdots = tmt_filterInfo("[^\\d\\.]", "");

// Create an object that stores filters's info
function tmt_filterInfo(rex, replaceStr){
	var infoObj = new Object;
	infoObj.rex = new RegExp(rex, "g");
	infoObj.str = replaceStr;
	return infoObj;
}

// Clean up the field based on filter's info
function tmt_filterField(fieldNode){
	var filtersArray = fieldNode.getAttribute("tmt:filters").split(",");
	for(var i=0; i<filtersArray.length; i++){
		var filtObj = tmt_globalFilters[filtersArray[i]];
		// Be sure we have the filter's data, then clean up
		if(filtObj){
			fieldNode.value = fieldNode.value.replace(filtObj.rex, filtObj.str)
		}
		// We handle demoroziner as a special case
		if(filtersArray[i] == "demoronizer"){
			fieldNode.value = tmt_filterDemoronizer(fieldNode.value);
		}
	}
}

// Replace MS Word's non-ISO characters with plausible substitutes
function tmt_filterDemoronizer(str){
	str = str.replace(new RegExp(String.fromCharCode(710), "g"), "^");
	str = str.replace(new RegExp(String.fromCharCode(732), "g"), "~");
	// Evil "smarty" quotes
	str = str.replace(new RegExp(String.fromCharCode(8216), "g"), "'");
	str = str.replace(new RegExp(String.fromCharCode(8217), "g"), "'");
	str = str.replace(new RegExp(String.fromCharCode(8220), "g"), '"');
	str = str.replace(new RegExp(String.fromCharCode(8221), "g"), '"');
	// More MS Word's garbage
	str = str.replace(new RegExp(String.fromCharCode(8211), "g"), "-");
	str = str.replace(new RegExp(String.fromCharCode(8212), "g"), "--");
	str = str.replace(new RegExp(String.fromCharCode(8218), "g"), ",");
	str = str.replace(new RegExp(String.fromCharCode(8222), "g"), ",,");
	str = str.replace(new RegExp(String.fromCharCode(8226), "g"), "*");
	str = str.replace(new RegExp(String.fromCharCode(8230), "g"), "...");
	str = str.replace(new RegExp(String.fromCharCode(8364), "g"), "€");
	return str;
}

/* Helper functions */

// Get an array of submit button nodes contained inside a given node
function tmt_getSubmitNodes(startNode){
	var submitArray = new Array();
	var inputNodes = startNode.getElementsByTagName("input");
	// Get an array of submit nodes
	for(var i=0; i<inputNodes.length; i++){
		if(inputNodes[i].getAttribute("type").toLowerCase() == "submit"){
			submitArray[submitArray.length] = inputNodes[i];
		}
	}
	return submitArray;
}

// Get an array of input and textarea nodes contained inside a given node
function tmt_getTextfieldNodes(startNode){
	var inputsArray = new Array();
	var inputNodes = startNode.getElementsByTagName("input");
	var areaNodes = startNode.getElementsByTagName("textarea");
	// Get an array of text, password and file nodes
	for(var i=0; i<inputNodes.length; i++){
		if(!inputNodes[i].getAttribute("type")){
			inputNodes[i].setAttribute("type", "text");
		}
		var fieldType = inputNodes[i].getAttribute("type").toLowerCase();
		if((fieldType == "text") || (fieldType == "password") || (fieldType == "file") || (fieldType == "hidden")){
			inputsArray[inputsArray.length] = inputNodes[i];
		}
	}
	// Append textarea nodes too
	for(var j=0; j<areaNodes.length; j++){
	    inputsArray[inputsArray.length] = areaNodes[j];
	}
	return inputsArray;
}

// Return an object (sort of an hashtable) containing checkboxes/radios data
// The returned object has two properties:
// name: the group name
// boxes: an array containing the DOM node of each checkbox/radio that share the same name
function tmt_getNodesTable(formNode, type){
	// This object will store data fields, just as an hash table
	var boxHolder = new Object;
	var boxNodes = formNode.getElementsByTagName("input");
	for(var i=0; i<boxNodes.length; i++){
		if(boxNodes[i].getAttribute("type") && (boxNodes[i].getAttribute("type").toLowerCase() == type)){
			// Store the reference to make it easier to read the code
			var boxName = boxNodes[i].name;
			if(boxHolder[boxName]){
				// We already have an entry with the same name
				// Append the DOM node to the relevant entry inside the object
				boxHolder[boxName].elements[boxHolder[boxName].elements.length] = boxNodes[i];
			}
			else{
				// Create a brand new entry inside the object
				boxHolder[boxName] = new Object;
				boxHolder[boxName].name = boxName;
				// Initialize the array that will store all the DOM nodes that share the same name
				boxHolder[boxName].elements = new Array;
				boxHolder[boxName].elements[0] = boxNodes[i];
			}
		}
	}
	return boxHolder;
}

// The function below was developed by John Resig
// For additional info see:
// http://ejohn.org/projects/flexible-javascript-events
// http://www.quirksmode.org/blog/archives/2005/10/_and_the_winner_1.html
function addEvent(obj, type, fn){
	if(obj.addEventListener){
		obj.addEventListener(type, fn, false);
	}
	else if(obj.attachEvent){
		obj["e" + type + fn] = fn;
		obj[type + fn] = function(){
				obj["e" + type + fn](window.event);
			}
		obj.attachEvent("on" +type, obj[type+fn]);
	}
}

addEvent(window, "load", tmt_validatorInit);
