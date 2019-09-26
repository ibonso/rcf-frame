#ifndef _RCF_OP_BBLUE_HPP_
#define _RCF_OP_BBLUE_HPP_

#include <string>
#include <stdio.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "adm-user.hpp"
#include "adm-session.hpp"


#include "rcf-op.hpp"

/**
 *  http://rapidjson.org/md_doc_tutorial.html
 *
 *  IN: {"name":"bblue",  "session":"aaaaxxxxxxx", "sigop":xyz ,"bblue":{"oper":"ccccccc","param":"wwww" }}
 *  OUT:{"name":"bblue","sigop":xxx, "time1":0,"time2":0, "user":{"status":"success","data"{"msg":"xxxxxxk"}}}
 *
 **/

#define  OPER_NAME_BBLUE "bblue"
#define  STR_PARAM_OPER "oper"
#define  STR_PARAM_PARAM "param"


class rcfOpBlue : public  rcfOp
{
	std::string sResult;

	public:

		rcfOpBlue(sqlite3 *pdb, rapidjson::Document& msgin ):rcfOp(pdb, getOperName(), msgin) {
			sResult="";
		}

		virtual ~rcfOpBlue() {
		}

        virtual rapidjson::Document& getDocumentJ1()  {
            return m_doutJ1;
        }

		static void createMsgJ1(rapidjson::Document& msgJ1, const char* exepath);
	    static const char* getOperName() {return OPER_NAME_BBLUE;}

    protected:

		std::string work();
		rapidjson::Document m_doutJ1;

    private:
        void createMsgJ1_success( const std::string& sResult );
        void createMsgJ1_fail( const std::string&  msgout );
        std::string rcf_exec_blue(const char* socket_path, const char* cmd, const char* param);
};

#endif //_RCF_OP_BBLUE_HPP_
