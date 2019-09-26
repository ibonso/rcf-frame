

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>

#include <sqlite3.h>

#include "../rcf-bbb-01/rapidjson/document.h"
#include "../rcf-bbb-01/rapidjson/writer.h"
#include "../rcf-bbb-01/rapidjson/stringbuffer.h"

#include "./util/base64.h"
#include "rcf-op-ping.hpp"
#include "rcf-op-login.hpp"
#include "rcf-op-logout.hpp"
#include "rcf-op-lsdir.hpp"
#include "rcf-op-sessionuser.hpp"
#include "rcf-op-user.hpp"
#include "rcf-op-command.hpp"
#include "rcf-op-service.hpp"
#include "rcf-op-blue.hpp"




/**
 Create db and the tables session, user
**/
int create_tables(sqlite3** db, const char* dbName)
{
	int res;
	res = sqlite3_open(dbName, db);
	if( res!= SQLITE_OK ) {
			return res;
	}
	
	AdmSession admSession;
	res = admSession.create_table_index(*db);
	AdmUser admUser;
	res = admUser.create_table_index(*db);
	
	if(db) {
		res =sqlite3_close(*db);

	}
	return res;
}



int syslog_level=LOG_ERR;

/** param example form login
 *  
  
  param: path de la llamada url
  msgin: el mensaje de entrada. Codificado en base 64. Tiene que ser un json
  
  <root>/<opername>/<b64_json_msg_in>
   
 /fcgi/login/eyJuYW1lIjoibG9naW4iLCJsb2dpbiI6eyJpZCI6ImFkbSIsInBhc3MiOiJiMDljNjAwZmRkYzU3M2YxMTc0NDliMzcyM2YyM2Q2NCJ9fQrcf_proxy 0 
 Path: login/eyJuYW1lIjoibG9naW4iLCJsb2dpbiI6eyJpZCI6ImFkbSIsInBhc3MiOiJiMDljNjAwZmRkYzU3M2YxMTc0NDliMzcyM2YyM2Q2NCJ9fQ

 * 
 **/ 

extern "C" char* rcf_proxy(const char* param, const char* msgin, int argc, char* argv[])
{
	char dbName[200];
	
	bool bCrearUsuario=false;	
	char userLogin[100];
	char userPass[200];
	
	AdmBase::dbg(" rcf_proxy Param: %s\n",param);
	
	if( argc>1 ) { //Database name
		fprintf(stderr, "Parameter database: %s\n", argv[1]);	
		strncpy(dbName,argv[1],199);
	}
	
	//Admin usr and password to be created
	if(argc>3 && strlen(argv[2])>=3 && strlen(argv[3])>=3 ) {
		fprintf(stderr, "Parameters: %d\n",argc);	
		strncpy(userLogin,argv[2],99);
		strncpy(userPass,argv[3],99);
		AdmBase::dbg("rcf_proxy parameters usuario/pass: %s/%s ",userLogin, userPass);
		bCrearUsuario=true;
	}	
	
	if( !AdmBase::file_exists( dbName ) ) {
		AdmBase::dbg(LOG_INFO ,"INFO Database file %s doesnt exist. Will be created now", dbName);
		sqlite3* dbx;
		int resx=create_tables(&dbx, dbName);
		AdmBase::dbg(LOG_DEBUG, "rcf_proxy: create_tables res: %d",resx);
		
		if(dbx) {
			int resClose=sqlite3_close(dbx);
			AdmBase::dbg(LOG_INFO, "rcf_proxy: create_tables close: %d",resClose);
		}
	}
	
	sqlite3 *db;
	int rc = sqlite3_open(dbName, &db);
	
	if(rc != SQLITE_OK) {
		AdmBase::dbg(LOG_ERR, "Can't open database_ :%s", dbName);
		char *ptrError=(char *)malloc(300);
		sprintf(ptrError,"ERROR: %d . Cant open database: %s ", rc, dbName);
		return ptrError;
	} else {
		AdmBase::dbg( LOG_DEBUG, "Database opened  successfully:%s", dbName);
	}
	
	if( bCrearUsuario ) {
		AdmBase::dbg( LOG_DEBUG, "rcf_proxy: creando usuario: %s",userLogin);

		AdmUserDao admUserDao;
		AdmUser admUser;

		int resExiste=admUser.user_info_by_login(db, userLogin, &admUserDao );
		if(resExiste == SQLITE_DONE &&  admUserDao.getPkid()<1)  {
		//No existe el usuario, se crea
			int res=admUser.createUser( db, userLogin, userPass, "nomail@nono.com", userPass );
			assert(res==SQLITE_DONE);
			AdmBase::dbg(LOG_DEBUG, "rcf_proxy: creando usuario, res: %d",res);
		} else {
			AdmBase::dbg(LOG_DEBUG, "rcf_proxy: ya existe usuario: %s",userLogin);
		}

	}
	
	AdmBase::dbg(LOG_DEBUG, "rcf_proxy 0_ ");
	
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	
	char* ptr_respuesta;
	
	std::stringstream strError;
	rapidjson::Document rapid_login;
	
	std::string msgindecoded=base64_decode_2(std::string(msgin));

    AdmBase::dbg("rcf_proxy Before Parse (b64encode): %s",  msgin);
    int resParse=createJson(rapid_login, msgindecoded.c_str(), strError  );
    
	if( resParse!=0 ) {
	//json parser error
		DBGX("Parse done with error %s",strError.str().c_str());
		ptr_respuesta=(char*)malloc(514);
		sprintf(ptr_respuesta, "Json not correct %s", __TIMESTAMP__ );
		DBGX("Parse done with error 3","");
	} else { 
	//json objecto ok
		rcfOp* operation=nullptr;
		std::string respOperation;

		if(0==(param-strstr((char*)param, rcfOpPing::getOperName()))) { //ping
			operation = (rcfOp*) new rcfOpPing(db, rapid_login);
		}else if( 0==(param-strstr((char*)param, rcfOpLsDir::getOperName()))) { // lsdir
			operation = (rcfOp*) new rcfOpLsDir(db, rapid_login);
		}else if ( 0==(param-strstr(param, rcfOpLogin::getOperName())) ){ //login
			operation = (rcfOp*) new rcfOpLogin(db, rapid_login);
		}else if ( 0==(param-strstr(param, rcfOpLogout::getOperName())) ) {	//logout
			operation = (rcfOp*) new rcfOpLogout(db, rapid_login);
        }else if(0==(param-strstr(param, rcfOpSessionUser::getOperName()))) { //sessionuser
            operation = (rcfOp*) new rcfOpSessionUser(db, rapid_login);
		}else if ( 0==(param-strstr(param, rcfOpUser::getOperName())) ) {    //user
            operation = (rcfOp*) new rcfOpUser(db, rapid_login);

		}else if ( 0==(param-strstr(param, rcfOpCommand::getOperName())) ) {	//command
			operation = (rcfOp*) new rcfOpCommand(db, rapid_login);
		}else if ( 0==(param-strstr(param, rcfOpService::getOperName())) ) {	//service
			operation = (rcfOp*) new rcfOpService(db, rapid_login);
		}else if ( 0==(param-strstr(param, rcfOpBlue::getOperName())) ) {	//blue
					operation = (rcfOp*) new rcfOpBlue(db, rapid_login);
		}else {
			const char* msgError="<html><head></head><body>URL operation not defined in proxy.<p>%s <body></html>";
			ptr_respuesta=(char*)malloc(sizeof(msgError)+201);
			sprintf(ptr_respuesta, msgError, __TIMESTAMP__ );
		}

		if( operation!=nullptr ) {
			respOperation = operation->dowork();
			//AdmBase::dbg("rcf_proxy: resp logout:%s", respOperation.c_str());
			ptr_respuesta=(char*)malloc(respOperation.size()+1);
			sprintf(ptr_respuesta,"%s", respOperation.c_str() );
		}

		AdmBase::dbg(LOG_DEBUG,"rcf_proxy, resp size: %ul", respOperation.size() );
		syslog(LOG_DEBUG,"rcf_proxy, resp size: %ul", (unsigned int)respOperation.size());
		//Merece la pena new delete?? que se gana?
		if(operation!=nullptr) {
			delete operation;
		}

	}
	
	// close database
	if(db) {
		int resClose=sqlite3_close(db);
		AdmBase::dbg( "rcf_proxy CLOSE DB: %d",resClose);
		if(resClose== SQLITE_BUSY ) {
			AdmBase::dbg( "rcf_proxy Close: SQLITE_BUSY",resClose);
		}
	}

	return (char*)ptr_respuesta;
}


/***
 * 
 *  Get full rute and split operation, json 
 */
void splitParameters(const char* str, std::string operacion, std::string json )
{
	
	
	
	
	
}


