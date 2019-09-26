#include <sqlite3.h>
#include <getopt.h>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <array>
#include <list>
#include <assert.h>




#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdint>

#include <openssl/md5.h>

#include "util/rcf-md5.h"
#include "util/base64.h"
#include "adm-user.hpp" 
 


const char* SQL_CREATE_TABLE_USER = "CREATE TABLE User(pkid  INTEGER PRIMARY KEY ASC ,Firstname varchar(50), Login varchar(50), Email varchar(50),Password varchar(50), timestamp INT )";
const char* SQL_USER_INDEX_LOGIN = "CREATE UNIQUE INDEX index_name ON User (Login);";
const char* SQL_DELETE_TABLE_USER = "DROP TABLE IF EXISTS User;";
const char* SQL_INSERT_USER = "INSERT INTO User(Firstname, Login, Email, Password, timestamp) VALUES ('%s', '%s', '%s' ,'%s',CURRENT_TIMESTAMP)";
const char* SQL_UPDATE_USER = "UPDATE USER set Firstname='%s', Email='%s', Password='%s'  WHERE pkid=%d";
const char* SQL_SELECT_USER_BY_ID ="SELECT * FROM User WHERE pkid =%d";
const char* SQL_SELECT_USER_BY_LOGIN ="SELECT * FROM User WHERE Login = '%s'";
//const char* SQL_SELECT_USER_ALL ="SELECT * FROM User";// ORDERED BY Login asc";
const char* SQL_SELECT_USER_ALL ="SELECT pkid, Firstname , Login, Email, Password, timestamp FROM User";
const char* SQL_DELETE_USER ="DELETE FROM User WHERE pkid=%d";






/**
 *  See: https://resources.oreilly.com/examples/9780596521196/blob/master/ch07/ex_ctable.c
 *  
 *  return SQLITE_DONE when ok
 **/

const char* AdmUser::getSQLCreateTable() {return SQL_CREATE_TABLE_USER;}
const char* AdmUser::getSQLCreateIndex() {return SQL_USER_INDEX_LOGIN;}

	
int AdmUser::user_create(sqlite3 *db, const AdmUserDao* admUserDao, long* newPkid )
	{
		char buff_sql[TAM_BUFF_SQL];
		sqlite3_stmt    *stmt = NULL;
		
		snprintf(buff_sql, sizeof(buff_sql), SQL_INSERT_USER, admUserDao->getName(), admUserDao->getLogin(), admUserDao->getEmail(), admUserDao->getPassword());
		
		int rc = sqlite3_prepare_v2( db, buff_sql, -1, &stmt, NULL );
		if ( rc != SQLITE_OK) return rc;

		rc = sqlite3_step( stmt );
		if ( rc != SQLITE_DONE ) return rc;
    
		sqlite3_finalize( stmt );
		Adm adm;
		long pkid=adm.last_insert_rowid(db);
		*newPkid=pkid;

		return rc;
		
	}

int AdmUser::user_update(sqlite3 *db, const AdmUserDao* admUserDao) {
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt *stmt = NULL;

	snprintf(buff_sql, sizeof(buff_sql), SQL_UPDATE_USER, admUserDao->getName(), admUserDao->getEmail(),
			 admUserDao->getPassword(), admUserDao->getPkid());

	AdmBase::dbg("user_update 2: %s ", buff_sql);

	int rc = sqlite3_prepare_v2(db, buff_sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK) return rc;

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) return rc;

    if( stmt!=NULL ) sqlite3_finalize(stmt);

	AdmBase::dbg("user_update ok ");

	return rc;
}


int AdmUser::user_delete(sqlite3 *db, int pkid)
{
		char buff_sql[TAM_BUFF_SQL];
		sqlite3_stmt    *stmt = NULL;
		
		snprintf(buff_sql, sizeof(buff_sql), SQL_DELETE_USER, pkid);
		
		int rc = sqlite3_prepare_v2( db, buff_sql, -1, &stmt, NULL );
		if ( rc != SQLITE_OK) exit( -1 );

		rc = sqlite3_step( stmt );
		if ( rc != SQLITE_DONE ) exit ( -1 );
    
		sqlite3_finalize( stmt );
	
		return rc;
		

}

/***
 * when ok returns: SQLITE_DONE
 ***/ 

int AdmUser::user_delete_table(sqlite3 *db)
{
	sqlite3_stmt    *stmt = NULL;
	
	int rc = sqlite3_prepare_v2( db, SQL_DELETE_TABLE_USER, -1, &stmt, NULL );
	if ( rc != SQLITE_OK) exit( -1 );

	rc = sqlite3_step( stmt );
	if ( rc != SQLITE_DONE ) return rc;

	sqlite3_finalize( stmt );

	return rc;

	/**
	int res = sqlite3_exec(db, SQL_DELETE_TABLE_USER, user_callback, 0, &szErrMsg);
	
	if( res != SQLITE_OK ) {
		setError( szErrMsg );
		sqlite3_free(szErrMsg);
	}
	return res;
	**/
}


int AdmUser::createUser(sqlite3* db, const char* name, const char* login, const char* email, const char* passText )
{
	std::stringstream ss;
	long newPkid=-1;
	
	AdmUser admUser;
	AdmUserDao admU;
	admU.setName(name);
	admU.setLogin(login);
	admU.setEmail(email);
	std::string pass=md5(passText);
	admU.setPassword( AdmBase::string_to_hex(pass).c_str() );
	int res=admUser.user_create(db, &admU, &newPkid);
	ss << "Create name:"<< admU.getName() <<" login:"<<admU.getLogin()<<" \n";	
	ss << "New id:"<< newPkid <<" Result:"<<res<<" \n";	
    AdmBase::dbg(ss.str().c_str());
	return res;
}

/***
 *  Return SQLITE_DONE when ok.
 *  To check if user is found  readAdmUserDao->getPkid()
 * 
 **/

int AdmUser::user_info_by_login(sqlite3 *db, const char* name, AdmUserDao* readAdmUserDao )
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt;
	
	snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_USER_BY_LOGIN, name);
	int rc=-1;
	
	if ( (rc=sqlite3_prepare_v2(db, buff_sql, -1, &ppStmt, NULL)) != SQLITE_OK) {
		AdmBase::dbg("user_info_by_login 3","");
		AdmBase::dbg("Prepare failure: %s",sqlite3_errmsg(db));
		return rc;
	}		

	AdmBase::dbg("user_info_by_login 4","");
	while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		AdmBase::dbg("usr_info_all 5","");
		int numColums=sqlite3_column_count(ppStmt);
		AdmBase::dbg("colums:%d", numColums);
		AdmUser::readRow(readAdmUserDao, ppStmt );
	}
	return rc;
		

}

/**
 *  Returns SQLITE_DONE if ok.
 *  To check if AsmUserDao was found readAdmUserDao->getPkid()
 * 
 **/
	
int AdmUser::user_info_by_id(sqlite3 *db, int id, AdmUserDao* readAdmUserDao )
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt* ppStmt;
	
	snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_USER_BY_ID, id);
	int rc=-1;
	
	if ( (rc=sqlite3_prepare_v2(db, buff_sql, -1, &ppStmt, NULL)) != SQLITE_OK) {
		AdmBase::dbg("user_info_by_id 3","");
		AdmBase::dbg("Prepare failure: %s",sqlite3_errmsg(db));
		return rc;
	}		

	AdmBase::dbg("user_info_by_id 4","");
	while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
		AdmBase::dbg("user_info_by_id 5","");
		int numColums=sqlite3_column_count(ppStmt);
		AdmBase::dbg("colums:%d", numColums);
		AdmUser::readRow(readAdmUserDao, ppStmt );
	}
	if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
	return rc;

}



int AdmUser::usr_info_all(sqlite3 *db, std::list<AdmUserDao*>& listAdmUserDao)
{
		char buff_sql[TAM_BUFF_SQL];

		snprintf(buff_sql, sizeof(buff_sql), SQL_SELECT_USER_ALL,"");
		
		AdmBase::dbg("usr_info_all 0","");
		
		sqlite3_stmt* ppStmt;
		int rc;
		AdmBase::dbg("usr_info_all 1","");
		if ( (rc=sqlite3_prepare_v2(db, SQL_SELECT_USER_ALL, -1, &ppStmt, NULL)) != SQLITE_OK) {
			AdmBase::dbg("usr_info_all 3","");
			AdmBase::dbg("Prepare failure: %s",sqlite3_errmsg(db));
			return rc;
		}		
		
		int numUser=0;
		AdmBase::dbg("usr_info_all 4","");
		while ((rc = sqlite3_step(ppStmt)) == SQLITE_ROW) {
			AdmBase::dbg("usr_info_all 5","");
			int numColums=sqlite3_column_count(ppStmt);
			AdmBase::dbg("colums:%d", numColums);
			AdmUserDao* pUserDao=new AdmUserDao();
			AdmUser::readRow(pUserDao, ppStmt );
			listAdmUserDao.push_front( pUserDao );
			numUser++;
		}
		AdmBase::dbg("usr_info_all 6","");
		if( ppStmt!=NULL ) sqlite3_finalize(ppStmt);
					
		if (rc != SQLITE_DONE) {
			 AdmBase::dbg("Step failure: %s", sqlite3_errmsg(db));
			 return rc;
		}
		AdmBase::dbg("usr_info_all 7");
		return SQLITE_DONE;
		
}	




/**
 * pkid ,Firstname var(50), Login var(50), Email var(50),Password var(50), timestamp INT 
 * Columna are read by position. To read my column name
 * 
 * 
 **/

void AdmUser::readRow(AdmUserDao* pUser, sqlite3_stmt* ppStmt )
{
	assert( sqlite3_column_count(ppStmt)==6);
	pUser->setPkid(sqlite3_column_int(ppStmt,0));
	pUser->setName((const char*)sqlite3_column_text(ppStmt,1));
	pUser->setLogin((const char*)sqlite3_column_text(ppStmt,2)); //sqlite3_column_text16
	pUser->setEmail((const char*)sqlite3_column_text(ppStmt,3));
	pUser->setPassword((const char*)sqlite3_column_text(ppStmt,4));
	pUser->setTimestamp(sqlite3_column_int(ppStmt,5));
}

 
/**** 
void AdmUser::readRow(AdmUserDao* pUser, int argc, char **argv, char **szColName )
{
  for(int i = 0; i < argc; i++) 
  {
	//std::cout<<"  "<<szColName[i]<<" "<<argv[i]<<std::endl;  
	if( strcmp(szColName[i],"Firstname")==0 ){
		pUser->setName(argv[i]);
	}else if( strcmp(szColName[i],"Login") ==0) {
		pUser->setLogin(argv[i]);
	}else if( strcmp(szColName[i],"Email")==0 ) {
		pUser->setEmail(argv[i]);
	}else if( strcmp(szColName[i],"Password")==0 ) {
		pUser->setPassword(argv[i]);
	}else if( strcmp(szColName[i],"pkid")==0 ) {
		pUser->setPkid(atoi(argv[i]));
	}else if( strcmp(szColName[i],"timestamp")==0 ) {
		pUser->setTimestamp(AdmBase::sqlite_time_mill(argv[i]));
	}		
  }
}****/
 














