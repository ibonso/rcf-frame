
#include <assert.h>
#include <cstdlib>
#include "adm.hpp"


/**
 * General comment
 **/

const char* Adm::SQL_TABLE_EXIST="SELECT name FROM sqlite_master WHERE type='table' AND tbl_name='%s'";
const char* Adm::SQL_LAST_INSERT="SELECT last_insert_rowid()";
Adm* Adm::s_ptrRead;


	
bool Adm::tableExists(sqlite3 *db, const char* name) {
	char buff_sql[TAM_BUFF_SQL];
	char *szErrMsg;
	snprintf(buff_sql, sizeof(buff_sql), Adm::SQL_TABLE_EXIST, name);
	setNummrows(0);
	Adm::s_setAdmRead(this);
	int res=sqlite3_exec(db, buff_sql, Adm::adm_callback, 0, &szErrMsg);
	if( res != SQLITE_OK ) {
		setError( szErrMsg );
		AdmBase::dbg("tableExists ERROR:%S",szErrMsg);
		sqlite3_free(szErrMsg);
		return false;
	}else if( getNumrows()==0) {
		AdmBase::dbg("tableExists ERROR:0","");
		return false;
	}
	return true;
}

		
int Adm::last_insert_rowid(sqlite3 *db)
{
	char* szErrMsg=0;
	Adm::s_setAdmRead(this);	
	int res=sqlite3_exec(db, SQL_LAST_INSERT, adm_callback_last_insert_row, 0, &szErrMsg);
	if( res != SQLITE_OK ) {
		setError( szErrMsg );
		sqlite3_free(szErrMsg);
		return res;
	}
	//OK: return last inserted row id
	return this->lastInsertRowid;
	
	
}
		
int Adm::adm_callback_last_insert_row(void *NotUsed, int argc, char **argv, char **szColName)
{
  Adm* pAdm=Adm::s_getAdmRead();	
  assert (pAdm!=NULL);
  //AdmBase::dbg("adm_callback_last_insert_row" );
  for(int i = 0; i < argc; i++) 
  {
	if( strcmp(szColName[i],"last_insert_rowid()")==0 ){
		pAdm->lastInsertRowid=atoi(argv[i]);
		//AdmBase::dbg("LAST INSERT %d", pAdm->lastInsertRowid);
	}		
	pAdm->incNumrows();
  }	
  
  return 0;
}

int Adm::adm_callback(void *NotUsed, int argc, char **argv, char **szColName)
{
  Adm* pUser=Adm::s_getAdmRead();	
  assert (pUser!=NULL);
  for(int i = 0; i < argc; i++) 
  {
	if( strcmp(szColName[i],"name")==0 ){
		pUser->setName(argv[i]);
	}		
	pUser->incNumrows();
  }	
  
  return 0;
}




