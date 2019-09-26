
#include "adm-base.hpp"

#include <sstream>
#include <iostream>     // std::cout, std::endl
#include <iomanip>      // std::setfill, std::setw
#include <cstdarg> 
#include <cstdio>
#include <ctime>
#include <time.h>
#include <syslog.h>

//#include <random>
#include <stdlib.h>


	
int AdmBase::create_table_index(sqlite3 *db)
{
	char buff_sql[TAM_BUFF_SQL];
	sqlite3_stmt    *stmt = NULL;
	
	snprintf(buff_sql, sizeof(buff_sql), getSQLCreateTable(), "");
		
	int rc = sqlite3_prepare_v2( db, buff_sql, -1, &stmt, NULL );
	
	if ( rc != SQLITE_OK) return rc;

	rc = sqlite3_step( stmt );
	if ( rc != SQLITE_DONE ) return rc;
	
	sqlite3_finalize( stmt );
	snprintf(buff_sql, sizeof(buff_sql), getSQLCreateIndex(), "");
	int rc2 = sqlite3_prepare_v2( db, buff_sql, -1, &stmt, NULL );
	if ( rc2 != SQLITE_OK) return rc2;
	rc2 = sqlite3_step( stmt );
	if ( rc2 != SQLITE_DONE ) return rc2;
	sqlite3_finalize( stmt );
	
	return rc2;

}

	

	
	/** 
	 * Convert byte string to hexadecimal representation 
	 *  md5 pass is stored this way in bd
	 * https://stackoverflow.com/questions/3381614/c-convert-string-to-hexadecimal-and-vice-versa#3382894
	 ***/
	std::string AdmBase::string_to_hex( const std::string& in ) 
	{
		std::stringstream ss;
		ss << std::hex << std::uppercase << std::setfill('0');
		for (size_t i = 0; i<in.length() ; ++i) {
			ss << std::setw(2)<< static_cast<unsigned int>(static_cast<unsigned char>(in[i]));
		}
		return ss.str(); 
	}
	
	void AdmBase::dbgx(const std::string& s, const char* file, int line , ...) 
	{
		char buff[200];
		snprintf(buff, 200,"%s line:%d ", file, line);
		
		std::string sFormat(buff);
		
		std::string snew(sFormat+s);
		va_list args;
		va_start(args, line);
		dbg(snew, args);
	}


	void AdmBase::dbgv(int level, const char* s, va_list args)
	{
		vsyslog(level, s, args);
	}

	void AdmBase::dbg(int level, std::string s,...)
	{
		va_list args;
		va_start(args, s.c_str());
		dbgv(level, s.c_str(), args);
		va_end(args);
	}


	void AdmBase::dbg(std::string s,...)
	{
		va_list args;
		va_start(args, s.c_str());
		dbgv(LOG_DEBUG, s.c_str(), args);
		va_end(args);
	}
	
	void AdmBase::dbg(const char* pattern,...)
	{
		va_list args;
		va_start(args, pattern);
		dbgv(LOG_DEBUG, pattern, args);
		va_end(args);
	}

	
	/**
	 * 3 digit  max random number
	 */
	int AdmBase::getramdom(int min,int max)
	{
		srand ( time(NULL) );
		int int_rand=rand();
		return int_rand%1000;
	}
	
	/***
	 *  1: file pathDb exists
	 *  0: it does not exist
	 **/ 
	int AdmBase::file_exists( const char* pathDb  )
	{
		//https://stackoverflow.com/questions/13945341/c-check-if-a-file-exists-without-being-able-to-read-write-possible
		struct stat st;
		int result = stat(pathDb, &st);
		if( result == 0 ) {
				AdmBase::dbg( "file_exists %s", pathDb);
				return 1;
		}
		AdmBase::dbg( "file_exists NOT %s", pathDb);
		return 0;
	}

	
	/**
	 *  What we get from sqlite: 2018-09-14 19:50:16
	 * 
	 **/
	int AdmBase::sqlite_time_mill(const char* buff)
	{
		struct tm time;
		strptime(buff, "%Y-%m-%d %H:%M:%S", &time);
		time_t loctime = mktime(&time);  // timestamp in current timezone
		//time_t gmttime = timegm(&time);  // timestamp in GMT
		return loctime;
	}

	void AdmBase::mill_time_buff( char* buff, size_t size, time_t time)
	{
		const char* format="%Y-%m-%d %H:%M:%S";
		const struct tm* timeptr;
		//timeptr = gmtime (&time); 
		timeptr = localtime (&time); 
		strftime (buff, size, format,   timeptr );
	}

	int AdmBase::getTimeNowSec()
	{
		time_t now;
		now = time(0);
		return (int) now;
	}




