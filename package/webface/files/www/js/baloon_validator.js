
function displayError(formNode, validators){
   BaloonValidator.closeAll();

   for(var elm, i = 0; elm = validators[i]; i++) {
      var node = formNode[elm.name];
      if(node[0]) node = node[0];
      BaloonValidator.open(node, elm.message);
   }
}

var BaloonValidator = {
   index: 0,
   lists: new Array,

   closeAll: function() {
      for(var field, i = 0; obj = this.lists[i]; i++)
         obj.close();
   },

   open: function(field, msg) {
      if(!field._validbaloon) {
         var obj = new this.element(field);
         obj.create();
         field._validbaloon = obj;
         this.lists.push(obj);
      }

      field._validbaloon.show(msg);
   },

   element: function() {
      this.initialize.apply(this, arguments);
   }
};

BaloonValidator.element.prototype = {
   initialize: function(field) {
      this.parent = BaloonValidator;
      this.field = field;
   },

   create: function() {
      var field  = this.field;

      var box = document.createElement('div');
      box.className = 'baloon';

      var offset = this.parent.Position.offset(field);
      var top  = offset.y - 25;
      var left = offset.x - 20 + field.offsetWidth;
      box.style.top  = top +'px';
      box.style.left = left+'px';

      var self = this;
      addEvent(box, 'click', function() { self.toTop(); });

      var link = document.createElement('a');
      link.appendChild(document.createTextNode('X'));
      link.setAttribute('href', 'javascript:void(0);');
      addEvent(link, 'click', function() { self.close(); });

      var msg = document.createElement('span');
      var div = document.createElement('div');
      div.appendChild(link);
      div.appendChild(msg);
      box.appendChild(div);
      document.body.appendChild(box);

      this.box = box;
      this.msg = msg;
   },

   show: function(msg) {
      this.msg.innerHTML  = msg;

      this.box.style.display = '';
      this.toTop();
   },

   close: function() {
      this.box.style.display = 'none';
   },

   toTop: function() {
      this.box.style.zIndex = ++ this.parent.index;
   }
};

BaloonValidator.Position = {
   offset: function(elm) {
      var pos = {};
      pos.x = this.getOffset('Left', elm);
      pos.y = this.getOffset('Top', elm);
      return pos;
   },

   getOffset: function(prop, el) {
      if(!el.offsetParent || el.offsetParent.tagName.toLowerCase() == "body")
         return el['offset'+prop];
      else
         return el['offset'+prop]+ this.getOffset(prop, el.offsetParent);
   }
};

addEvent(window, 'load', function() {
   // preload image
   var obj = new BaloonValidator.element(document.body);
   obj.create();
   document.body.removeChild(obj.box);
});

