/*
 * Copyright (c) 2018, Ibon <ibon@gmai.com>
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
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <sqlite3.h>


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


#include "../kcgi.h"
#include "../kcgijson.h"

#include "rcf-op-login.hpp"

int
main(void)
{
	struct kreq	 req;
	struct kjsonreq jsonreq;
	struct kfcgi	*fcgi;
	enum kcgi_err	 er;
	
	const char 	*page[] = { "index", "test" };

	if (KCGI_OK != khttp_fcgi_init(&fcgi, NULL, 0, page, 2, 0))
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
		
		//Ejemplo	
		const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
		rapidjson::Document d;
		d.Parse(json);

		// 2. Modify it by DOM.
		rapidjson::Value& s = d["stars"];
		s.SetInt(s.GetInt() + 1);

		// 3. Stringify the DOM
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		d.Accept(writer);
				
		std::string salida(buffer.GetString(), buffer.GetSize());
		
		
		// open database
		sqlite3 *db;
		int rc = sqlite3_open("/var/www/froga.db", &db);
		if(rc)	{
			khttp_puts(&req, "OPEN ERROR");	
		} else {
			khttp_puts(&req, "OPEN ok");	
			
			rcfOpLogin opLogin(db);
			
			std::string s="otra";
			//std::string s=opLogin.work("{name=\"login\",time=10,login=[id=\"ibon\",pass=\"vvvvv\" ]}");
			/***
			rapidjson::Document jlogin;
			jlogin.Parse(s.c_str());
			jlogin.Accept(writer);
			std::string otra(buffer.GetString(), buffer.GetSize());
			salida=otra;
			****/ 
			salida=s;
		}	
			
		// close database
		if(db) {
			sqlite3_close(db);
		}	
		
		
		//...proper code	
		khttp_puts(&req, (const char*) salida.c_str());	
		khttp_free(&req);
	}

	khttp_fcgi_free(fcgi);
	return 0;
}
