/***
 rcf-frame javascript operations
 
        C++            javascript
        
rcfOp	                    (Create_rcfOperation)
  |
  |---- rcfOpPing      		(Create_rcfPing)
  |---- rcfOpLogin          (Create_rcfLogin) 
  |---- rcfOpLogout         (Create_rcfLogout)
  |---- rcfOpLsDir          (Create_rcfLsdir)
  |-----rcfSessionUser      (Create_rcfSessionUser) 
  |-----rcfOpUser           (Create_rcfUser) 
  |-----rcfOpCommand        (Create_rcfCommand)
  |-----rcfOpService        (Create_rcfService)
  |-----rcfOpBlue           (Create_rcfServBlue)
 
***/

/***
Common to all operations	
Objecto oriented js
  https://scotch.io/tutorials/object-oriented-programming-in-javascript
***/
// inherit() returns a newly created object that inherits properties from the
// prototype object p. It uses the ECMAScript 5 function Object.create() if
// it is defined, and otherwise falls back to an older technique.
function inherit(p) {
	if (p == null) throw TypeError(); // p must be a non-null object
	if (Object.create)	// If Object.create() is defined...
		return Object.create(p);
	
	var t = typeof p;
	// Otherwise do some more type checking
	if (t !== "object" && t !== "function") 
		throw TypeError();
	function f() {};	// Define a dummy constructor function.
	f.prototype = p;	// Set its prototype property to p.
	return new f();		// Use f() to create an "heir" of p.
}


/****
 Operation: operation
 Object(class): Create_rcfOperation
****/	

function Create_rcfOperation(nombre, bSession )
{
		console.log('create_rcfOperation--CONSTRUCTOR: '+nombre);
		this.name=name;
		this.name2=nombre;
		this.session=bSession;
		this.resJ0='x';
}

        
/**
	Return J0 in string mode.(will change to obj)
**/
Create_rcfOperation.prototype.createJ0 = function(J1, session, sigop) {
	
		//Check this.name exists and has value!!
		if ( this.hasOwnProperty('name') ) {
			console.log('createJ0 OK, THERE IS NAME: '+this.name);
			console.log('createJ0 J1: ');
			console.log(J1);
		}else {
			console.log('createJ0 ERROR, THERE IS NO NAME: '+this.name);
			throw "createJ0: Error no name";
		}
	
		//crear J0 e insertar los datos relevantes
		var obj = new Object();
		obj.name = this.name; //elemento name:'<nombreop>'
		
		if( session==undefined ) {
			console.log('createJ0 no session jet:'+session);
			console.log('createJ0 sigop:'+sigop);
			
		}else {
			console.log('createJ0 session:'+session);
			console.log('createJ0 sigop:'+sigop);
			
			obj.session = session;
			obj.sigop = Number(sigop);
			
		}
		
		obj[this.name] = J1; // <nombreop>:{ J1 }
		//console.log('rcfOperation.createJ0 J1:'+ JSON.stringify(J1));
		console.log('rcfOperation.createJ0 J1:'+ J1.toString());
		
		return obj; //JSON.stringify(obj);
 };

 /**	
	 j0Obj: object  J0

 **/
 Create_rcfOperation.prototype.callOperationJ0 = function (j0Obj, url) {
	 
		var jCompleto=JSON.stringify(j0Obj);
		console.log('J0 a enviar:'+  jCompleto);
		
		document.getElementById("send_txt").innerHTML=jCompleto;
		
		var params = 'rcf='+rstr2b64(jCompleto);
		console.log(" PARAM: "+params);
		
		rcfAPP.glob_operation=JSON.parse(jCompleto)['name'];
		console.log(" OPERATION1: "+j0Obj['name']);
		console.log(" OPERATION2: "+rcfAPP.glob_operation);
		
		rcfAPP.glob_J0=null;	
		rcfAPP.glob_J1=null;                        
		
		window.rcf_ajax_post(url, this.response, this.noresponse ,params);
		console.log('callOperationJ0:'+this.name+ " session"+ this.session);
 };
		
 
 
 Create_rcfOperation.prototype.response = function(data) {
	   var opername=data["name"];	
	   console.log('RESPUESTA_x hecho :'+data);
	   console.log('NAME: hecho :'+opername);
	   this.resJ0=data;	
       rcf_response(data);
 };

Create_rcfOperation.prototype.noresponse = function(httpstatus) {
	console.log('rcfoperation.js Create_rcfOperation noresponse: '+httpstatus);
	rcf_noresponse(data);
}


/***
 rcfOpPing ping operation
***/

function Create_rcfPing(  )
{
	var name='ping';
	this.name=name;
	this.url='/fcgi/ping/';
	this.j1=''; //Toda la cadena J1
	this.j0=''; //La cadena J0
}

// Make Create_rcfPing a subclass of Create_rcfOperation:
// Only share function/methods, so no no data!We need to replicate properties en each constructor!
Create_rcfPing.prototype = inherit(Create_rcfOperation.prototype);
Create_rcfPing.prototype.constructor = Create_rcfPing;

Create_rcfPing.prototype.crearJ1=function() {
		console.log('rcfPing.crearJ1: ' );
		var obj = new Object();
		obj.type = "rcf"; 
		console.log( obj );
		return obj;
}

Create_rcfPing.prototype.callOperation=function() {
		console.log('rcfPing.callOperation J0: ' );
		
		var jsonJ1=this.crearJ1();
		console.log('rcfPing.callOperation J1: '+ jsonJ1 );
		console.log(jsonJ1 );
		
		var jsonJ0=this.createJ0(jsonJ1, rcfAPP.glob_session, rcfAPP.glob_sigop);
		console.log('rcfPing.callOperation J2: '+ JSON.stringify(jsonJ0) );
		this.callOperationJ0(jsonJ0, this.url );
}



Create_rcfPing.rcf_op_ping= function () { 
	console.log('rcf_op_ping ');	
	const opPing = new Create_rcfPing();
	console.log('rcf_op_ping 2');
	opPing.callOperation();
	console.log('rcf_op_ping 3');
}	







