#include <getopt.h>
#include <iostream>
#include <list>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <array>


#include <openssl/md5.h>

#include "util/jsonutil.hpp"
#include "adm-base.hpp"
#include "adm.hpp"

#ifndef _ADM_USER_HPP_
#define _ADM_USER_HPP_



//Global funcion
std::string md5(const std::string &str);


extern const char* SQL_CREATE_TABLE_USER;
extern const char* SQL_USER_INDEX_LOGIN;
extern const char* SQL_DELETE_TABLE_USER;
extern const char* SQL_INSERT_USER;
extern const char* SQL_SELECT_USER_BY_ID;
extern const char* SQL_SELECT_USER_BY_LOGIN;
extern const char* SQL_SELECT_USER_ALL;
extern const char* SQL_DELETE_USER;


#define  STR_PARAM_DATA "data"
#define  STR_PARAM_MSG "msg"
#define  STR_PARAM_PKID "pkid"
#define  STR_PARAM_LOGIN "login"
#define  STR_PARAM_NAME "name"
#define  STR_PARAM_EMAIL "email"
#define  STR_PARAM_PASS "pass"
#define  STR_PARAM_PASSNEW "passnew"


class AdmUserDao {

	protected:

	long pkid;
	char name[50];
	char login[50];
	char email[50];
	char password[50]; //We keep it md5+b64
	int  timestamp;

	public:

	long 	    getPkid() 	  const {return pkid;}
    const char* getName()     const {return name;}
    const char* getLogin()    const {return login;}
    const char* getEmail() 	  const {return email;}
    const char* getPassword() const {return password;}
    int  getTimestamp()       const {return timestamp;}

    AdmUserDao& setPkid(long id) {pkid=id;return *this;}
	AdmUserDao& setName(const char* pname ) {strncpy(name, pname,sizeof(name)-1); name[sizeof(name)-1]='\0';return *this; }
	AdmUserDao& setLogin(const char* plogin ) {strncpy(login, plogin,50); login[sizeof(login)-1]='\0';return *this;}
	AdmUserDao& setEmail(const char* pemail) {strncpy(email, pemail,50);email[sizeof(email)-1]='\0';return *this;}
	AdmUserDao& setPassword(const char* ppassword ) {strncpy(password, ppassword,50);password[sizeof(password)-1]='\0';return *this;}
	AdmUserDao& setTimestamp(int ptimestamp) {timestamp=ptimestamp;return *this;}

	AdmUserDao()
	{
		setPkid(-1);
		name[0]=login[0]=email[0]=password[0]='\0';
	}

	AdmUserDao(const AdmUserDao& otro)
	{
	this->operator=(otro);
	}

	AdmUserDao& operator=(const AdmUserDao& otro)
	{
		if( this != &otro ) {
			setPkid(otro.getPkid());
			setTimestamp(otro.getTimestamp());
			setName(otro.getName());
			setLogin(otro.getLogin());
			setEmail(otro.getEmail());
			setPassword(otro.getPassword());
			setTimestamp(otro.getTimestamp());
		}
		return *this;
	}

	bool operator==(const AdmUserDao& other) {
			if( samevalue(other)==0 ) return true;
			return false;
	}

	int samevalue(const AdmUserDao& otro)
	{
		if(getPkid()!=otro.getPkid()) {
			AdmBase::dbg("Id diferente\n");
			return -1;
		}else if(getTimestamp()!=otro.getTimestamp()) {
			AdmBase::dbg("timestamp diferente\n");
			return -2;
		}else if(strcmp(getLogin(),otro.getLogin())!=0) {
			AdmBase::dbg("login diferente\n");
			return -3;
		}else if(strcmp(getPassword(),otro.getPassword())!=0) {
			AdmBase::dbg("Password diferente\n");
			return -4;
		}else if(strcmp(getName(),otro.getName())!=0) {
			AdmBase::dbg("Name diferente\n");
			return -5;
		}
		return 0;
	}

	std::string toString()
	{
		return toString(0);
	}

	void toJsonJ1(rapidjson::Document& document, rapidjson::Value& dataJ1) const
	{
		static const std::string msg_out_success="{\"status\":\"success\", \"data\":{\"msg\":\"\", \"pkid\":\"\", \"login\":\"\", \"name\":\"\", \"email\":\"\" }}";

		//Tenemos la plantilla, completar con parametros
		dataJ1[STR_PARAM_MSG].SetString("ok", static_cast<rapidjson::SizeType>(strlen("ok")), document.GetAllocator());
		dataJ1[STR_PARAM_PKID].SetInt(getPkid());
		dataJ1[STR_PARAM_LOGIN].SetString(getLogin(), static_cast<rapidjson::SizeType>(strlen(getLogin())), document.GetAllocator());
		dataJ1[STR_PARAM_NAME].SetString(getName(), static_cast<rapidjson::SizeType>(strlen(getName())), document.GetAllocator());
		dataJ1[STR_PARAM_EMAIL].SetString(getEmail(), static_cast<rapidjson::SizeType>(strlen(getEmail())), document.GetAllocator());


	}

	bool readFromJ1(const rapidjson::Value& msgJ1)
	{
		AdmBase::dbg("readMsgJ1 0");
		if(msgJ1.HasMember(STR_PARAM_PKID)) {
			AdmBase::dbg("readMsgJ1 0_1");
			setPkid(msgJ1[STR_PARAM_PKID].GetInt());
		}

		if( !msgJ1.HasMember(STR_PARAM_LOGIN) ||
			!msgJ1.HasMember(STR_PARAM_NAME) ) {

			return false;
		}

		AdmBase::dbg("readMsgJ1 1");
		setLogin(msgJ1[STR_PARAM_LOGIN].GetString());
		AdmBase::dbg("readMsgJ1 2");
		setName(msgJ1[STR_PARAM_NAME].GetString());

		if( msgJ1.HasMember(STR_PARAM_EMAIL) ) {
			AdmBase::dbg("readMsgJ1 3 ");
			setEmail(msgJ1[STR_PARAM_EMAIL].GetString());
		}

		if( msgJ1.HasMember(STR_PARAM_PASS) ) {
			setPassword(msgJ1[STR_PARAM_PASS].GetString());
		}
		return true;

	}

	std::string toString(int tab)
	{
		std::string s="";
		std::string margin="";
		
		for(int cnt=0;cnt<tab;cnt++)
		{
				margin.append(" ");
		}
		
		s.append(margin).append("pkid:").append(std::to_string(getPkid())).append("\n");
		s.append(margin).append("login:").append(getLogin()).append("\n");
		s.append(margin).append("name:").append(getName()).append("\n");
		s.append(margin).append("email:").append(getEmail()).append("\n");
		//s.append(margin).append("pass:").append("--------").append("\n");
        s.append(margin).append("pass:").append(getPassword()).append("\n");
		s.append(margin).append("timestamp:").append(std::to_string(getTimestamp())).append("\n");
		
		return s;
	}
	
};


class AdmUser: public AdmBase
{

protected:
	const char* getSQLCreateTable();
	const char* getSQLCreateIndex();


public:	


	
	void readRow(AdmUserDao* pUser, sqlite3_stmt* ppStmt );
	

	int createUser(sqlite3* db, const char* name, const char* login, const char* email, const char* pass );
	int user_create(sqlite3 *db, const AdmUserDao* admUserDao, long* newPkid);
	int user_update(sqlite3 *db, const AdmUserDao* admUserDao);
	int user_delete_table(sqlite3 *db);
	int user_delete(sqlite3 *db, int pkid);
	int user_info_by_id(sqlite3 *db, int id, AdmUserDao* readAdmUserDao);
	int user_info_by_login(sqlite3 *db, const char* name, AdmUserDao* readAdmUserDao);
	int usr_info_all(sqlite3 *db, std::list<AdmUserDao*>& listAdmUserDao); //returs SQL_DONE if ok
		
	
	AdmUser()
	{}
	
		
	virtual ~AdmUser() 
	{}
	
};

#endif //_ADM_USER_HPP_







