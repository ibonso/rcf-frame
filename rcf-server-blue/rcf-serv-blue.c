#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>


#include <signal.h>

#include "rcf-conf.h"


/** BBLUE **/
#include "rc_usefulincludes.h"
#include "roboticscape.h"

// choice of 1,2,4,8,16 oversampling. Here we use 16 and sample at 25hz which
// is close to the update rate specified in robotics_cape.h for that oversample.
#define OVERSAMPLE  BMP_OVERSAMPLE_16
// choice of OFF, 2, 4, 8, 16 filter constants. Here we turn off the filter and 
// opt to use our own 2nd order filter instead.
#define INTERNAL_FILTER	BMP_FILTER_OFF

// our own low pass filter
#define ORDER			2
#define CUTOFF_FREQ		2.0f	// 2rad/s, about 0.3hz
#define BMP_CHECK_HZ	25
#define	DT				1.0f/BMP_CHECK_HZ
/** BBLUE-END **/

#define LISTENQ 5 


/**
 *   Deamon to run under root user. It reads the commands throug socket
 *   and send result (STDIN) and STDERR to same socket.
 *   It will allow rcf-op-command lo execute like root and connect to beaglebone
 *
 **/

#define SYSLOGNAME "rcf-serv-b-log"
#define PARAMS_NUM 20
#define PARAMS_MAXSIZE 100
#define BUFF_PARAM_SIZE 214
#define BUFF_SIZE 514

#define COMM_MOVE   "move"

//Direct commands 
#define COMM_ONE  "exemotor1"
#define COMM_TWO  "exemotor2"
#define COMM_TREE "exemotor3"
#define COMM_FOUR "exemotor4"

typedef  int bool;

int split_command_arguments(char* buff, char exepath[], char *argv[]);
int bblue_initialize(bool bInitMotor, bool bInitBarometer);
int bblue_close();
int main_test_barometer(int fd, int seconds);

/***BBLUE globals *****/
float temp, pressure, altitude, filtered_alt;
rc_filter_t lowpass;

//Configuration: can be changed with conf command... (@TODO)
int g_motorOne=1;
int g_motorTwo=2;

/*****/

struct pidfh *pfh;
pid_t otherpid, childpid;

const char *socket_path = OP_BBLUE_SOCKET ;
const char *pidfile_path="rcf-serv-blue.pid";

int socket_listen(const char *socket_path );
void rcf_exec(const char* cmd, int outsocket );

char buftraza[2140];

int main(int argc, char *argv[])
{
    int listenfd, connfd;
	int opt,opt2;
    int rc;
    const char* respFija="comando vuelta ejemplo\n";
    char pidfile[BUFF_PARAM_SIZE];
    char socketin_file[BUFF_PARAM_SIZE];
	char buff[BUFF_SIZE];
	char buff2[BUFF_SIZE];


    char msgsalida[514];

    strcncpy(socketin_file, socket_path, BUFF_PARAM_SIZE);
    strcncpy(pidfile, pidfile_path, BUFF_PARAM_SIZE);


    while (-1 != (opt = getopt(argc, argv, "s:dh"))) {
    		switch (opt) {
    		case ('s'):
				socket_path=optarg;
    			strncpy(socketin_file, optarg,BUFF_PARAM_SIZE-1);
    			break;
    		case ('f'):
    				strncpy(pidfile, optarg,BUFF_PARAM_SIZE-1);
					break;
    		default:
    			break;
    		}
    }

    /* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		syslog( LOG_ERR, "rcf-serv-blue error forking: %d", pid);
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
		syslog( LOG_INFO, "rcf-serv-blue forking ok, close parent. Child pid=%d", pid);
		exit(EXIT_SUCCESS);
	}


	//Create /var/run/rcf-serv-blue.pid
	//ok if file doesnt exist
	/**
	if( (statFd=open(RCF_BLUE_PID_FILE, O_WRONLY|O_CREAT|O_SYNC|O_EXCL, S_IRWXU|S_IRGRP|S_IROTH))>0 ) {
		snprintf(logbuff, LOG_BUFF_SIZE ,"%d\n",  getppid());
		syslog( LOG_INFO, "a escribir: %s", logbuff );
		if( (tamwrite=write(statFd, logbuff, strlen(logbuff))) < 0) {
			syslog( LOG_ERR, "error writing to: %s . %s", RCF_FRAME_PID_FILE, strerror(errno) );
			exit(1);
		}
		fsync(statFd);
		close(statFd);
	}else {
		syslog( LOG_INFO, "sample-fcgi. File exists.Nothing todo.");
	}**/


	/* Change the file mode mask */
	umask(0);

	openlog(SYSLOGNAME, LOG_PERROR | LOG_PID, LOG_DAEMON);

	sid = setsid();
	if (sid < 0) {
		syslog( LOG_ERR, "rcf-serv-blue  setsid(). Pid: %d", getpid());
		exit(EXIT_FAILURE);
	}
	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		syslog( LOG_ERR, "rcf-serv-blue  chdir(\"/\"). Pid: %d", getpid());
		exit(EXIT_FAILURE);
	}
	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	syslog(LOG_INFO, "rcf-serv-blue start, deamonized ok. Socket:%s pid:%d", socketin_file , getpid());

    syslog(LOG_DEBUG,"rcf-serv-b start. CLOCKS_PER_SEC:%d" , CLOCKS_PER_SEC );
    syslog(LOG_INFO,"rcf-serv-b start. CLOCKS_PER_SEC:%d" , CLOCKS_PER_SEC );
    
    listenfd=socket_listen( socketin_file );

    mode_t mode=S_IWOTH|S_IXOTH|S_IROTH|S_IRWXU|S_IRGRP|S_IWGRP|S_IXGRP;
    //Make socket writable for all
    int res=chmod(socketin_file, mode);
    syslog(LOG_INFO, "rcf-serv-b socket chmod res:%d", res);

    //Signal(SIGCHLD, sig_chld);
    //Signal(SIGINT, sig_int);

    openlog(SYSLOGNAME, LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "rcf-serv-blue  %d", getpid());

    bblue_initialize(1, 1);


    for ( ; ; ) {
		int m;
		long t;
		float d;
	
        syslog(LOG_INFO, "rcf-serv-b inicio for");
        if ( (connfd = accept(listenfd, NULL, NULL)) < 0) {
            if (errno == EINTR)
                continue;		/* back to for() */
            else
                perror("accept error");
        }
        
        syslog(LOG_INFO, "rcf-serv-b accept vuelto: %d", connfd);
        
	    //Ahora no necesitamos fork.Solo para lanzar exec.. ahora 
	    //se 'contacta' bblue en el mismo proceso
        int param_cnt=0;
        int barometer=0;
        char exepath[PARAMS_MAXSIZE+1];
        char* argv_l2[PARAMS_NUM];
        char* command;
 
		syslog(LOG_INFO, "rcf-serv-b accept antes memset");
 
        memset(exepath, 0, sizeof(exepath[0])*(PARAMS_MAXSIZE+1));
        memset(argv, 0, sizeof(argv_l2[0])*PARAMS_NUM);
		
		syslog(LOG_INFO, "rcf-serv-b accept antes dup2");
		
        //dup2(connfd, STDOUT_FILENO);
        //dup2(connfd, STDERR_FILENO);
        
        syslog(LOG_INFO, "rcf-serv-b accept se va hacer read");

        rc=read(connfd,buff,sizeof(buff)-1);
        if( rc<0 ) {
           syslog(LOG_ERR, "rcf-serv-b child socket:%s",socketin_file);
           syslog(LOG_ERR, "rcf-serv-b child read error:%d",rc);
           return 1;
        }
        syslog(LOG_INFO, "rcf-serv-b leido:%d",rc);
        buff[rc]='\0';
        
        sprintf(msgsalida,"ok rcf-serv-blue early close");
		write( connfd, msgsalida, strlen(msgsalida));
		syslog(LOG_INFO, "rcf-serv-b write hecho");
        //close(connfd);			
        syslog(LOG_INFO, "rcf-serv-b socket closed");
        
        
        syslog(LOG_INFO, "rcf-serv-b (command): (%s)",buff);
        

        strncpy(buff2,buff, BUFF_SIZE);
        param_cnt=split_command_arguments(buff2, exepath, argv_l2);

        syslog(LOG_INFO, "rcf-serv-b exefile:%s",exepath);
        syslog(LOG_INFO, "rcf-serv-b numparam:%d",param_cnt);
        
        {
			int cnt=0;
			for(cnt=0;cnt<param_cnt;cnt++)
			{
			  	syslog(LOG_INFO, "rcf-serv-b param %d:%s", cnt,argv_l2[cnt]);
			}
		}
        
        
        
        if( (command=strstr(exepath, COMM_ONE))== exepath ) {
				int res=main_motor1( param_cnt, argv_l2, buff );
				syslog(LOG_INFO, "rcf-serv-b main_motor1:%d", res);
		}else if( (command=strstr(exepath, COMM_TWO))== exepath ){
			
		}else if( (command=strstr(exepath, COMM_TREE))== exepath ){
			
		
		}else if( (command=strstr(exepath, COMM_MOVE))== exepath ) {
				syslog(LOG_INFO, "rcf-serv-b move command:%s",exepath);
				int res=main_move( param_cnt, argv_l2, buff );
				syslog(LOG_INFO, "rcf-serv-b move res:%d",res);
		}
       
        if(barometer) {
        	main_test_barometer(connfd, 1);
        }

		//Late close
		//sprintf(buff,"rcf-serv-blue ok");
		//write( connfd, buff, strlen(buff));
        //close(connfd);			
        syslog(LOG_INFO, "rcf-serv-b fin test_barometer");

		close(connfd);

    }//end for socket reading
    
	syslog(LOG_INFO, "rcf-serv-b fin");
    bblue_close(1, 1);

    closelog(); 
    
}

/***
 motor command interact with each motor individually. Good for testing
 and especific orders. For more general orders, user move commands: main_move
 

***/
int main_motor1(int param_cnt, char *argv[], const char* command )
{
	int opt2;
	int in;
    int motnumber;
    int allmotor=1;
    useconds_t timeon=2000000; //micrsecons
    double duty = 0.5; 

	syslog(LOG_INFO, "main_motor1 inicio, %d", param_cnt);	
	optind=1; 
	while ((opt2 = getopt(param_cnt, argv, "m:d:t:h")) != -1) {
		syslog(LOG_INFO, "main_motor1 while:%d", opt2);	
       	switch (opt2){
			case 'm': // motor channel option
				in = atoi(optarg);
				syslog(LOG_INFO, "main_motor1 param m:%d",in);
				
				if(in>=1 && in<=4){
					motnumber = in;
					allmotor = 0;
				}else {
					allmotor=1;
					motnumber=0;
				}
				break;
			case 't':
				syslog(LOG_INFO, "main_motor1 param t: %s", optarg);
				timeon=atoi(optarg)*1000;
				break;
			case 'd':
				syslog(LOG_INFO, "main_motor1 param d");
				duty = atof(optarg);
				break;
			default:
				syslog(LOG_INFO,"rcf-serv-blue main_motor1, unknown parameter: %s", command);
			
       	}
    }//fin while getopt

	syslog(LOG_INFO, "rcf-serv-blue main_motor1");
	syslog(LOG_INFO, "rcf-serv-blue main_motor1 timeon: %d", timeon);
	syslog(LOG_INFO, "rcf-serv-blue main_motor1 duty: %f", duty);

	if( allmotor ) {
		syslog(LOG_INFO, "rcf-serv-blue main_motor1 allmotor ");
		rc_set_motor_all(duty);
		int ret=usleep(timeon);
		rc_set_motor_all(0);

	}else if (motnumber>0 && motnumber<5 ) {
		syslog(LOG_INFO, "rcf-serv-blue main_motor1 motnumber:%d",motnumber);
		rc_set_motor(motnumber, duty);
		int ret=usleep(timeon);
		rc_set_motor_all(0);
	}else {
		syslog(LOG_INFO, "rcf-serv-blue main_motor1 no motor");
	}
	return 0;
}

/***
 Main move command. 
 
 move -f  -t<milis> -d0.8
 move -b  -t<milis> -d0.8
 move -s0                   (spin left)
 move -s1                   (spin right) 
  
****/
int main_move(int param_cnt, char *argv[], const char* allCommand )
{
	int opt;
	int forward=0;
    int backward=0;
    int spinleft=0;
    int spinright=0;
    double dutyParam=0.5;
    double dutyMotor1=0;
    double dutyMotor2=0;
    
    useconds_t timeon=2000000; //micrsecons
    
	
	while ((opt = getopt(param_cnt, argv+1, "fbs:t:d:h")) != -1) {
		int paramValue;
		
       	switch (opt){
		case 'f':
			forward=1;
		break;
		case 'b':
			backward=1;
		break;
		case 's':
			paramValue=atoi(optarg);
			spinleft=(paramValue==0)?1:0;
			spinright=(paramValue==0)?0:1;
		break;
	    case 't':
			timeon=atoi(optarg)*1000;
	    break;
	    case 'd':
			dutyParam=atof(optarg);
	    break;
	    default:
			abort ();
		}
	
	}
	
	//Check parameter are coherent
	if( (forward+backward+spinleft+spinright)>1) {
			syslog(LOG_ERR, "rcf-serv-blue main_move:wrong command sintax. -b -f -s[0|1] Cant go together");
			syslog(LOG_ERR,"%s",allCommand);
			return;
	}
	if( (forward+backward+spinleft+spinright)==0) {
		syslog(LOG_ERR, "rcf-serv-blue main_move: main command missing. -b -f -s[0|1] ");
		syslog(LOG_ERR,"%s",allCommand);
		return;
	}
	
	//Calculate how to move 2 motors...
	if( forward==1 ) {
		dutyMotor1=dutyMotor2=dutyParam;
	}else if(backward==1) {
		dutyMotor1=dutyMotor2=-dutyParam;
	}else if( spinleft==1  ) {
		dutyMotor1=-dutyParam;
		dutyMotor2=dutyParam;
	}else if( spinright==1  ) { 
		dutyMotor1=dutyParam;
		dutyMotor2=-dutyParam;
	}else  {
			syslog(LOG_ERR, "rcf-serv-blue main_move: something wrong with parameters");
			syslog(LOG_ERR,"%s",allCommand);
			return;
	}
	
	rc_set_motor(g_motorOne, dutyMotor1); rc_set_motor(g_motorTwo, dutyMotor2);
	int ret=usleep(timeon);
	rc_set_motor_all(0);
	
}


/***
 * buf contains command sor of:  /bin/rc_test_motor -m2 -l3  -c
 * Split commands,
 * return: number of parameters (3 this example)
 * argv[0]  rc_test_motro
 * argv[1]  -m2
 * ...
 * argv[n]  -c
 *
 */

int split_command_arguments( char* buff, char exepath[], char *argv[])
{
    int param_cnt=0;
    char *token;



    argv[param_cnt]=buff;

    token = strtok(buff, " ");

    while(token) {
        argv[param_cnt++]=token;
        token = strtok(NULL, " ");
    }
    strncpy(exepath, argv[0], PARAMS_MAXSIZE);

    if(param_cnt>1) {
        int fileseppos=-1;
        unsigned int cnt;

        for(cnt=0;cnt<strlen(exepath);cnt++)
        {
            if(exepath[cnt]=='/') {
                fileseppos=cnt;
            }
        }

        if(fileseppos>=0) {
            argv[0]+=fileseppos+1;
        }
    }
    return param_cnt;
}

int
socket_listen(const char *socket_path )
{
    struct sockaddr_un addr;
    int listenfd;
    int retry=5;
    char tracebuff[250];

    do {
        if ( (listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
            perror("socket error");
            //exit(-1);
            continue; /* error, try next one */
        }

        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        if (*socket_path == '\0') {
            *addr.sun_path = '\0';
            strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
        } else {
            strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
            unlink(socket_path);
        }

        //Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == 0)
            break;			/* success */

        close(listenfd);	/* bind error, close and try next one */
    } while (retry-- > 0 );

    if (retry == 0) {	/* errno from final socket() or bind() */
        sprintf( tracebuff, " Error (socket/bind) %s :   %s ", socket_path, strerror( errno ) );
        perror( tracebuff );
        exit(1);
    }
 
    if (listen(listenfd, LISTENQ) < 0) {
        sprintf( tracebuff, " Error (listen) %s :   %s ", socket_path, strerror( errno ) );
        perror( tracebuff );
        exit(1);
    }

    return(listenfd);
}






int bblue_initialize(bool bInitMotor, bool bInitBarometer )
{

	// initialize hardware first 
	if(rc_initialize()){
		fprintf(stderr,"ERROR: failed to run rc_initialize(), are you root?\n");
		return -1;
	}

	if( bInitBarometer && rc_initialize_barometer(OVERSAMPLE, INTERNAL_FILTER)<0){
		fprintf(stderr,"ERROR: rc_initialize_barometer failed\n");
		return -1;
	}

	if( bInitBarometer ) {
		// create the lowpass filter and prefill with current altitude
		if(rc_butterworth_lowpass(&lowpass, ORDER, DT, CUTOFF_FREQ)){
			fprintf(stderr,"ERROR: failed to make butterworth filter\n");
			return -1;
		}
		altitude = rc_bmp_get_altitude_m();
		rc_prefill_filter_inputs(&lowpass, altitude);
		rc_prefill_filter_outputs(&lowpass, altitude);
	}

	if(bInitMotor) {
		rc_enable_motors();
		rc_set_led(GREEN,ON);
		rc_set_led(RED,ON);
	}


	return 0; 	
}


int bblue_close( bool bMotor, bool bBaraometer)
{
	syslog(LOG_INFO, "rcf-serv-b bblue_close, pid: %d", getpid());
	pidfile_remove(pfh);
	if( bMotor) {
		rc_disable_motors();
	}

	if( bBaraometer ) {
		rc_power_off_barometer();
	}
	
    rc_cleanup();
    return 0;
}

/**
 *  based on: rc_test_motors.c
 *  Spin engine forward or backward for xxxx miliseconds
 *
	IN             (Initialize bbblue)
	M1 f 10000     (Motor 1, forward)
	M2 b 34444
	MX f <ttt>     (All motors, forward)
	MX b <ttt>
	SA
	RB
	RT
	TG
 *
 *
 */
int main_engine( int engine, int direction, long milisec   )
{




}

int main_test_barometer(int fd, int seconds)
{
    char buff[250];
    int cntlecturas=0;
    syslog(LOG_INFO, "rcf-serv-b main_test_barometer 0");
    
	// print a header
	snprintf(buff, sizeof(buff)/sizeof(buff[0]), "\ntemp|pressure|altitude|filtered\n" );
	write(fd, buff, strlen(buff));
		
	
	syslog(LOG_INFO, "rcf-serv-b main_test_barometer 1");
	
	time_t T= time(NULL);
	
	int seconds_wait=seconds;
	clock_t begin;
	double time_spent;
	unsigned int i;

	syslog(LOG_INFO, "rcf-serv-b main_test_barometer 2");

   	/* Mark beginning time */
	begin = clock();
	

	//now just wait, print_data will run
	//while (get_state()!=EXITING) {
	for(i=0;1;i++)
   	{
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;
		time_spent = time_spent*10;
		
		//Para por repeticiones
		if( cntlecturas>5) break;
		
		
        if (time_spent>=seconds_wait)
		        break;
		syslog(LOG_INFO, "rcf-serv-b main_test_barometer loop 3: %f  %d ",time_spent, seconds_wait);

		usleep(1000000/BMP_CHECK_HZ);
		
		syslog(LOG_INFO, "rcf-serv-b main_test_barometer loop 3.1");
		
		// perform the i2c reads to the sensor, this takes a bit of time
		if(rc_read_barometer()<0){
			syslog(LOG_INFO, "rcf-serv-b main_test_barometer loop 4");
			printf("\rERROR: Can't read Barometer");
			fflush(stdout);
			break; 
		}
		
		syslog(LOG_INFO, "rcf-serv-b main_test_barometer loop 5");

		// if we got here, new data was read and is ready to be accessed.
		// these are very fast function calls and don't actually use i2c
		temp = rc_bmp_get_temperature();
		pressure = rc_bmp_get_pressure_pa();
		altitude = rc_bmp_get_altitude_m();
		filtered_alt = rc_march_filter(&lowpass,altitude);

        snprintf(buff, sizeof(buff)/sizeof(buff[0]), "\n%6.2fC |%7.2fkpa | %8.2fm |%8.2fm\n", temp, pressure/1000.0,altitude,filtered_alt );
		write(fd, buff, strlen(buff));
		cntlecturas++;
		
		/***
		printf("\r");
		printf("%6.2fC |", temp);
		printf("%7.2fkpa |", pressure/1000.0);
		printf("%8.2fm |", altitude);
		printf("%8.2fm |", filtered_alt);
		printf("%8.2 |", time_spent);
		fflush(stdout); 
		***/ 
	}

	
	return 0;
}


/***
	
 ***/ 
void sig_handler(int signo)
{
  syslog(LOG_INFO, "sig_handler (RARO...):  %d",signo);	
  if (signo == SIGINT || signo == SIGKILL || signo==SIGSTOP ) {
	  syslog(LOG_INFO, "sig_handler (RARO...) llamando bblue_close");
		bblue_close( 1, 1);
  }
}




