#include "rcf-op-command.hpp"
#include "util/base64.h"
#include "rcf-conf.h"



static const std::string temp_j1_msg_in="{ \"exepath\":\"\" }";



static const std::string msg_out_success="{\"status\":\"success\", \"data\":{\"msg\":\"\" }}";
static const std::string msg_out_fail="{\"status\":\"fail\", \"data\":{\"msg\":\"\"}}";



 /***
   // 2. Get user and pass
   //rapidjson::Value& name = din["name"];
 **/
std::string rcfOpCommand::work()
{
    assert(db!=NULL);

    try {

        const rapidjson::Value& valueJ1 = m_dinJ0[rcfOpCommand::getOperName()];
        AdmBase::dbg("rcfOpCommand::work_0 login is object!");


        if(!valueJ1.HasMember(STR_PARAM_EXEPATH) ) {
            AdmBase::dbg("rcfOpCommand::work  missing exepath");
            createMsgJ1_fail(std::string("rcfOpCommand::work  missing parameter"));
            std::string messagefail=printRapidDocument( m_doutJ1 );
            AdmBase::dbg("rcfOpCommand::work w_out:%s", messagefail.c_str() );
            return messagefail;
        }


        std::string exePath=std::string(valueJ1[STR_PARAM_EXEPATH].GetString());



        rcfOp::dbg("rcfOpLsDir::work path:%s",exePath.c_str());

        //std::string result = rcf_exec(exePath.c_str());

        std::string result = rcf_exec_socket(OP_COMMAND_SOCKET ,exePath.c_str());

        createMsgJ1_success(  result.c_str() );

        std::string message=printRapidDocument( m_doutJ1 );
        AdmBase::dbg("rcfOpCommand::work w_out:%s", message.c_str() );

        return message;

    }catch(std::exception& e) {
        AdmBase::dbg("rcfOpCommand::work EXCEPTION: %s \n",e.what());
        std::string sRes="rcfOpCommand::work EXCEPTION: ";
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

void rcfOpCommand::createMsgJ1_success( const std::string& sResult )
{
    std::stringstream strError;
    unsigned res=createJson( m_doutJ1, msg_out_success.c_str(), strError );
    assert( res==0 );

    std::string resBase64=base64_encode_2((const unsigned char*)sResult.c_str(), sResult.length());

    //Tenemos la plantilla, completar con parametros
    m_doutJ1[STR_PARAM_DATA][STR_PARAM_MSG].SetString(resBase64.c_str(), static_cast<rapidjson::SizeType>(resBase64.size()), m_doutJ1.GetAllocator());
}



void rcfOpCommand::createMsgJ1_fail( const std::string&  msgout )
{
    rcfOp::createMsgJ1_fail(msgout);
}


void rcfOpCommand::createMsgJ1(rapidjson::Document& msgJ1, const char* exepath)
{
    std::stringstream strError;
    unsigned res=createJson( msgJ1, temp_j1_msg_in.c_str(), strError  );
    assert( res==0 );
    msgJ1[STR_PARAM_EXEPATH].SetString(exepath, static_cast<rapidjson::SizeType>(strlen(exepath)), msgJ1.GetAllocator());

    std::string traza=printRapidDocument( msgJ1 );
    AdmBase::dbg("rcfOpCommand Creado document J1 para test:%s", traza.c_str());
}





 
 
 
 
