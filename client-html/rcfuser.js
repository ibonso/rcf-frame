
/*****
  NewUser
 ****/ 

function Create_rcfNewUser(username, login, email, pass)
{
	this.name='user';
	this.url='/fcgi/user/';
	
	this.username=username;
	this.login=login;
	this.email=email;
	this.passmd5=hex_md5(pass);
	
}

Create_rcfNewUser.prototype = inherit(Create_rcfOperation.prototype);
Create_rcfNewUser.prototype.constructor = Create_rcfNewUser;

Create_rcfNewUser.prototype.crearJ1=function() {
		
		var objJ1=new Object();
		objJ1.oper2="new";
		
		var obj = new Object();
		obj.name=this.username;
		obj.login=this.login;
		obj.email=this.email;
		obj.pass=this.passmd5;
		
		console.log('rcfUser.crearJ2 ');
		console.log(obj);
		
		objJ1.new=obj;
		return objJ1;
}

Create_rcfNewUser.prototype.callOperation=function() {
		var jsonJ1=this.crearJ1();
		console.log('NewUser.callOperation J1: '+ JSON.stringify( jsonJ1 ) );
		var jsonJ0=this.createJ0(jsonJ1, rcfAPP.glob_session, rcfAPP.glob_sigop);
		console.log('NewUser.callOperation J0: '+jsonJ0);
		this.callOperationJ0(jsonJ0, this.url );
}


/****
Operation: user
Object(class): Create_rcfUser
****/
        
	function Create_rcfUser(pkid, username, login, email, pass, passNew)
	{
			this.name='user';
			this.url='/fcgi/user/';
			
			if( !pkid ) {
					throw "Create_rcfUser: ERROR NO pkid provided"; 
			}
			
			if( !login ) {
					throw "Create_rcfUser: ERROR NO pkid provided"; 
			}
			
			this.pkid=parseInt(pkid);
			this.username=username;
			this.login=login;
			this.email=email;
			if(!isEmpty(pass) && !isEmpty(passNew))  {
			  console.log('create_rcfUser SI hay password   ');	
			  this.passmd5=hex_md5(pass);
			  this.passmd5New=hex_md5(passNew);
			}else {
			  console.log('create_rcfUser No hay password   ');		
			  this.passmd5='';
			  this.passmd5New='';	
			}
	}
	
	Create_rcfUser.prototype = inherit(Create_rcfOperation.prototype);
	Create_rcfUser.prototype.constructor = Create_rcfUser;

	Create_rcfUser.prototype.crearJ1=function() {
		
		var objJ1=new Object();
		objJ1.oper2="update";
		
		var obj = new Object();
		obj.pkid=this.pkid;
		obj.name=this.username;
		obj.login=this.login;
		obj.email=this.email;
		console.log('rcfUser.crearJ2 ');
		console.log(obj);
		
		
		if(!isEmpty(this.passmd5) && !isEmpty(this.passmd5New)) {
			console.log('Changin password   ');
			obj.pass=this.passmd5;
			obj.passnew=this.passmd5New;
		}else {
			obj.pass='';
			obj.passnew='';
		}
		objJ1.update=obj;
		return objJ1;
	}

	Create_rcfUser.prototype.callOperation=function() {
		var jsonJ1=this.crearJ1();
		console.log('rcfUser.callOperation J1: '+ JSON.stringify( jsonJ1 ) );
		var jsonJ0=this.createJ0(jsonJ1, rcfAPP.glob_session, rcfAPP.glob_sigop);
		console.log('rcfUser.callOperation J0: '+jsonJ0);
		this.callOperationJ0(jsonJ0, this.url );
	}
		
		
/***
   listuser 
 ***/
function Create_rcfListUser()
{
	this.name='user';
	this.url='/fcgi/user/';
}

Create_rcfListUser.prototype = inherit(Create_rcfOperation.prototype);
Create_rcfListUser.prototype.constructor = Create_rcfListUser;

Create_rcfListUser.prototype.crearJ1=function() {
		
		var objJ1=new Object();
		objJ1.oper2="list";
		
		var obj = new Object();
		obj.name="";
		
		console.log('rcfUser.crearJ2 ');
		console.log(obj);
		
		objJ1.list=obj;
		return objJ1;
}

Create_rcfListUser.prototype.callOperation=function() {
		var jsonJ1=this.crearJ1();
		console.log('ListUser.callOperation J1: '+ JSON.stringify( jsonJ1 ) );
		var jsonJ0=this.createJ0(jsonJ1, rcfAPP.glob_session, rcfAPP.glob_sigop);
		console.log('ListUser.callOperation J0: '+jsonJ0);
		this.callOperationJ0(jsonJ0, this.url );
}

