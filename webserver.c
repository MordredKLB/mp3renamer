//==============================================================================
//
// Title:		webserver.c
// Purpose:		A short description of the implementation.
//
// Created on:	8/14/2017 at 1:09:21 AM by Kevin.
// Copyright:	Subversive Software. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <ansi_c.h>
#include <userint.h>
#include "webserver.h"

typedef long off_t;
typedef wchar_t sdk_wchar_t;
#define MHD_PLATFORM_H  1
typedef int socklen_t;
#include "microhttpd.h"
#include <formatio.h>
#include <utility.h>

#include "mp3renamer.h"

//==============================================================================
// Constants

#define PORT 666
#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define WEB_ROOT	"D:\\Source\\test\\mp3renamer-ui\\dist\\"
#define CONNECTION_HEADERS_BUF_LEN	10000

//==============================================================================
// Prototypes
int print_out_key (void *cls, enum MHD_ValueKind kind, 
                   const char *key, const char *value);
char* replace_char(char* str, char find, char replace);
void ClearFileList(void);


//==============================================================================
// Types

//==============================================================================
// Static global variables

char connectionHeadersBuf[CONNECTION_HEADERS_BUF_LEN];
struct MHD_Connection *openConnection = NULL;
char *openConnectionData = NULL;
char **fileList = NULL;
int numFiles = 0;


//==============================================================================
// Static functions

//==============================================================================
// Global variables

extern int panelHandle;

//==============================================================================
// Global functions

struct MHD_Daemon* startServerDaemon()
{
	return MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_ALLOW_UPGRADE, PORT, NULL, NULL,
                             &answer_to_connection, NULL,
                             MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                             NULL, MHD_OPTION_END);
}

void stopServerDaemon(struct MHD_Daemon* daemon)
{
	if (openConnectionData) {
		free(openConnectionData);
		openConnectionData = NULL;
	}
	if (openConnection) {
		MHD_resume_connection(openConnection);
	}
	MHD_stop_daemon (daemon);
}


int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls)
{
	struct connection_info_struct *con_info;
	int ret;
	struct MHD_Response *response;
	
	if (NULL == *con_cls) {

		con_info = malloc(sizeof (struct connection_info_struct));
		con_info->isSuspended = false;
		if (NULL == con_info)
			return MHD_NO;
		
		if (strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
			con_info->connectiontype = POST;
		} else if (strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
			con_info->connectiontype = GET;
		} else if (strcmp(method, MHD_HTTP_METHOD_OPTIONS) == 0) {
			con_info->connectiontype = OPTIONS;
		} else {
			return MHD_NO;
		}

		*con_cls = (void *)con_info;

		return MHD_YES;
	}
	
	con_info = (struct connection_info_struct *)(*con_cls);
	
	switch (con_info->connectiontype) {
		case GET:
			if (!strncmp("/rest/open", url, 11)) {
				if (con_info->isSuspended) {
					if (openConnectionData) {
						openConnection = 0;
						ret = send_data(connection, openConnectionData);
						free(openConnectionData);
						openConnectionData = NULL;
					} else {
						ret = MHD_NO;
					}
					return ret;
				} else {
					openConnection = connection;
					MHD_suspend_connection (connection);
					con_info->isSuspended = TRUE;
					return MHD_YES;
				}
			} else if (!strncmp("/rest/", url, 6)) {
				// data
				return send_data(connection, "{ \"a\": \"data\", \"b\": true, \"c\": 1 }");
			} else {
				// files
				return send_page(connection, url);
			}
		
		case POST:
			if (!con_info->isParsing) {
				con_info->isParsing = TRUE;
				con_info->read_post_data[0] = 0;
	            return MHD_YES;
			} else {
				if (*upload_data_size != 0) {
				    // Receive the post data and write them into the buffer
				
				    strncpy(con_info->read_post_data, upload_data, *upload_data_size);
					con_info->read_post_data[*upload_data_size] = 0;
				    *upload_data_size = 0;
				    return MHD_YES;
				} else {
				    sprintf(connectionHeadersBuf, "Received data:\n%s", con_info->read_post_data);
				    appendToConnectionLog("\n\n");
					SetCtrlVal (panelHandle, PANEL_RECEIVE, connectionHeadersBuf);

					if (!strncmp("/command/file", url, 13)) {
						BrowseCB(panelHandle, PANEL_BrowseButton, EVENT_COMMIT, 0, 0, 0);
						ret = send_data(connection, "{ \"result\": \"success\" }");
					} else {														 
					    // *upload_data_size == 0 so all data have been received
					    ret = send_data(connection, con_info->read_post_data);
					}
					free(con_info);
				    con_info = NULL;
				    *con_cls = NULL;
					return ret;
				}
			}

		case OPTIONS:
			response = MHD_create_response_from_buffer (0, "", MHD_RESPMEM_PERSISTENT);
			MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
			MHD_add_response_header (response, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
			MHD_add_response_header (response, "Access-Control-Allow-Headers", "X-PINGOTHER, Content-Type");
			MHD_add_response_header (response, "Accept", 
				"text/html,application/json;q=0.9,*/*;q=0.8");
			ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
			MHD_destroy_response (response);
			return ret;
			
		default:
		 	response = MHD_create_response_from_buffer (0, "", MHD_RESPMEM_PERSISTENT);
			ret = MHD_queue_response (connection, MHD_HTTP_NOT_IMPLEMENTED, response);
			MHD_destroy_response (response);
			return ret;
	}

}

static void request_completed(void *cls, struct MHD_Connection *connection,
							   void **con_cls, enum MHD_RequestTerminationCode toe) 
{
	struct connection_info_struct *con_info = *con_cls;

	if (NULL == con_info)
		return;

	if (con_info->connectiontype == POST) {
	}

	free(con_info);
	*con_cls = NULL;
}

static int send_data (struct MHD_Connection *connection, const char *pageData)
{
	int ret;
	struct MHD_Response *response;
//	char *page = pageData;
 	
	MHD_get_connection_values (connection, MHD_HEADER_KIND, &print_out_key, NULL);

	appendToConnectionLog(pageData);
	response = MHD_create_response_from_buffer (strlen (pageData), (void *) pageData, MHD_RESPMEM_MUST_COPY);
	if (!response)
		return MHD_NO;
	MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
	MHD_add_response_header (response, "Cache-Control", "no-cache, no-store, must-revalidate");
	MHD_add_response_header (response, "Content-type", "application/json");
	MHD_add_response_header (response, "Expires", "0");
	ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
	MHD_destroy_response (response);
	
	appendToConnectionLog("\n");
	SetCtrlVal (panelHandle, PANEL_RECEIVE, connectionHeadersBuf);

	return ret;
}

static int send_page (struct MHD_Connection *connection, const char *url)
{
	int ret;
	struct MHD_Response *response;
 	char filePath[512] = {0};
	ssize_t	size;

	if (strlen(url)) {
		sprintf(connectionHeadersBuf, "URI: %s\n", url);
		sprintf(filePath, "%s%s", WEB_ROOT, url+1);
		if (strlen(url) == 1) {
			sprintf(filePath, "%s%s", WEB_ROOT, "index.html");
		}
		replace_char(filePath, '/', '\\');
		appendToConnectionLog( "PATH: ");
		appendToConnectionLog( filePath);
		appendToConnectionLog("\n");
	}
	
	if (GetFileInfo(filePath, &size)) {
		response = MHD_create_response_from_file_path(filePath);
		ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
		MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
		MHD_destroy_response (response);
	} else {
	 	response = MHD_create_response_from_buffer (0, "", MHD_RESPMEM_PERSISTENT);
		ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
		MHD_add_response_header (response, "Access-Control-Allow-Origin", "*");
		MHD_destroy_response (response);
	}
	appendToConnectionLog("\n");
	SetCtrlVal (panelHandle, PANEL_RECEIVE, connectionHeadersBuf);

	return ret;
}

/*
static int iterate_post (void *coninfo_cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off,
              size_t size)
{
	struct connection_info_struct *con_info = coninfo_cls;

	return MHD_YES;
}
*/

void resume_connection(char *json)
{
	if (openConnectionData) {
		free(openConnectionData);
		openConnectionData = NULL;
	}
	if (openConnection) {
		openConnectionData = malloc(strlen(json) + 1);
		strcpy(openConnectionData, json);
		MHD_resume_connection(openConnection);
	}
}

//==============================================================================
// Helper Functions

char* replace_char(char* str, char find, char replace) {
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

int print_out_key (void *cls, enum MHD_ValueKind kind, 
                   const char *key, const char *value)
{
	char buf[512];
	sprintf (buf, "%s: %s\n", key, value);
	appendToConnectionLog(buf);
	
	return MHD_YES;
}

void appendToConnectionLog(char *data)
{
	if (strlen(connectionHeadersBuf) + strlen(data) > CONNECTION_HEADERS_BUF_LEN) {
		connectionHeadersBuf[0] = '\0';
	}
	strcat(connectionHeadersBuf, data);
}
