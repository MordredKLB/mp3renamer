//==============================================================================
//
// Title:		webserver.h
//
// Created on:	8/14/2017 at 1:09:21 AM by Kevin.
// Copyright:	Subversive Software. All Rights Reserved.
//
//==============================================================================

#ifndef __webserver_H__
#define __webserver_H__

#ifdef __cplusplus
    extern "C" {
#endif

#define true	1
#define false   0
		
//==============================================================================
// Include files

typedef long off_t;
typedef wchar_t sdk_wchar_t;
#define MHD_PLATFORM_H  1
typedef int socklen_t;
#include "microhttpd.h"

//==============================================================================
// Constants

typedef enum {
	GET,
	DELETEREQ,
	PATCH,
	POST,
	OPTIONS
} ReqType;

//==============================================================================
// Types

struct connection_info_struct
{
	ReqType connectiontype;
	int isParsing;
	int isSuspended;
	char read_post_data[4096];
	char method[32];
	struct MHD_PostProcessor *postprocessor;
};

//==============================================================================
// External variables

//==============================================================================
// Global functions

struct MHD_Daemon* startServerDaemon();
void stopServerDaemon(struct MHD_Daemon* daemon);
int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls);
static void  request_completed(void *cls, struct MHD_Connection *connection,
							   void **con_cls, enum MHD_RequestTerminationCode toe);
static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off,
              size_t size);
static int send_page (struct MHD_Connection *connection, const char *url);
static int send_data (struct MHD_Connection *connection, const char *pageData);
void resume_connection(char *json);
void appendToConnectionLog(char *data);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __webserver_H__ */
