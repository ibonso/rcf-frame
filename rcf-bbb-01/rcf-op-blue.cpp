#include "rcf-op-blue.hpp"
#include "util/base64.h"
#include "rcf-conf.h"



static const std::string temp_j1_msg_in="{ \"oper\":\"\" }";



static const std::string msg_out_success="{\"status\":\"success\", \"data\":{\"msg\":\"\" }}";
static const std::string msg_out_fail="{\"status\":\"fail\", \"data\":{\"msg\":\"\"}}";


 /***
   // 2. Get user and pass
   //rapidjson::Value& name = din["name"];
 **/
std::string rcfOpBlue::work()
{
	AdmBase::dbg("rcfOpBblue::work 0");
    assert(db!=NULL);

    try {
    	AdmBase::dbg("rcfOpBblue::work 1");

        const rapidjson::Value& valueJ1 = m_dinJ0[rcfOpBlue::getOperName()];

        AdmBase::dbg("rcfOpBblue::work 2");

        std::string trazaJ0=printRapidDocument( m_dinJ0 );
        AdmBase::dbg("rcfOpBblue::work m_dinJ0: %s", trazaJ0.c_str());

        std::string trazaJ1=printRapidDocument( valueJ1 );
        AdmBase::dbg("rcfOpBblue::work J1: %s", trazaJ1.c_str());


        if(! valueJ1.HasMember(STR_PARAM_OPER) ) {
            AdmBase::dbg("rcfOpBblue::work  missing %s", STR_PARAM_OPER );
            createMsgJ1_fail(std::string("rcfOpCommand::work  missing parameter"));
            std::string messagefail=printRapidDocument( m_doutJ1 );
            AdmBase::dbg("rcfOpBblue::work w_out:%s", messagefail.c_str() );
            return messagefail;
        }
        AdmBase::dbg("rcfOpBblue::work 3");

        std::string exeOper=std::string(valueJ1[STR_PARAM_OPER].GetString());
        std::string param=std::string("");

        rcfOp::dbg("rcfOpBlue::work path:%s", exeOper.c_str());

        std::string result = rcf_exec_blue(OP_BBLUE_SOCKET, exeOper.c_str(), param.c_str());

        createMsgJ1_success(  result.c_str() );

        std::string message=printRapidDocument( m_doutJ1 );
        AdmBase::dbg("rcfOpBlue::work w_out:%s", message.c_str() );

        return message;

    }catch(std::exception& e) {
        AdmBase::dbg("rcfOpBlue::work EXCEPTION: %s \n",e.what());
        std::string sRes="rcfOpBlue::work EXCEPTION: ";
        std::string sRes2=e.what();
        return std::string(sRes+sRes2);
    }

}


/***
 *
 * It will add <result> parameter to msg field of json document.
 * {"status":"success", "data":{"msg":"<ls_result_base64>"}}
 *
 * @param msgout: template with success json document
 *
 */

void rcfOpBlue::createMsgJ1_success( const std::string& sResult )
{
    std::stringstream strError;
    unsigned res=createJson( m_doutJ1, msg_out_success.c_str(), strError );
    assert( res==0 );

    std::string resBase64=base64_encode_2((const unsigned char*)sResult.c_str(), sResult.length());

    //Tenemos la plantilla, completar con parametros
    m_doutJ1[STR_PARAM_DATA][STR_PARAM_MSG].SetString(resBase64.c_str(), static_cast<rapidjson::SizeType>(resBase64.size()), m_doutJ1.GetAllocator());
}



void rcfOpBlue::createMsgJ1_fail( const std::string&  msgout )
{
    rcfOp::createMsgJ1_fail(msgout);
}


void rcfOpBlue::createMsgJ1(rapidjson::Document& msgJ1, const char* exepath)
{
    std::stringstream strError;
    unsigned res=createJson( msgJ1, temp_j1_msg_in.c_str(), strError  );
    assert( res==0 );
    msgJ1[STR_PARAM_OPER].SetString(exepath, static_cast<rapidjson::SizeType>(strlen(exepath)), msgJ1.GetAllocator());

    std::string traza=printRapidDocument( msgJ1 );
    AdmBase::dbg("rcfOpBblue Creado document J1 para test:%s", traza.c_str());
}


/**
 Send to command to bblue server through socket
**/
std::string rcfOpBlue::rcf_exec_blue(const char* socket_path, const char* cmd, const char* param)
{
    struct sockaddr_un addr;
    int fd;
    int rc;
    std::array<char, 1028> buffer;
    std::string result;

    AdmBase::dbg("rcf_exec_blue socket: %s", socket_path);
    AdmBase::dbg("rcf_exec_blue    (cmd): (%s)", cmd);

    int old_umask = umask(S_IXUSR|S_IXGRP|S_IWOTH|S_IROTH|S_IXOTH);

    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        AdmBase::dbg("rcf_exec_blue error socket: %s", strerror( errno ));
        return std::string(" Error socket ");
    }

    umask(old_umask);

    AdmBase::dbg("rcf_exec_blue socket_path: %s", socket_path);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        AdmBase::dbg("connect error: %s", strerror( errno ));
        return std::string(" Error socket connect");
    }

    int cmdSize=strlen(cmd);

    if ((rc=write(fd, cmd, cmdSize )) < 0 ) {
            AdmBase::dbg("write socket error %s", strerror( errno ));
            return std::string(" Error socket write");
    }

    //sleep?

    while ( (rc=read(fd, buffer.data(), buffer.size()-1)) > 0) {
        buffer[rc+1]='\0';
        AdmBase::dbg("%s", buffer.data());
        result += buffer.data();
    }

    AdmBase::dbg("rcf_exec_blue fin");
    return result;

}








