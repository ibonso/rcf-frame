#include <gtest/gtest.h>
#include <openssl/md5.h>

#include "adm-base.hpp"
#include "adm.hpp"
#include "adm-user.hpp"
#include "util/base64.h"
#include "util/jsonutil.hpp"
#include "rcf-op-login.hpp"
#include "rcf-op-blue.hpp"
#include "test_util.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"



sqlite3 *db=0;


int main(int argc, char* argv[]){
    
    testing::InitGoogleTest(&argc, argv);
    
    std::stringstream ss;
	
	if(argc<=1) {
			std::cout << "--We need database name\n";
			exit(0);
	}	
	int rc = sqlite3_open(argv[1], &db);
	if(rc) {
		AdmBase::dbg( "Can't open database:%s", argv[1]);
		exit(0);
	} else {
		AdmBase::dbg( "Database opened  successfully:%s",argv[1]);
	}

    int resTest=RUN_ALL_TESTS();  
	
	if(db)
	{
	sqlite3_close(db);
	}
    return resTest;
}

/***
 *  Test an ok login with user admin, password: admin.
 * 
 */


void callOpBlue(rapidjson::Document& document, const char* session, int sigop, const char * path  )
{
	std::cout << "callOpBlue:"<<session<<" Path:"<<path<<std::endl;

    rapidjson::Document J0;
    rapidjson::Document J1;

    rcfOpBlue::createMsgJ1(J1, path);
    std::cout << "callOpBlue:fin createMsgJ1"<<std::endl;
    rcfOp::createMsgJ0_session(J0, J1, rcfOpBlue::getOperName(), session, sigop );
    std::cout << "callOpBlue:fin createMsgJ0"<<std::endl;

    std::string trace=printRapidDocument( J0 );
    std::cout << "J0 message:"<<trace.c_str()<<std::endl;

    rcfOpBlue opBlue(db, J0);
    std::cout << "callOpBlue:fin constructor"<<std::endl;

    std::string strBlue=opBlue.dowork();
    std::cout<<"rcf-op-blue respuesta: "<<strBlue.c_str()<<std::endl;

    document.Parse(strBlue.c_str());

    std::string traceRespuesta=printRapidDocument( document );
    std::cout<<"rcf-op-blue respuesta after parse: "<<traceRespuesta.c_str()<<std::endl;

    std::string sMensajeB64=std::string( document["bblue"]["data"]["msg"].GetString());
    std::string sMensaje=base64_decode_2(sMensajeB64);
    std::cout<<"msg b64:"<<sMensaje<<std::endl;


}

/***
 *  Test an ok login with user admin, password: admin.
 * 
 */
 

TEST(rcfopblue, rcfopbluebat)
{
    int sigop=-1;
    std::string sSessionId=getSessionLogin("admin","21232F297A57A5A743894A0E4A801FC3", sigop );

    std::cout << "SessionId:"<<sSessionId<<std::endl;

    const char* path="/home/ibon";

    rapidjson::Document berri;
    callOpBlue( berri, sSessionId.c_str(), sigop, path  );

    std::string traza=printRapidDocument( berri );

    std::cout << "RESPuesta:"<<traza<<std::endl;

}



