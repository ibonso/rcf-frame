 
#ifndef _ADM_BASE_HPP
#define _ADM_BASE_HPP

#include <sstream>
#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setfill, std::setw
#include <cstdarg> 
#include <cstdio>
#include <ctime>
#include <time.h>
#include <random>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sqlite3.h>


#define	 DBGX(s, ...) \
		 AdmBase::dbgx(s,__FILE__, __LINE__, __VA_ARGS__); 


#define TAM_BUFF_SQL 500
#define TAM_BUFF_LOG 5000*4
#define RCF_TIMEOUT 5*60*1000

/**
 *   Groups some static functions usefull to deal with sqlite
 */

class AdmBase {
	
	protected:
	std::string sErrorDb=""; //Keep db error after calling sqlite3_free(szErrMsg);
	
	void setError(const char* szEr) {szEr!=0?sErrorDb=szEr:sErrorDb="_e_";}
		
	virtual const char* getSQLCreateTable()=0;
	virtual const char* getSQLCreateIndex()=0;
		
		
	public:
	
	virtual ~AdmBase() {}
	
	int create_table_index(sqlite3 *db);
	
	std::string getError() { return sErrorDb; }
	
	/** 
	 * Convert byte string to hexadecimal representation 
	 *  md5 pass is stored this way in bd
	 * https://stackoverflow.com/questions/3381614/c-convert-string-to-hexadecimal-and-vice-versa#3382894
	 ***/
	static std::string string_to_hex( const std::string& in ); 
	
	static void dbgx(const std::string& s, const char* file, int line , ...); 
	


	static void dbgv(int level, const char* s, va_list args);

	static void dbg(int level, std::string s,...);

	static void dbg(std::string s,...);

	
	static void dbg(const char* pattern,...);
	
	static int getramdom(int min,int max);
	
	static int file_exists( const char* pathDb );
	
	/**
	 *  What we get from sqlite: 2018-09-14 19:50:16
	 **/
	static int sqlite_time_mill(const char* buff);
	
	static void mill_time_buff( char* buff, size_t size, time_t time);
	
	static int getTimeNowSec();
		
	
};


#endif //_ADM_BASE_HPP

