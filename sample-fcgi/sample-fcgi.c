/*	$Id: sample-fcgi.c,v 1.13 2018/04/10 15:30:36 kristaps Exp $ */
/*
 * Copyright (c) 2015, 2018 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "kcgi.h"
#include <kcgihtml.h>
#include "kcgijson.h"

extern const char* rcf_proxy(const char* paran, const char* msgin ,int argc, char* argv[]);


#define LOG_BUFF_SIZE 1024
const char * RCF_FRAME_PID_FILE="/var/www/rcf-frame.pid";


enum key {
  KEY_RCFPARAM,
  KEY_INTEGER,
  KEY__MAX
};



static const struct kvalid keys[KEY__MAX] = {
  { kvalid_stringne, "rcf" }, /* KEY_RCFPARAM */
  { kvalid_int, "integer" }, /* KEY_INTEGER */
};



/**
 * Log info of params recived
**/
static void param_trace(struct kreq *r) {
      struct kpair *p;
      struct khtmlreq req;
      
      fprintf(stderr, "Parameter trace\n");  
      
      if ((p = r->fieldmap[KEY_RCFPARAM])) {
		if( p->parsed.s == 0 )  {
			fprintf(stderr, "param p->parsed.s ==0 (No hay datos)) \n" );  
		}else {//Parameter complete
			fprintf(stderr, "param rcf valid string: %s\n",p->parsed.s );  
		}
      } else if (r->fieldnmap[KEY_RCFPARAM]) {
        fprintf(stderr, "param rcf NOT valid string: %s\n",p->parsed.s );  
      } else {
        fprintf(stderr, "param rcf NOT provided \n" );  
      }
    }



int
main(int argc, char* argv[])
{
	struct kreq	 req;
	struct kjsonreq	 jsonreq;
	struct kfcgi	*fcgi;
	enum kcgi_err	 er;
	const char 	*page[] = { "fcgi/index", "login", "test" };
	
	char logbuff[LOG_BUFF_SIZE]; //Buffer for logger
	
	//Create rcf-frame.pid file
	int	retStat, statFd, pidFile;
	ssize_t tamwrite;
	struct stat statbuf;
	char*ptr;


	openlog(0, LOG_PERROR | LOG_PID, LOG_DAEMON);
	syslog( LOG_INFO, "sample-fcgi. Parent pid=%d  child pid=%d", getppid(), getpid());


	//Create /www/var/rcf-frame.pid
	//ok if file doesnt exist
	if( (statFd=open(RCF_FRAME_PID_FILE, O_WRONLY|O_CREAT|O_SYNC|O_EXCL, S_IRWXU|S_IRGRP|S_IROTH))>0 ) {
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
	}

	
	char* ptrProxy; 

	if (KCGI_OK != khttp_fcgi_init(&fcgi, keys, KEY__MAX, page, 3, 0))
		return 0;

	for (;;) {
		er = khttp_fcgi_parse(fcgi, &req);
		if (KCGI_EXIT == er) {
			fprintf(stderr, "khttp_fcgi_parse: terminate\n");
			break;
		} else if (KCGI_OK != er) {
			fprintf(stderr, "khttp_fcgi_parse: error: %d\n", er);
			break;
		}

		er = khttp_head(&req, kresps[KRESP_STATUS], 
			"%s", khttps[KHTTP_200]);
		if (KCGI_HUP == er) {
			fprintf(stderr, "khttp_head: interrupt\n");
			khttp_free(&req);
			continue;
		} else if (KCGI_OK != er) {
			fprintf(stderr, "khttp_head: error: %d\n", er);
			khttp_free(&req);
			break;
		}

		er = khttp_head(&req, kresps[KRESP_CONTENT_TYPE], 
			"%s", kmimetypes[req.mime]);
		if (KCGI_HUP == er) {
			fprintf(stderr, "khttp_head: interrupt\n");
			khttp_free(&req);
			continue;
		} else if (KCGI_OK != er) {
			fprintf(stderr, "khttp_head: error: %d\n", er);
			khttp_free(&req);
			break;
		}

		er = khttp_body(&req);
		if (KCGI_HUP == er) {
			fprintf(stderr, "khttp_body: interrupt\n");
			khttp_free(&req);
			continue;
		} else if (KCGI_OK != er) {
			fprintf(stderr, "khttp_body: error: %d\n", er);
			khttp_free(&req);
			break;
		}
		
		//process_safe(&req);
		
		//IMPORTANTE
		//https://kristaps.bsd.lv/kcgi/kcgijson.3.html
		
		if( strcmp(req.suffix,"json")==0 ) {
				kjson_open(&jsonreq, &req);
				kjson_obj_open(&jsonreq);
				kjson_arrayp_open(&jsonreq, "hello");
				kjson_putstring(&jsonreq, "world");
				kjson_array_open(&jsonreq);
				kjson_putstring(&jsonreq, "world");
				kjson_array_close(&jsonreq);
				kjson_array_close(&jsonreq);
				kjson_putstringp(&jsonreq, "hello", "world");
				kjson_obj_close(&jsonreq);
				kjson_close(&jsonreq);
			
		}else {
			struct kpair *rcf_param;
			param_trace( &req );
			
			unsigned int i;
			char buff[200];
			for (i = 0; i < req.reqsz; i++) {
						snprintf(buff, sizeof(buff), "<p>Cabecera: %s=%s</p>\n",req.reqs[i].key, req.reqs[i].val);
						//khttp_puts(&req, buff);		
			}
				
			snprintf(logbuff, sizeof(logbuff), "Fullpath: %s\n Path: %s \n Suffix: %s\n ",req.fullpath, req.path, req.suffix );
			fprintf(stderr, logbuff, "");
			
			//khttp_puts(&req, buff);	
		     
			/** IBON **/
			rcf_param=req.fieldmap[KEY_RCFPARAM];
			
			if( !rcf_param ) {
				snprintf(logbuff, 1024, "There is no rcf param. %s %d \n", __FILE__, __LINE__);
				fprintf(stderr, logbuff, "");
				snprintf(logbuff, 1024, "<html><head></head>%s<p>%d<p>%s<body></body></html>", __FILE__, __LINE__, __TIMESTAMP__);
				er = khttp_puts(&req, logbuff );	
			}else {	
			
				ptrProxy=0;
				ptrProxy=(char*)rcf_proxy(req.path, rcf_param->parsed.s ,argc, argv);
				if( ptrProxy !=0 ) {
					snprintf(logbuff, 1023, "End of rcf_proxy: %s \n", ptrProxy);
					
					er=khttp_puts(&req, ptrProxy);
					free(ptrProxy);

				} else {
					snprintf(logbuff, 1024, "End of rcf_proxy NULL \n");
					fprintf(stderr, logbuff, "");
					er = khttp_puts(&req, "{\"name\":\"error\",\"desc\":\"call to rcf_proxy\"}");	
				}
			}
			
			
			if (KCGI_HUP == er) {
				fprintf(stderr, "khttp_puts: interrupt\n");
				khttp_free(&req);
				continue;
			} else if (KCGI_OK != er) {
				fprintf(stderr, "khttp_puts: error: %d\n", er);
				khttp_free(&req);
				break;
			}
		}

		khttp_free(&req);
	}

	khttp_fcgi_free(fcgi);
	return 0;
}
