#include <sqlite3.h>
#include <getopt.h>
#include <iostream>

#include <stdio.h>
#include <cstring>
#include <string.h>
#include <string>
#include <array>
#include <list>
#include <assert.h>

#include <openssl/md5.h>

#include "adm-base.hpp"

#ifndef _ADM_SESSION_HPP_
#define _ADM_SESSION_HPP_




enum session_state {NOINIT=0, INIT = 1, LOGIN = 2, TIMEOUT = 3, LOGOUT = 4};


class AdmSessionDao
{
	long pkid;
	char id[50]; //session identifier
	int sigop;
	int idusuario;
	session_state state;
	long  timestamp;
	char ip[50];
	char useragent[50];
		
	public:
	
	AdmSessionDao() {
		AdmBase::dbg("AdmSessionDao contructor");
		empty();
	};
	
	AdmSessionDao(const AdmSessionDao& otro) {
		AdmBase::dbg("AdmSessionDao contructor copia");
		this->operator=(otro);
		AdmBase::dbg("AdmSessionDao contructor copia fin");
	}
	
	void empty()
	{
		pkid=-1;
		id[0]='\0';
		sigop=0;
		idusuario=-1;
		state=NOINIT;
		time_t now = time(0);
		localtime(&now);
		timestamp=now;
		ip[0]='\0';
		useragent[0]='\0';
	}
	
	AdmSessionDao& operator=(const AdmSessionDao other) {
		AdmBase::dbg("AdmSessionDao operator copia");
		if( this != &other ) {
			setPkid(other.getPkid());
			setId(other.getId());
			setSigop(other.getSigop());
			setIdusuario(other.getIdusuario());
			setState(other.getState());
			setTimestamp(other.getTimestamp());
			setIp(other.getIp());
			setUseragent(other.getUseragent());
		}
		AdmBase::dbg("AdmSessionDao operator copia fin");
		return *this;
	}
	
	bool operator==(const AdmSessionDao& other) {
			if( pkid!=other.getPkid() ) return false;
			if( strcmp(id, other.getId())!=0 ) return false;
			if( sigop!=other.getSigop() ) return false;
			if( idusuario != other.getIdusuario() ) return false;
			if( state!=other.getState()) return false;
			if( timestamp!=other.getTimestamp()) return false;
			if( strcmp(ip,other.getIp())!=0 ) return false;
			if( strcmp(useragent, other.getUseragent())!=0 ) return false;
			return true;
	}
	
	long getPkid() 		const	{return pkid;}
	const char* getId() 		const	{return id;}
	int getSigop() 		const	{return sigop;}
	int getIdusuario()	const	{return idusuario;}
	session_state getState() const {return state;}
	long getTimestamp() 	const	{return timestamp;}
	const char* getIp() 		const	{return ip;}
	const char* getUseragent()const	{return useragent;}
	
	AdmSessionDao& setPkid(long id) {pkid=id;return *this;}	
	AdmSessionDao& setId(const char* pid ) {strncpy(id, pid,sizeof(id)-1); id[sizeof(id)-1]='\0'; return *this;}
	AdmSessionDao& setSigop(int psigop ) 	 {sigop=psigop;return *this;}
	AdmSessionDao& setIdusuario(long pidusuario) {idusuario=pidusuario;return *this;}
	AdmSessionDao& setState(session_state pstate)  {state=pstate;return *this;}
	AdmSessionDao& setTimestamp(long ptimestamp) {timestamp=ptimestamp;return *this;}
	AdmSessionDao& setIp(const char* pip) {strncpy(ip, pip,sizeof(ip)-1); id[sizeof(ip)-1]='\0'; return *this;}
	AdmSessionDao& setUseragent(const char* puseragent) {strncpy(useragent, puseragent,sizeof(useragent)-1); id[sizeof(useragent)-1]='\0'; return *this;}
	
		
	std::string toString() const;
	
	static std::string printList( std::list<AdmSessionDao*> listResult );
};



/***
 *  Table: Session
 *  AdmSession: each row initiates  after user log in correctly with login/password
 * 
 * 
 **/
class AdmSession: public AdmBase
{
protected:
	const char* getSQLCreateTable();
	const char* getSQLCreateIndex();

public:

	static int session_callback(void *NotUsed, int argc, char **argv, char **szColName);
	
	
	AdmSession()
	{
	}
	
	
	virtual ~AdmSession() { 
	 }
	
	int session_delete_table(sqlite3 *db);

	int session_create(sqlite3 *db, const AdmSessionDao* sessionDao, long* newPkid);
	int session_update(sqlite3 *db, const AdmSessionDao* sessionDao);
	int session_delete_by_id(sqlite3 *db, const char * idSession);
	int session_info_by_pkid(sqlite3 *db, long pkid, AdmSessionDao* sessionDao);
	int session_info_by_state(sqlite3 *db, long idsession, AdmSessionDao* sessionDao);
	int session_info_by_id(sqlite3 *db, const char* idsession, AdmSessionDao* sessionDao);
	int session_info_by_iduser(sqlite3 *db, long iduser, AdmSessionDao* sessionDao);
	int session_info_by_id_sigop( sqlite3 *db, const char* sessionid ,int sigop, AdmSessionDao* sessionDao );

	void readRow(AdmSessionDao* sessionDao, sqlite3_stmt* ppStmt );
		
	
};

#endif //_ADM_SESSION_HPP_
