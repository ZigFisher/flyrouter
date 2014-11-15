function displayError(formNode, validators){
	// Create a <div> that will act as an error display
	var displayNode = document.createElement("div");
	// Create an id that will identify the errors who belongs to this specific form and assign it to the <div>
	var errorId = formNode.id + "displayError";
	displayNode.setAttribute("id", errorId);
	var listNode = document.createElement("ul");
	// Append an entry to the <ul> for each error
	for(var i=0;i<validators.length;i++){
	var listentryNode = document.createElement("li");
		var emNode = document.createElement("em");
		emNode.appendChild(document.createTextNode(validators[i].name));
		listentryNode.appendChild(emNode);
		listentryNode.appendChild(document.createTextNode(": " + validators[i].message));
		listNode.appendChild(listentryNode);
	}
	displayNode.appendChild(listNode);
	var oldDisplay = document.getElementById(errorId);
	// If an error display is already there, we replace it, if not, we create one from scratch 
	if(oldDisplay){
		formNode.parentNode.replaceChild(displayNode, oldDisplay); 
	}
	else{
		formNode.parentNode.insertBefore(displayNode, formNode);
	}
}