#ifndef  _ADM_HPP
#define  _ADM_HPP

#include <sqlite3.h>
#include <cstring>
#include "adm-base.hpp"

/**
 *   Virtual base class for every sqlite table.
 *   Some of the derived classes:  AdmUser, AdmSession
 */ 

class Adm: public AdmBase {
	
	static Adm* s_ptrRead;	
	static const char* SQL_TABLE_EXIST;
	static const char* SQL_LAST_INSERT;

	
	char tablename[100];
	int numrows=0;
	int lastInsertRowid=-1;
	
	const char* getSQLCreateTable() {return (const char*)0;}; //AdmBase is abstract, son we define these two
	const char* getSQLCreateIndex() {return (const char*)0;};
		
	public:
		Adm()
		{;}
		virtual ~Adm() {
		}
	
				
		static Adm* s_getAdmRead() {return s_ptrRead;}
		static void s_setAdmRead(Adm* p) {Adm::s_ptrRead=p;}

		const char* getTableName() {return tablename;}
		void setName(const char* p) {strncpy(tablename, p,sizeof(tablename)-1); tablename[sizeof(tablename)-1]='\0';}
		void setNummrows(int num) {numrows=num;}
		void incNumrows() {numrows++;}	
		int  getNumrows() {return numrows;}

		/**
		 *  This methos return true if the table exists, falsa otherwise.
		 */
		virtual bool tableExists(sqlite3 *db, const char* name);
				
		int last_insert_rowid(sqlite3 *db);
		
		static int adm_callback_last_insert_row(void *NotUsed, int argc, char **argv, char **szColName);
		
		static int adm_callback(void *NotUsed, int argc, char **argv, char **szColName);
}; 



#endif  //_ADM_HPP
