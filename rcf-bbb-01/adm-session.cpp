#include <sqlite3.h>
#include <getopt.h>
#include <iostream>

#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <array>
#include <list>
#include <assert.h>

#include <openssl/md5.h>    

#include "adm-session.hpp"
#include "adm.hpp"



const char* SQL_CREATE_TABLE_SESSION = "CREATE TABLE Session(pkid  INTEGER PRIMARY KEY ASC , id varchar(50), sigop INT, id_user INT, state INT , lastlogin INT, ip varchar(40), useragent varchar(150) )";
const char* SQL_CREATE_INDEX_LOGIN = "CREATE UNIQUE INDEX SessionIndex ON Session (id)";
const char* SQL_DELETE_TABLE = "DROP TABLE IF EXISTS Session";
const char* SQL_INSERT_SESSION = "INSERT INTO Session(id, sigop, id_user, state, lastlogin, ip, useragent) VALUES ('%s', %d, %d, %d, %d, '%s' ,'%s')"; //CURRENT_TIMESTAMP
const char* SQL_UPDATE_SESSION = "UPDATE Session set sigop=%d , state=%d  WHERE id='%s'";
const char* SQL_DELETE_SESSION_ID ="DELETE FROM Session WHERE id='%s'";


const char* SQL_SELECT_SESSIO_BY_PKID="SELECT * FROM Session WHERE pkid ='%d'";
const char* SQL_SELECT_SESSION_BY_ID ="SELECT * FROM Session WHERE id ='%s'";
const char* SQL_SELECT_USER_BY_PKID = "SELECT * FROM Session WHERE pkid = %d";
const char* SQL_SELECT_SESSIO_BY_STATE = "SELECT * FROM Session WHERE state = %d";
const char* SQL_SELECT_SESSION_BY_USER = "SELECT * FROM Session WHERE id_user = %d";
const char* SQL_SELECT_SESSION_BY_SESSIONID_SIGOP = "SELECT * FROM Session WHERE id = '%s' and sigop=%d and state=0";
const char* SQL_DELETE_SESSION ="DELETE FROM Session WHERE pkid=%d";

/** Tables: SERVICIO, SESSION_SERVICIO
const char* SQL_CREATE_TABLE_SERVICIO="CREATE TABLE Servicio (pkid servicio INTEGER PRIMARY KEY ASC, nombre varchar(50), descripcion varchar(150))";
const char* SQL_CREATE_TABLE_SESSION_SERVICIO="CREATE TABLE SessionServicio (id_session INT, id_servicio INT, timestamp INT)";
const char* SQL_CREATE_SERVICIO="INSERT INTO Servicio( nombre, descripcion) VALUES ('%s', %s )";
const char* SQL_CREATE_SESSION_SERVICIO="INSERT INTO SessionServicio( id_session, id_servicio, timestamp) VALUES ('%s', %s )";
***/

void AdmSession::readRow(AdmSessionDao* sessionDao, sqlite3_stmt* ppStmt )
{
	assert( sqlite3_column_count(ppStmt)==8);
	sessionDao->setPkid(sqlite3_column_int(ppStmt,0));
	sessionDao->setId((const char*)sqlite3_column_text(ppStmt,1));
	sessionDao->setSigop(sqlite3_column_int(ppStmt,2)); 
	sessionDao->setIdusuario(sqlite3_column_int(ppStmt,3));
	
	int state=sqlite3_column_int(ppStmt,4);
	sessionDao->setState((session_state)state);

	int timeStampX=sqlite3_column_int(ppStmt,5);


    Adm::dbg("AdmSession timeStampX leido: %s", (const char*)sqlite3_column_text(ppStmt,5));
	Adm::dbg("AdmSession timeStampX leido: %d", timeStampX);
	sessionDao->setTimestamp(timeStampX);
	sessionDao->setIp((const char*)sqlite3_column_text(ppStmt,6));
	sessionDao->setUseragent((const char*)sqlite3_column_text(ppStmt,7));
    Adm::dbg("AdmSession timeStampX leido fin: ");
}





const char* AdmSession::getSQLCreateTable() {return SQL_CREATE_TABLE_SESSION;}
const char* AdmSession::getSQLCreateIndex() {return SQL_CREATE_INDEX_LOGIN;}
 

/**
 *  session_update
 **/ 

int AdmSession::session_update(sqlite3 *db, const AdmSessionDao* sessionDao )
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;
	
	Adm::dbg("AdmSession::session_update 0");
	
	snprintf(buff_sql, sizeof(buff_sql), SQL_UPDATE_SESSION, sessionDao->getSigop(),sessionDao->getState(), sessionDao->getId() );
	int rc = sqlite3_prepare_v2( db, buff_sql, -1, &ppStmt, NULL );
	if ( rc != SQLITE_OK) return rc;
	
	if ( (rc = sqlite3_step( ppStmt ))!= SQLITE_DONE ) return rc;
	
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	
	return rc;
}

int AdmSession::session_delete_by_id(sqlite3 *db, const char * idSession)
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;

    Adm::dbg("AdmSession::session_delete_by_id ");

	snprintf(buff_sql, sizeof(buff_sql), SQL_DELETE_SESSION_ID, idSession );

    Adm::dbg("AdmSession::session_delete_by_id sql: %s", buff_sql);

	int rc = sqlite3_prepare_v2( db, buff_sql, -1, &ppStmt, NULL );
	
	if ( rc != SQLITE_OK) return rc;
	
	if ( (rc = sqlite3_step( ppStmt ))!= SQLITE_DONE ) return rc;
	
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	
	return rc;
}


int AdmSession::session_delete_table(sqlite3 *db)
{
	sqlite3_stmt* ppStmt=0;
	int rc = sqlite3_prepare_v2( db, SQL_DELETE_TABLE, -1, &ppStmt, NULL );
	if ( rc != SQLITE_OK) return rc;
	if ( (rc = sqlite3_step( ppStmt ))!= SQLITE_DONE ) return rc;
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	return rc;
}

/**
 *  AdmSession member have to be initialized. This method creates a new row in Session
 *  with that information.
 *  return SQLITE_DONE if ok
 *  
 */
int AdmSession::session_create(sqlite3 *db, const AdmSessionDao* sessionDao, long* newPkid )
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;

	snprintf(buff_sql, sizeof(buff_sql), SQL_INSERT_SESSION, sessionDao->getId(), sessionDao->getSigop(),sessionDao->getIdusuario(), sessionDao->getState(), 
	sessionDao->getTimestamp(), sessionDao->getIp(), sessionDao->getUseragent());
	
	int rc = sqlite3_prepare_v2( db, buff_sql, -1, &ppStmt, NULL );
	if ( rc != SQLITE_OK) return rc;

	rc = sqlite3_step( ppStmt );
	if ( rc != SQLITE_DONE ) return rc;

	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);

	Adm adm;
	long pkid=adm.last_insert_rowid(db);
	*newPkid=pkid;
	
	return rc;
} 

int AdmSession::session_info_by_pkid(sqlite3 *db, long pkid, AdmSessionDao* sessionDao)
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;
	char* szErrMsg=0; 
		
	snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_SESSIO_BY_PKID, pkid);
	int rc=-1;
	
	if ( (rc=sqlite3_prepare_v2(db, buff_sql, -1, &ppStmt, NULL)) != SQLITE_OK) {
		setError( szErrMsg );
		return rc;
	}
	while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		sqlite3_column_count(ppStmt);
		AdmSession::readRow(sessionDao, ppStmt );
	}
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	return rc;
	
}


/**
 *  Returns session witht state idsession
 * 
 **/
int AdmSession::session_info_by_state(sqlite3 *db, long state, AdmSessionDao* sessionDao )
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;
	char* szErrMsg=0; 
		
	snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_SESSIO_BY_STATE, state);
	int rc=-1;
	
	if ( (rc=sqlite3_prepare_v2(db, buff_sql, -1, &ppStmt, NULL)) != SQLITE_OK) {
		setError( szErrMsg );
		return rc;
	}
	while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		sqlite3_column_count(ppStmt);
		AdmSession::readRow(sessionDao, ppStmt );
	}
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	return rc;
}

	
	
int AdmSession::session_info_by_id(sqlite3 *db, const char* idsession, AdmSessionDao* sessionDao)
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;
	char* szErrMsg=0;
	
	int rc;
	
	snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_SESSION_BY_ID, idsession);
    Adm::dbg("session_info_by_id: %s ", buff_sql);


	if ( (rc=sqlite3_prepare_v2(db, buff_sql, -1, &ppStmt, NULL)) != SQLITE_OK) {
		setError( szErrMsg );
		return rc;
	}
	while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		sqlite3_column_count(ppStmt); //return numcolum
		AdmSession::readRow(sessionDao, ppStmt );
	}
    Adm::dbg("session_info_by_id: redrows ");
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
    Adm::dbg("session_info_by_id: %d ", rc );
	return rc;
	
}


int AdmSession::session_info_by_iduser(sqlite3 *db, long iduser, AdmSessionDao* sessionDao)
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;
	char* szErrMsg=0;
	
	int rc;
	
	snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_SESSION_BY_USER, iduser);
	if ( (rc=sqlite3_prepare_v2(db, buff_sql, -1, &ppStmt, NULL)) != SQLITE_OK) {
		setError( szErrMsg );
		return rc;
	}

	while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		sqlite3_column_count(ppStmt);
		AdmSession::readRow(sessionDao, ppStmt );
	}
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	return rc;
	
}

/**
 *  check 'sessionid' session in bd
 *
 */
int AdmSession::session_info_by_id_sigop( sqlite3 *db, const char* sessionid, int sigop, AdmSessionDao* sessionDao )
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt=0;
	char* szErrMsg=0;
	int rc;
	
	snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_SESSION_BY_SESSIONID_SIGOP, sessionid, sigop);
	AdmBase::dbg("TEST: AdmSession::session_info_by_id_sigop: %s", buff_sql);


	if ( (rc=sqlite3_prepare_v2(db, buff_sql, -1, &ppStmt, NULL)) != SQLITE_OK) {
		setError( szErrMsg );
		return rc;
	}
	AdmBase::dbg("AdmSession::session_info_by_id_sigop prepare ok");
	while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		sqlite3_column_count(ppStmt);
		AdmSession::readRow(sessionDao, ppStmt );
	}
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	AdmBase::dbg("AdmSessionDao leido:%s",sessionDao->toString());
	AdmBase::dbg("AdmSession::session_info_by_id_sigop rc:%d",rc);
	return rc;
}


std::string AdmSessionDao::printList( std::list<AdmSessionDao*> listResult )
{
	std::stringstream s;
	std::list<AdmSessionDao*>::iterator it=listResult.begin();
	while(it!=listResult.end())
	{
			s<<((*it)->toString());
			s<<"\n";
			++it;
	}	
	return s.str();
}


std::string AdmSessionDao::toString() const
{
	std::stringstream salida;
	salida<<"pkid:"<<getPkid()<<std::endl;
	salida<<"id:"<<getId()<<std::endl;
	salida<<"sigop:"<<getSigop()<<std::endl;
	salida<<"id_usuario:"<<getIdusuario()<<std::endl;
	salida<<"state:"<<getState()<<std::endl;
	salida<<"timestamp:"<<getTimestamp()<<std::endl;
	salida<<"ip:"<<getIp()<<std::endl;
	salida<<"useragent:"<<getUseragent()<<std::endl;
	
	return salida.str();
}










