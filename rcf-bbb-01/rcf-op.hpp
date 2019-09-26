#ifndef _RCF_OP_HPP
#define _RCF_OP_HPP

#include <sqlite3.h>
#include <unistd.h>
#include <string>
#include <stdio.h>
#include <ctime>
#include <cstdio>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>

#include <sys/socket.h>
#include <sys/un.h>

#include "util/jsonutil.hpp"

#include "adm-session.hpp"

#define STR_PARAM_NAME "name"
#define STR_PARAM_TIME1 "time1"
#define STR_PARAM_TIME2 "time2"
#define STR_PARAM_DATA "data"
#define STR_PARAM_SESSION "session"
#define STR_PARAM_STATUS "status"
#define STR_PARAM_MSG "msg"
#define STR_PARAM_SIGOP "sigop"

#define JSON_MSG_FAIL "{\"status\":\"fail\", \"data\":{\"msg\":\"\"}}"


class rcfOp {
	
	protected:

    rcfOp(sqlite3 *pdb, const char* oper ,rapidjson::Document& msgin):m_dinJ0(msgin) {
    	if( service_pid==0 ) {
    		service_pid=getppid();
    	}
        this->db=pdb; //db has to be opened
        sigop=0;
        m_pValueInJ1=nullptr;
        operationName=std::string(oper);
    }

    static pid_t service_pid;

	public:
    virtual ~rcfOp() {db=0;}

	protected:

	sqlite3 *db;
	AdmSessionDao m_admSessionDao;
	

	pid_t getServicePid() {return service_pid;}

	rapidjson::Document& m_dinJ0;  // rapidjson::Document with in message
	rapidjson::Document  m_dout;   // rapidjson::Document with out message. will be created: createMsgJ0_init createMsgJ0_end
	
	//j0 in
	std::string operationName;
	std::string sessionid="";
	int sigop;
	rapidjson::Value* m_pValueInJ1; //J1 message
	



	virtual std::string work()=0;
	virtual rapidjson::Document& getDocumentJ1()=0; //out J1


	virtual bool needSession(void) {return true;}
	void createMsgJ1_fail( const std::string& msg );


    public:

        std::string dowork();

		static void createMsgJ0(rapidjson::Document& msgJ0, rapidjson::Document& msgJ1, const char* opername );
        static void createMsgJ0_session(rapidjson::Document& J0, rapidjson::Document& J1, const char* opname, const char* session, int sigop );

	protected:

		AdmSessionDao& getAdmSessionDao() {return m_admSessionDao;}
        void createMsgJ0_init( rapidjson::Document& document );
		void createMsgJ0_end(rapidjson::Document& document, rapidjson::Document& valueJ1 );
        void createMsgJ0_error(rapidjson::Document& document, const char*msg );
        void msg_add_servicepid(rapidjson::Document& document, const char* fieldname);
        void msg_add_time(rapidjson::Document& document, const char* );

		virtual void create_msg_out( const std::string& msg_out, rapidjson::Value& valueJ1 );
		//virtual rapidjson::Document& getDocumentJ1() {return m_dout;}
		static void dbg(const char* pattern,...);
		std::string rcf_exec(const char* cmd);
        std::string rcf_exec_socket(const char* socket_path, const char* cmd);



};
#endif //_RCF_OP_HPP
