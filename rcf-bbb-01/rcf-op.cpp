
#include "rcf-op.hpp"


#include "util/jsonutil.hpp"




static const std::string templae_rcf_op_out="{\"name\":\"\", \"status\"="" , \"sigop\":\"\", \"time1\":0,\"time2\":0,\"xxxx\":{}}";

static const std::string msg_out_fail_J1="{\"status\":\"fail\", \"data\":{\"msg\":\"\"}}";

pid_t rcfOp::service_pid=0;

/****
 *  Parses de in message in rapid document and call proper operation method
 *  m_in has documente
 * 
 ***/

std::string rcfOp::dowork( )
	{
		AdmBase::dbg("rcfOp::dowork 0, operation: %s", operationName.c_str());
		std::stringstream strError;
		
		//logs
		std::string str=printRapidDocument(m_dinJ0);
		AdmBase::dbg("rcfOp::dowork Json in: %s",str.c_str());
		
		//get operation name
		operationName=m_dinJ0[STR_PARAM_NAME].GetString();
        AdmBase::dbg("rcfOp::dowork operation: %s",operationName.c_str());

        if( !m_dinJ0.HasMember(operationName.c_str()) ) {
            char buff[300];
            sprintf(buff,"rcfOp::dowork There is no J1 element. operation={...}: %s ",operationName.c_str());
            rcfOp::dbg(buff);
            createMsgJ0_error(m_dout, buff);
            return std::string("NO J1 element ");
        }

        rapidjson::Value& v=m_dinJ0[operationName.c_str()];
		
		m_pValueInJ1=(rapidjson::Value*)&v;
		assert(m_pValueInJ1->IsObject());

		createMsgJ0_init( m_dout );
				
		if( needSession() ) {
		//Validate sesssion and sigop
            rcfOp::dbg("rcfOp::dowork need session, YES ");

            if( !m_dinJ0.HasMember(STR_PARAM_SESSION) ) {
                rcfOp::dbg("rcfOp::dowork no session menber ");
                createMsgJ0_error(m_dout, "No sesssion" );
                return std::string("NO Session ");
            }
            if( !m_dinJ0.HasMember(STR_PARAM_SIGOP) ) {
                rcfOp::dbg("rcfOp::dowork no sigop menber ");
                createMsgJ0_error(m_dout, "No sigop" );
                return std::string("NO sigop ");
            }

            sessionid=m_dinJ0[STR_PARAM_SESSION].GetString();
            rapidjson::Value& rap_sigop=m_dinJ0[STR_PARAM_SIGOP];
            if( rap_sigop.IsInt() ) {
                sigop = rap_sigop.GetInt();
            }else {
                rcfOp::dbg("rcfOp::dowork no sigop int ");
                createMsgJ0_error(m_dout, "sigop no value" );
                return std::string("NO sigop int");
            }

            if( sessionid.size()<5 ) {
                rcfOp::dbg("rcfOp::dowork no sesion string ");
                createMsgJ0_error(m_dout, "session no value" );
                return std::string("NO Session empty");
            }

            //Get the session
            AdmSession admSession;
            int resList=admSession.session_info_by_id_sigop(db, sessionid.c_str(), sigop, &getAdmSessionDao() );

            rcfOp::dbg("rcfOp::dowork session_info_by_id_sigop res: %d", resList);
            rcfOp::dbg("rcfOp::dowork session_info_by_id_sigop for session: %s", sessionid.c_str() );
            rcfOp::dbg("rcfOp::dowork session_info_by_id_sigop for session: %d %d %d", resList, SQLITE_OK ,SQLITE_DONE );

            if( resList!=SQLITE_DONE ) {
                //Error	de session
                rcfOp::dbg("rcfOp::dowork No valid session: %d", resList);
                return std::string("Session is not valid");
            } else if(  getAdmSessionDao().getPkid()<0 ) {
                rcfOp::dbg("rcfOp::dowork no session not found: %s %d", sessionid.c_str(), sigop);
                return std::string("{\"msg\":\"__Session is not valid__\"}");
            }
            rcfOp::dbg("rcfOp::dowork session_info_by_id_sigop SESSION: %s", getAdmSessionDao().toString().c_str() );

		}else {
            rcfOp::dbg("rcfOp::dowork DONT NEED SESSION");
		}

		rcfOp::dbg("rcfOp::dowork before work","");
		std::string sRes=work();
		rcfOp::dbg("rcfOp::dowork work fin: %s", sRes.c_str());

		rapidjson::Document& docJ1=getDocumentJ1();

        std::string sJ1=printRapidDocument( docJ1 );
        AdmBase::dbg("rcfOpLogin::work J1 : %s", sJ1.c_str());

        std::string sJ0=printRapidDocument( m_dout );
        AdmBase::dbg("rcfOpLogin::work J0 before: %s", sJ0.c_str());

        createMsgJ0_end(m_dout, docJ1 );

        std::string sJ0end=printRapidDocument( m_dout );
        AdmBase::dbg("rcfOpLogin::work J0: %s", sJ0end.c_str());

		return sJ0end;
	}


/**
 *  Create a J0 error message
 *
 *  J0: {"time1":<ddddd> }
 */
void rcfOp::createMsgJ0_error(rapidjson::Document& document, const char* errormsg ) {
    if (!document.IsObject()) {
    document.Parse("{}");
    }
    assert(document.IsObject());
    rcfOp::dbg("createMsgJ0_init 1 ");
    rapidjson::Value status;
    {
        char buffer2[100];
        int len = sprintf(buffer2, "%s", "error");
        status.SetString(buffer2, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
    }
    document.AddMember(STR_PARAM_STATUS, status, document.GetAllocator());
    rapidjson::Value msg;
    {
        char buffer2[510];
        int len = sprintf(buffer2, "%s", errormsg);
        status.SetString(buffer2, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
    }
    document.AddMember(STR_PARAM_MSG, msg, document.GetAllocator());

    rcfOp::dbg("createMsgJ0_init 2 ");

}



/**
 *  Create a J0 message initial values
 *
 *  J0: {"time1":<ddddd> }
 */
void rcfOp::createMsgJ0_init(rapidjson::Document& document )
{
    document.Parse("{}");
    assert(document.IsObject());
    rcfOp::dbg("createMsgJ0_init 1 ");
	msg_add_time(document, STR_PARAM_TIME1 );
    rcfOp::dbg("createMsgJ0_init 2 ");

}



/****
 *
 *  Create a J0 message  (rapidjson::Document) that will be in message form a work
 *
 *  name: operation  name
 *  valueJ1: {"  ":"  "  }
 * 
 *  J0: {"name":"login", "sigop":xxxxx, "login":{" ":"  ","":""   }}
 *                                      -----------------
 * 									            J1
 *      ---------------------------------------------
 * 						J0
 * 					  
 ***/
 void rcfOp::createMsgJ0_end(rapidjson::Document& document, rapidjson::Document& valueJ1 )
{
    rcfOp::dbg("createMsgJ0 1 %s",operationName.c_str());
	char buffer2[100];
	rapidjson::Value valkey;
    {
        int len = sprintf(buffer2, "%s", STR_PARAM_NAME);
        valkey.SetString(buffer2, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
    }
    rcfOp::dbg("createMsgJ0 2");
    rapidjson::Value author;
    {
        int len = sprintf(buffer2, "%s", operationName.c_str());  // synthetic example of dynamically created string.

        author.SetString(buffer2, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
        // Shorter but slower version:
        // document["hello"].SetString(buffer, document.GetAllocator());
        // Constructor version:
        // Value author(buffer, len, document.GetAllocator());
        // Value author(buffer, document.GetAllocator());
        memset(buffer2, 0, sizeof(buffer2)); // For demonstration purpose.
    }
    // Variable 'buffer' is unusable now but 'author' has already made a copy.
    rcfOp::dbg("createMsgJ0 3");
    document.AddMember(valkey, author, document.GetAllocator());

    rapidjson::Value valSigop;
    {
        int len = sprintf(buffer2, "%s", STR_PARAM_SIGOP);
        valSigop.SetString(buffer2, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
    }
    rapidjson::Value sigopval;
    {
        sigopval.SetInt( getAdmSessionDao().getSigop() );
    }
    rcfOp::dbg("createMsgJ0 4");
    document.AddMember(valSigop, sigopval, document.GetAllocator());

    msg_add_servicepid(document, "servicepid" );
	msg_add_time(document, STR_PARAM_TIME2 );


    rcfOp::dbg("createMsgJ0 5");

	rapidjson::Value valJ1;
    {
        char bufferx[100];
        int len = sprintf(bufferx, "%s",operationName.c_str());  // synthetic example of dynamically created string.
        valJ1.SetString(bufferx, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
    }

    rcfOp::dbg("createMsgJ0 5-1");
    document.AddMember(valJ1, valueJ1, document.GetAllocator());
    rcfOp::dbg("createMsgJ0 6");

}

/***
 * Create J0 message. For testing
 * @param msgJ0: document to be created.
 * { "name":"<opername>", "<opername":{ <msgJ1> }   }
 *
 * @param msgJ1
 */

void rcfOp::createMsgJ0(rapidjson::Document& J0, rapidjson::Document& J1, const char* opername )
{
    createMsgJ0_session( J0, J1, opername, nullptr, 0 );
}

void rcfOp::createMsgJ0_session(rapidjson::Document& J0, rapidjson::Document& J1, const char* opername, const char* session, int sigop )
{
    J0.Parse("{}");
    rapidjson::Value name;
    {
        name.SetString(opername, static_cast<rapidjson::SizeType>(strlen(opername)), J0.GetAllocator());
    }
    J0.AddMember(STR_PARAM_NAME, name, J0.GetAllocator());
    if( session!= nullptr  ) {
        rapidjson::Value valsession;
        {
            valsession.SetString(session, static_cast<rapidjson::SizeType>(strlen(session)), J0.GetAllocator());
        }
        J0.AddMember(STR_PARAM_SESSION, valsession, J0.GetAllocator());
        rapidjson::Value valsigop;
        {
            valsigop.SetInt(sigop);
        }
        J0.AddMember(STR_PARAM_SIGOP, valsigop, J0.GetAllocator());
    }
    rapidjson::Value j1name;
    {
        j1name.SetString(opername, static_cast<rapidjson::SizeType>(strlen(opername)), J0.GetAllocator());
    }
    J0.AddMember(j1name, J1, J0.GetAllocator());
}


void rcfOp::msg_add_servicepid(rapidjson::Document& document, const char* fieldname)
{
	rapidjson::Value valparamkey;
	{
		char buffer3[100];
		int len = sprintf(buffer3, "%s", fieldname);
		valparamkey.SetString(buffer3, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
		memset(buffer3, 0, sizeof(buffer3));
	}

	rapidjson::Value valparam;
	valparam.SetInt((int)rcfOp::getServicePid() );
	document.AddMember(valparamkey, valparam, document.GetAllocator());

}


void rcfOp::msg_add_time(rapidjson::Document& document, const char* fieldname  )
{
	rcfOp::dbg("Msg_add_time 1");
	rapidjson::Value valtimekey;
	{
		char buffer3[100];
		int len = sprintf(buffer3, "%s", fieldname);
		valtimekey.SetString(buffer3, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
		memset(buffer3, 0, sizeof(buffer3));
	}
    rcfOp::dbg("Msg_add_time 2");
	rapidjson::Value valtime;
	{
		char buffer4[100];
        time_t now = time(0);
        localtime(&now);
        //long timestamp=now;
		int len = sprintf(buffer4, "%ld", (long)now);  // synthetic example of dynamically created string.
		valtime.SetString(buffer4, static_cast<rapidjson::SizeType>(len), document.GetAllocator());
		memset(buffer4, 0, sizeof(buffer4));
	}
    rcfOp::dbg("Msg_add_time 3");
	document.AddMember(valtimekey, valtime, document.GetAllocator());
    rcfOp::dbg("Msg_add_time 4");
}


/***
 *  Creates output fail message in:
 *  Document m_doutJ1;
  * {"status":"fail", "data":{"msg":"<msg>"}}";
 * @param msgout
 * @param msg
 */

void rcfOp::createMsgJ1_fail( const std::string& msg )
{

    const std::string  msgout=msg_out_fail_J1;

    AdmBase::dbg("rcfOpLogin::work  create_msg_out_fail 0");

    std::stringstream strError;
    unsigned res=createJson( getDocumentJ1(), msgout.c_str(), strError );

    assert( res==0 );
    assert( getDocumentJ1().IsObject() );


    char bufferx[900];
    int len= sprintf(bufferx, "%s",msg.c_str());  // synthetic example of dynamically created string.

    AdmBase::dbg("rcfOpLogin::work  create_msg_out_fail 1");

    assert(getDocumentJ1().HasMember("data"));
    rapidjson::Value& valmsg = getDocumentJ1()["data"]["msg"];

    valmsg.SetString(bufferx, static_cast<rapidjson::SizeType>(len), getDocumentJ1().GetAllocator());

    AdmBase::dbg("rcfOpLogin::work  create_msg_out_fail 2");


}



/**
 *  Need to get all info for m_dout menber variable.
 *  Assing finishing time, time2.
 *  Calculates next sigop value and stores it in bd
 * 
 *
 * 
 **/ 

void rcfOp::create_msg_out( const std::string& msg_out, rapidjson::Value& valueJ1)
{
		AdmBase::dbg("rcfOp::create_msg_out 0");
		
		std::stringstream strErrorResp;
		unsigned resResponse=createJson( m_dout, templae_rcf_op_out.c_str(), strErrorResp  );
		
		m_dout[operationName.c_str()].Swap(valueJ1);
		
		if(resResponse !=0 ) { //OK
				AdmBase::dbg("rcfOp::create_msg_out ERROR:%s",strErrorResp.str().c_str());
		}else {
			
			AdmBase::dbg("rcfOp::create_msg_out 1");
			
			int sigOp=getAdmSessionDao().getSigop();
			int sigOpPlusOne=(sigOp/1000)+1;
			int ramSigOp=AdmBase::getramdom(1,999);
			char buff[7];
			sprintf(buff,"%03d%03d", sigOpPlusOne,ramSigOp );
			sigOpPlusOne=atoi(buff);
			
			int timeNow=time(0);
			getAdmSessionDao().setTimestamp(timeNow);
			getAdmSessionDao().setSigop(sigOpPlusOne);
			AdmSession admSession;
			admSession.session_update(db, &getAdmSessionDao() );
			
			rapidjson::Value rap_sig(sigOpPlusOne);
			m_dout[STR_PARAM_SIGOP]=rap_sig;
			

			std::string strDocu=printRapidDocument( m_dout );
			AdmBase::dbg("rcfOp::create_msg_out OUT:%s", strDocu.c_str() );
		}
}




void rcfOp::dbg(const char* pattern,...)
{
    char buff[TAM_BUFF_LOG*3];
    va_list args;
    va_start(args, pattern);
    vsprintf(buff, pattern, args);
    va_end(args);
    std::cerr<<buff<<std::endl;
}


/**
 *   Exec external procee
 *   https://stackoverflow.com/questions/52164723/how-to-execute-a-command-and-get-return-code-stdout-and-stderr-of-command-in-c
 *   @TODO: esto esta fallando cuando hago:
 *   ls /usr/bin
 *   Imagino que es por el tamanio...
 *
 */
std::string rcfOp::rcf_exec(const char* cmd)
{
    std::array<char, 1028> buffer;
    std::string result;
    AdmBase::dbg("rcfOp::rcf_exec:%s",cmd);
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    AdmBase::dbg("rcfOp::rcf_exec 1");
    if (!pipe) throw std::runtime_error("popen() failed!");
    //AdmBase::dbg("rcfOp::rcf_exec 2");
    while (!feof(pipe.get())) {
        //AdmBase::dbg("rcfOp::rcf_exec 3");
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
            //AdmBase::dbg("rcfOp::rcf_exec 4");
            result += buffer.data();
        }
    }
    //Tracear pq al consultar /usr/bin la cadena no se puede mostrar...
    AdmBase::dbg("rcfOp::rcf_exec Listado tamanis: %d",result.size());
    for(unsigned  int cnt=0;cnt<result.size();cnt++)
    {
        char c=result[0];
        if(c=='\0') {
            AdmBase::dbg("rcfOp::rcf_exec posicion '\0': %d",cnt);
        }

    }
    AdmBase::dbg("rcfOp::rcf_exec Listado: %s",result.c_str());
    AdmBase::dbg("rcfOp::rcf_exec fin");
    return result;
}


/***
 * Run command trough socket
 * @param cmd
 * @return
 */

std::string rcfOp::rcf_exec_socket(const char* socket_path, const char* cmd)
{
    struct sockaddr_un addr;
    int fd;
    int rc;
    std::array<char, 1028> buffer;
    std::string result;

    AdmBase::dbg("rcf_exec_socket socket: %s", socket_path);
    AdmBase::dbg("rcf_exec_socket    (cmd): (%s)", cmd);

    int old_umask = umask(S_IXUSR|S_IXGRP|S_IWOTH|S_IROTH|S_IXOTH);

    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        AdmBase::dbg("rcf_exec_socket error socket: %s", strerror( errno ));
        return std::string(" Error socket ");
    }

    umask(old_umask);

    AdmBase::dbg("rcf_exec_socket socket_path: %s", socket_path);
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

    AdmBase::dbg("rcfOp::rcf_exec fin");
    return result;

}

