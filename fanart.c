#include "hyperlinkctrl.h"
#include "progressbar.h"
#include <userint.h>
#include "config.h"
#include "mp3renamer.h"
#include "xmlHelper.h"
#include "jsmn.h"
#include "json.h"
#include <wininet.h>

typedef struct {
	int	panel;
	int control;
} PanelControl;

void SetupFanartPanel(void);
static PanelControl* CurrentSelectedControl(void);
void RetrieveFileFromURL(HINTERNET connection, char *url, char *fileName, int binary);
void FreeCdArtPaths(void);

/* Fanart source is fanart.tv */

/*API Key:

URL
Artist API: http://fanart.tv/webservice/artist/apikey/musicbrainz_mbid/format/type/sort/limit/
Album API: http://fanart.tv/webservice/album/apikey/musicbrainz_release-group_mbid/format/type/sort/limit/
Record Label API: http://fanart.tv/webservice/label/apikey/musicbrainz_label_mbid/format/type/sort/limit/
Updates
You can now get a list of all the artists that have been updated since you last checked, you can then match this list against local artists and just update those instead of checking for every single artist

http://fanart.tv/webservice/newmusic/apikey/timestamp/

If you omit the timestamp then artists updated in the last 2 days will be returned, if included it must be a valid UNIX timestamp

Supported formats
json
php
 

Returns the results in the requested format
json (default) / php (returns a php serialized object)
type
Returns the requested image types
all (default) / cdart / artistbackground / albumcover / musiclogo / artistthumbs
sort
1 - Sorted by most popular image then newest(default)
2 - Sorted by newest uploaded image
3 - Sorted by oldest uploaded image 
*/

// Opeth cd-art: 			"http://api.fanart.tv/webservice/artist/e98c81989fa12e8171f86068c8b9989a/c14b4180-dc87-481e-b17a-64e4150f90f6/XML/cdart/1/1/"
// Roundhouse tapes cd-art: "http://api.fanart.tv/webservice/album/e98c81989fa12e8171f86068c8b9989a/a368082f-370c-3206-a1c1-e9d11211877f/XML/cdart/1/1/"
// Balls to the wall cover: "http://api.fanart.tv/webservice/album/e98c81989fa12e8171f86068c8b9989a/5945662c-c63b-3c03-9e4b-80d814de82e1/XML/albumcover/1/1/"

#define aKey 			"e98c81989fa12e8171f86068c8b9989a"
#define clientAPIKey	"62c1ba6dcd7701667d97cd7ea384206f"

// http://webservice.fanart.tv/v3/music/albums/43a0e8b5-6059-397f-81e6-b8df5a648496?api_key=e98c81989fa12e8171f86068c8b9989a&client_key=62c1ba6dcd7701667d97cd7ea384206f

#define kArtistQuery	"http://api.fanart.tv/webservice/artist/%s/%s/XML/hdmusiclogo/1/1/"
#define kCDartQuery		"http://api.fanart.tv/webservice/album/%s/%s/XML/cdart/1/1/"

#define kNewArtistQuery "http://webservice.fanart.tv/v3/music/%s?api_key=%s&client_key=%s"
#define kNewCDartQuery	"http://webservice.fanart.tv/v3/music/albums/%s?api_key=%s&client_key=%s"

#define kArtistPage		"http://fanart.tv/artist/"

#define kMaxLogos		12
#define kMaxCdArt		6

char logoControls[kMaxLogos] = {HDLOGO_HDLOGO_1, HDLOGO_HDLOGO_2, HDLOGO_HDLOGO_3, HDLOGO_HDLOGO_4, HDLOGO_HDLOGO_5, HDLOGO_HDLOGO_6, HDLOGO_HDLOGO_7, HDLOGO_HDLOGO_8, HDLOGO_HDLOGO_9, HDLOGO_HDLOGO_10, HDLOGO_HDLOGO_11, HDLOGO_HDLOGO_12};
char cdartCtrls[kMaxCdArt] = {CDART_CDART_1, CDART_CDART_2, CDART_CDART_3, CDART_CDART_4, CDART_CDART_5, CDART_CDART_6};//, CDART_CDART_7, CDART_CDART_8, CDART_CDART_9};
char logoName[kMaxLogos][10] = {"l1.png", "l2.png", "l3.png", "l4.png", "l5.png", "l6.png", "l7.png", "l8.png", "l9.png", "l10.png", "l11.png", "l12.png"};
char cdName[kMaxCdArt][10] = {"cd1.png", "cd2.png", "cd3.png", "cd4.png", "cd5.png", "cd6.png", "cd7.png", "cd8.png", "cd9.png", "cd10.png", "cd11.png", "cd12.png"};
char *cdArtPaths[kMaxCdArt];

char url[kMaxLogos][256], cdUrl[kMaxCdArt][256];

int  logoExists=0;
int	 cdExists=0;
int  hoverEnabled=1;

const char *cdArtNames[] = {"cd.png", "cd1.png", "cd2.png", "cd3.png", "cd4.png", "cd5.png", "cd6.png", "cd7.png", "cd8.png", "cd9.png", "cd10.png",
							"vinyl.png", "vinylA.png", "vinylB.png", "vinylC.png", "vinylD.png", "vinylE.png", "vinylF.png", "vinylG.png", 
							"vinylH.png", "vinylI.png", "vinylJ.png", "vinylK.png"};

int CVICALLBACK RetrieveFanart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char 		artistMBID[38], relGroupMBID[38], path[MAX_PATHNAME_LEN*2];
	char 		queryBuf[256], previewUrl[264], fileName[256], artist[128];
	int			i, numLogos=0, numCdArt=0;
	double		percent;
    HINTERNET	connection;
	CVIXMLElement   curElem = 0;
	CVIXMLDocument	doc = 0;
	
	switch (event)
	{
		case EVENT_COMMIT:
			SetupFanartPanel();
			InstallPopup(fanartPanHandle);
			ProgressBar_Start(fanartPanHandle, FANART_PROGRESSBAR, "");
			GetCtrlVal(tab3Handle, TAB3_ARTISTMBID, artistMBID);
			GetCtrlVal(tab3Handle, TAB3_REID, relGroupMBID);
			GetCtrlVal(panelHandle, PANEL_ARTIST, artist);
			DisableBreakOnLibraryErrors();
			MakeDir("tempFanart");	// probably exists
			EnableBreakOnLibraryErrors();

			/*** HD LOGO ***/
			sprintf(path, "%s\\%s.png", artistLogoPath, artist);
			if (logoExists = FileExists(path, 0)) {
				SetCtrlAttribute(hdlogoPanHandle, logoControls[0], ATTR_VISIBLE, 1);
				SetCtrlAttribute(hdlogoPanHandle, logoControls[0], ATTR_IMAGE_FILE, path);
				SetCtrlVal(hdlogoPanHandle, logoControls[0], 1);
			}
			if (strlen(artistMBID) > 16) {
				sprintf(queryBuf, kNewArtistQuery, artistMBID, aKey, clientAPIKey);
				sprintf(fileName, "tempFanart\\%s-hdlogo.json", artistMBID);
				DownloadFileIfNotExists(queryBuf, fileName);
				
				FILE *f = fopen(fileName, "r");
				fseek(f, 0, SEEK_END);
				size_t len = (unsigned long)ftell(f);
				fseek(f, 0, SEEK_SET);
				const char *buf;
				buf = malloc(len * sizeof(char) + 1);
				fread(buf, sizeof(char), len, f);
				fclose(f);
				//js = ...;

				jsmntok_t *tokens = json_tokenise(buf, len);
				i=0;
				jsmntok_t *t = &tokens[i];
				while (t->type >= JSMN_OBJECT && t->type <= JSMN_STRING) {
					if (t->type == JSMN_STRING && json_token_streq(buf, t, "url")) {
						t = &tokens[++i];
						if (strstr(json_token_tostr(buf, t), "hdmusiclogo")) {
							if (numLogos+logoExists < kMaxLogos) {
								strcpy(url[numLogos+logoExists], json_token_tostr(buf, t));
							}
							numLogos++;
						}
					}
					t = &tokens[++i];
				}
				free(buf);
				free(tokens);
				
				
				ProgressBar_GetPercentage(panel, ALBUMPANEL_PROGRESSBAR, &percent);
				if (percent < 15.0)
					ProgressBar_AdvanceMilestone(fanartPanHandle, FANART_PROGRESSBAR, 0);			
				if (numLogos > 0) {
					connection = InternetOpen("MP3Renamer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
					for (i=logoExists;i<numLogos+logoExists && i<kMaxLogos;i++) {
						sprintf(previewUrl, "http://assets.fanart.tv/preview/%s", url[i] + 31);
						
						sprintf(fileName, "tempFanart\\%s", logoName[i]);
						RetrieveFileFromURL(connection, previewUrl, fileName, TRUE);
						SetCtrlAttribute(hdlogoPanHandle, logoControls[i], ATTR_VISIBLE, 1);
						SetCtrlAttribute(hdlogoPanHandle, logoControls[i], ATTR_IMAGE_FILE, fileName);
					}
					InternetCloseHandle(connection);
				}
				if (numLogos+logoExists > 6) {
					SetPanelAttribute(hdlogoPanHandle, ATTR_SCROLL_BARS, VAL_VERT_SCROLL_BAR);
				}
			}
			ProgressBar_GetPercentage(panel, ALBUMPANEL_PROGRESSBAR, &percent);
			if (percent < 50.0)
				ProgressBar_AdvanceMilestone(fanartPanHandle, FANART_PROGRESSBAR, 0);
			/*** CDART ***/
			cdExists = 0;
			for (i=0;i<(sizeof (cdArtNames) / sizeof (const char *));i++) {
				sprintf(path, "%s\\%s", pathName, cdArtNames[i]);
				if (FileExists(path, 0)) {
					SetCtrlAttribute(cdartPanHandle, cdartCtrls[cdExists], ATTR_VISIBLE, 1);
					SetCtrlAttribute(cdartPanHandle, cdartCtrls[cdExists], ATTR_IMAGE_FILE, path);
					SetCtrlVal(cdartPanHandle, cdartCtrls[cdExists], 1);
					cdArtPaths[cdExists] = malloc(sizeof(char) * strlen(path) + 1);
					strcpy(cdArtPaths[cdExists], path);
					cdExists++;
				}
			}
			
			if (strlen(relGroupMBID) > 16) {
				sprintf(queryBuf, kNewCDartQuery, relGroupMBID, aKey, clientAPIKey);
				sprintf(fileName, "tempFanart\\%s-cdart.json", relGroupMBID);
				DownloadFileIfNotExists(queryBuf, fileName);
				
				FILE *f = fopen(fileName, "r");
				fseek(f, 0, SEEK_END);
				size_t len = (unsigned long)ftell(f);
				fseek(f, 0, SEEK_SET);
				char *buf;
				buf = malloc(len * sizeof(char) + 1);
				fread(buf, sizeof(char), len, f);
				buf[len] = '\0';
				fclose(f);
				//js = ...;

				jsmntok_t *tokens = json_tokenise(buf, len);
				int i = 0;
				jsmntok_t *t = &tokens[i];
				while (t->type >= JSMN_OBJECT && t->type <= JSMN_STRING) {
					if (t->type == JSMN_STRING && json_token_streq(buf, t, "url")) {
						t = &tokens[++i];
						if (strstr(json_token_tostr(buf, t), "cdart")) {	// if the url is a cdart, it will be in the URL's path
							if (numCdArt+cdExists < kMaxCdArt) { 
								strcpy(cdUrl[numCdArt+cdExists], json_token_tostr(buf, t));
							}
							numCdArt++;
						}
					}
					t = &tokens[++i];
				}
				free(buf);
				free(tokens);
					

				ProgressBar_GetPercentage(panel, ALBUMPANEL_PROGRESSBAR, &percent);
				if (percent < 65.0)
					ProgressBar_AdvanceMilestone(fanartPanHandle, FANART_PROGRESSBAR, 0);
				if (numCdArt > 0) {
					connection = InternetOpen("MP3Renamer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
					for (i=cdExists;i<numCdArt+cdExists && i<kMaxCdArt;i++) {
						sprintf(previewUrl, "http://assets.fanart.tv/preview/%s", cdUrl[i] + 31);
						sprintf(fileName, "tempFanart\\%s", cdName[i]);
						RetrieveFileFromURL(connection, previewUrl, fileName, TRUE);
						SetCtrlAttribute(cdartPanHandle, cdartCtrls[i], ATTR_VISIBLE, 1);
						SetCtrlAttribute(cdartPanHandle, cdartCtrls[i], ATTR_IMAGE_FILE, fileName);
				    }
					InternetCloseHandle(connection);
				}
				if (numCdArt+cdExists > 3) {
					SetPanelAttribute(cdartPanHandle, ATTR_SCROLL_BARS, VAL_VERT_SCROLL_BAR);
				}
			}
			ProgressBar_GetPercentage(panel, ALBUMPANEL_PROGRESSBAR, &percent);
			if (percent < 99.0)
				ProgressBar_AdvanceMilestone(fanartPanHandle, FANART_PROGRESSBAR, 0);
			break;
	}
Error:
	if (event == EVENT_COMMIT) {
		ProgressBar_End(fanartPanHandle, FANART_PROGRESSBAR, NULL, NULL);
		SetCtrlAttribute(fanartPanHandle, FANART_PROGRESSBAR, ATTR_VISIBLE, 0);
	}
	if (curElem)
		CVIXMLDiscardElement (curElem);
	if (doc)
    	CVIXMLDiscardDocument (doc);
	return 0;
}

void RetrieveFileFromURL(HINTERNET connection, char *url, char *fileName, int binary)
{
	HINTERNET	resource;
	FILE		*file;
	char		data[4096];
	unsigned long bytes_read;

	resource = InternetOpenUrl(connection, url, NULL, 0, INTERNET_FLAG_NO_CACHE_WRITE, 0);
	if (resource != NULL) {
		file = fopen(fileName, binary ? "wb" : "w");
		if (file) {
	        while (InternetReadFile(resource, data, sizeof(data) - 1, &bytes_read) && (bytes_read > 0))
	            fwrite(data, sizeof(char), bytes_read, file);
			fclose(file);
			file = NULL;
		}
        InternetCloseHandle(resource);
	}
}

void DownloadFileIfNotExists(char *url, char *filename) 
{
    HINTERNET		connection;

	if (!FileExists(filename, 0)) {
		connection = InternetOpen("MP3Renamer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		RetrieveFileFromURL(connection, url, filename, FALSE);
		InternetCloseHandle(connection);
	}
}

/* We store files in tempFanart\ and we assume that any file older than 1 day is out of date and so we want to delete them */
void DeleteOldXMLandJSONFiles (int removeAll)
{
	int 	res, mon, day, year, hour, min, sec;
	char 	path[MAX_FILENAME_LEN], fileName[MAX_FILENAME_LEN];
	const 	char * fileTypes[] = {"xml", "json"};
	double	time, fileTime;
	
	GetCurrentDateTime(&time);
	for (int i=0; i < (sizeof (fileTypes) / sizeof (const char *)); i++) {
		sprintf(path, "tempfanart\\*.%s", fileTypes[i]);
		res = GetFirstFile(path, 1, 0, 0, 0, 0, 0, fileName);
		while (res == 0) {
			sprintf(path, "tempFanart\\%s", fileName);
			GetFileDate(path, &mon, &day, &year);
			GetFileTime(path, &hour, &min, &sec);
			MakeDateTime(hour, min, sec, mon, day, year, &fileTime);
			if (time - fileTime > 24 * 60 * 60) {		// we want to delete files that weren't created in the last 24 hours
				DeleteFile("");										   
			}
			res = GetNextFile(fileName);
		}
	}
}

void SetupFanartPanel ()
{
	int 	i;
	
	DeleteFile("tempFanart\\*.png");
	DeleteOldXMLandJSONFiles(FALSE);
	SetPanelAttribute(hdlogoPanHandle, ATTR_TOP, 32);
	SetPanelAttribute(hdlogoPanHandle, ATTR_LEFT, 10);
	SetPanelAttribute(hdlogoPanHandle, ATTR_SCROLL_BARS, VAL_NO_SCROLL_BARS);
	SetPanelAttribute(hdlogoPanHandle, ATTR_VSCROLL_OFFSET, 0); 
	SetPanelAttribute(hdlogoPanHandle, ATTR_VISIBLE, 1);
	SetPanelAttribute(cdartPanHandle, ATTR_TOP, 259);
	SetPanelAttribute(cdartPanHandle, ATTR_LEFT, 10);
	SetPanelAttribute(cdartPanHandle, ATTR_SCROLL_BARS, VAL_NO_SCROLL_BARS);
	SetPanelAttribute(cdartPanHandle, ATTR_VSCROLL_OFFSET, 0); 
	SetPanelAttribute(cdartPanHandle, ATTR_VISIBLE, 1);
	for (i=0;i<kMaxLogos;i++) {
		SetCtrlAttribute(hdlogoPanHandle, logoControls[i], ATTR_IMAGE_FILE, "");
		SetCtrlVal(hdlogoPanHandle, logoControls[i], 0);
		SetCtrlAttribute(hdlogoPanHandle, logoControls[i], ATTR_VISIBLE, 0);
	}
	for (i=0;i<kMaxCdArt;i++) {
		SetCtrlAttribute(cdartPanHandle, cdartCtrls[i], ATTR_IMAGE_FILE, "");
		SetCtrlVal(cdartPanHandle, cdartCtrls[i], 0);
		SetCtrlAttribute(cdartPanHandle, cdartCtrls[i], ATTR_VISIBLE, 0);
	}
	CA_InitActiveXThreadStyleForCurrentThread(0, COINIT_APARTMENTTHREADED);
	ProgressBar_SetAttribute(fanartPanHandle, FANART_PROGRESSBAR, ATTR_PROGRESSBAR_UPDATE_MODE, VAL_PROGRESSBAR_AUTO_MODE);
	ProgressBar_SetMilestones(fanartPanHandle, FANART_PROGRESSBAR, 15.0, 50.0, 65.0, 99.0, 0.0);
	ProgressBar_SetTotalTimeEstimate(fanartPanHandle, FANART_PROGRESSBAR, 5.0);
	SetCtrlAttribute(fanartPanHandle, FANART_PROGRESSBAR, ATTR_VISIBLE, 1);
	DeleteImage(hdPreviewHandle, HDPREVIEW_PREVIEW);
	SetCtrlAttribute(hdlogoPanHandle, HDLOGO_PREVIEWTIMER, ATTR_ENABLED, 1);
	SetCtrlAttribute(cdartPanHandle, CDART_PREVIEWTIMER, ATTR_ENABLED, 1);
	SetCtrlVal(fanartPanHandle, FANART_VINYLART, 0);
	SetCtrlVal(fanartPanHandle, FANART_MULTIPLEDISCS, 0);
}

int CVICALLBACK FanartOKCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int 		i, val, numDiscs=0, multiDiscs, vinyl;
	char		artist[256], destPath[MAX_PATHNAME_LEN*2], path[MAX_PATHNAME_LEN*2];
    HINTERNET	connection;
	
	switch (event)
	{
		case EVENT_COMMIT:
			SetCtrlAttribute(hdlogoPanHandle, HDLOGO_PREVIEWTIMER, ATTR_ENABLED, 0);
			SetCtrlAttribute(cdartPanHandle, CDART_PREVIEWTIMER, ATTR_ENABLED, 0);
			for (i=logoExists;i<kMaxLogos;i++) {
				GetCtrlVal(hdlogoPanHandle, logoControls[i], &val);
				if (val) {
					GetCtrlVal(panelHandle, PANEL_ARTIST, artist);
					sprintf(destPath, "%s\\%s.png", artistLogoPath, artist);
					sprintf(path, "tempFanart\\full%s", logoName[i]);
					if (FileExists(path, 0))
						CopyFile(path, destPath);
					else {
						connection = InternetOpen("MP3Renamer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
						RetrieveFileFromURL(connection, url[i], destPath, TRUE);
						InternetCloseHandle(connection);
					}
					break;
				}
			}
			GetCtrlVal(fanartPanHandle, FANART_MULTIPLEDISCS, &multiDiscs);
			GetCtrlVal(fanartPanHandle, FANART_VINYLART, &vinyl);
			for (i=cdExists;i<kMaxCdArt;i++) {
				GetCtrlVal(cdartPanHandle, cdartCtrls[i], &val);
				if (val && pathName) {
					if (!vinyl) {
						if (multiDiscs)	{
							sprintf(destPath, "%s\\cd%d.png", pathName, ++numDiscs);
						} else {
							sprintf(destPath, "%s\\cd.png", pathName);
						}
					} else {
						if (multiDiscs)	{
							sprintf(destPath, "%s\\vinyl%c.png", pathName, ++numDiscs + 64);	// start at 'A'
						} else {
							sprintf(destPath, "%s\\vinyl.png", pathName);
						}
					}
					sprintf(path, "tempFanart\\full%s", cdName[i]);
					if (FileExists(path, 0)) {
						CopyFile(path, destPath);
					} else {
						connection = InternetOpen("MP3Renamer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
						RetrieveFileFromURL(connection, cdUrl[i], destPath, TRUE);
						InternetCloseHandle(connection);
					}
					if (!multiDiscs) break;
				}
			}
			FreeCdArtPaths();
			RemovePopup(0);
			break;
	}
	return 0;
}

int CVICALLBACK FanartCancelCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SetCtrlAttribute(hdlogoPanHandle, HDLOGO_PREVIEWTIMER, ATTR_ENABLED, 0);
			SetCtrlAttribute(cdartPanHandle, CDART_PREVIEWTIMER, ATTR_ENABLED, 0);
			FreeCdArtPaths();
			RemovePopup(0);
			break;
	}
	return 0;
}

void FreeCdArtPaths(void)
{
	for (int i=0; i<kMaxCdArt; i++) {
		free(cdArtPaths[i]);
	}
}


int CVICALLBACK SelectCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val, i, x, y;
	PanelControl *currentCtrl = NULL;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &val);
			if (val) {
				if (panel == hdlogoPanHandle) {
					for (i=0;i<kMaxLogos;i++) {
						if (control != logoControls[i])
							SetCtrlVal(panel, logoControls[i], 0);
					}
				} else {
					GetCtrlVal(fanartPanHandle, FANART_MULTIPLEDISCS, &val);
					if (!val) // if we allow selecting multiple discs, don't clear them when selecting a new one
						for (i=0;i<kMaxCdArt;i++) {
							if (control != cdartCtrls[i])
								SetCtrlVal(panel, cdartCtrls[i], 0);
						}
				}
			}
			break;
		case EVENT_RIGHT_CLICK:
			GetRelativeMouseState(panel, 0, &x, &y, 0, 0, 0);
			currentCtrl = CurrentSelectedControl();	// save panel and control for DownloadFanartCB
			currentCtrl->control = control;
			currentCtrl->panel = panel;
			RunPopupMenu(pMenuHandle, PMENU_MENU1, panel, y, x, 0, 0, 0, 0);
			break;
		
	}
	return 0;
}

static PanelControl* CurrentSelectedControl()
{
    static PanelControl pcStruct = {0,0};
	
    return &pcStruct;
}

void DownloadFanartCB(int menubar, int menuItem, void *callbackData, int panel)
{
	PanelControl *currPanCtrl = NULL;
	int		i;
	char	path[MAX_PATHNAME_LEN*2];	
	
	switch (menuItem) {
		case PMENU_MENU1_ITEM1:
			currPanCtrl  = CurrentSelectedControl();
			if (currPanCtrl->panel == hdlogoPanHandle) {
				for (i=0;i<kMaxLogos;i++) 
					if (currPanCtrl->control == logoControls[i]) {
						sprintf(path, "tempFanart\\full%s", logoName[i]);
						if (!FileExists(path, 0)) {
							HINTERNET	connection;
							connection = InternetOpen("MP3Renamer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
							RetrieveFileFromURL(connection, url[i], path, TRUE);
							InternetCloseHandle(connection);
						}
						break;
					}
			} else {
				for (i=0;i<kMaxCdArt;i++) 
					if (currPanCtrl->control == cdartCtrls[i]) {
						sprintf(path, "tempFanart\\full%s", cdName[i]);
						if (!FileExists(path, 0)) {
							HINTERNET	connection;
							connection = InternetOpen("MP3Renamer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
							RetrieveFileFromURL(connection, cdUrl[i], path, TRUE);
							InternetCloseHandle(connection);
						}
						break;
					}
			}
			SetActiveCtrl(currPanCtrl->panel, currPanCtrl->control);
			break;
	}
}

int CVICALLBACK PreviewTimerCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int 	i, x, y, top, left, height, width, fTop, fLeft, offset;
	char	artist[256], path[MAX_PATHNAME_LEN*2];	
	static	lastX=0, lastY=0;

	switch (event)
	{
		case EVENT_TIMER_TICK:
			
			GetRelativeMouseState(panel, 0, &x, &y, 0, 0, 0);
			if (x == lastX && y == lastY && hoverEnabled) {
				for (i=0;i<kMaxLogos;i++) {
					GetCtrlBoundingRect(panel, logoControls[i], &top, &left, &height, &width);
					GetPanelAttribute(hdlogoPanHandle, ATTR_VSCROLL_OFFSET, &offset); 	// account for being scrolled down 
					if (offset > top) {
						height -= offset-top;
						top = offset;
					}
					if (x>=left && x<=left+width && y>=top && y<=top+height) {
						if (logoExists && i==0) {
							GetCtrlVal(panelHandle, PANEL_ARTIST, artist);
							sprintf(path, "%s\\%s.png", artistLogoPath, artist);
						}
						else {
							sprintf(path, "tempFanart\\full%s", logoName[i]);
						}
						if (FileExists(path, 0)) {
							DisplayImageFile(hdPreviewHandle, HDPREVIEW_PREVIEW, path);
							GetPanelAttribute(panel, ATTR_TOP, &top);
							GetPanelAttribute(panel, ATTR_LEFT, &left);
							GetPanelAttribute(fanartPanHandle, ATTR_TOP, &fTop); 
							GetPanelAttribute(fanartPanHandle, ATTR_LEFT, &fLeft);
							SetCtrlAttribute(hdPreviewHandle, HDPREVIEW_PREVIEW, ATTR_WIDTH, 800);
							SetCtrlAttribute(hdPreviewHandle, HDPREVIEW_PREVIEW, ATTR_HEIGHT, 310);
							SetPanelPos(hdPreviewHandle, y+top+fTop-35-offset, x+left+fLeft-8);
							SetPanelSize(hdPreviewHandle, 335, 804);
							SetCtrlAttribute(hdlogoPanHandle, HDLOGO_PREVIEWTIMER, ATTR_ENABLED, 0);	// disable while panel is displayed
							SetCtrlAttribute(cdartPanHandle, CDART_PREVIEWTIMER, ATTR_ENABLED, 0);
							InstallPopup(hdPreviewHandle);
							SetCtrlAttribute(hdPreviewHandle, HDPREVIEW_TIMER, ATTR_ENABLED, 1);
						}
						break;
					}
				}
			}
			lastX = x;
			lastY = y;
			break;
	}
	return 0;
}

/* This is essentially the same as the function above. They probably ought to be combined into one function, but I'm lazy at the moment */
int CVICALLBACK CDPreviewTimerCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int 	i, x, y, top, left, height, width, fTop, fLeft, offset, monitorID, mWidth, mHeight, pTop, pLeft, maxSize = 1000;
	char	path[MAX_PATHNAME_LEN*2] = "";	
	static	lastX=0, lastY=0;

	switch (event)
	{
		case EVENT_TIMER_TICK:
			GetRelativeMouseState(panel, 0, &x, &y, 0, 0, 0);
			if (x == lastX && y == lastY && hoverEnabled) {
				for (i=0;i<kMaxCdArt;i++) {
					GetCtrlBoundingRect(panel, cdartCtrls[i], &top, &left, &height, &width);
					GetPanelAttribute(cdartPanHandle, ATTR_VSCROLL_OFFSET, &offset); 	// account for being scrolled down
					if (offset > top) {
						height -= offset-top;
						top = offset;
					}
					if (x>=left && x<=left+width && y>=top && y<=top+height) {
						if (cdExists && i<cdExists) {
							sprintf(path, cdArtPaths[i]);
						} else {
							sprintf(path, "tempFanart\\full%s", cdName[i]);
						}
						if (FileExists(path, 0)) {
							DisplayImageFile(hdPreviewHandle, HDPREVIEW_PREVIEW, path);
							GetMonitorFromPanel(panel, &monitorID);
							GetMonitorAttribute(monitorID, ATTR_WIDTH, &mWidth);
							GetMonitorAttribute(monitorID, ATTR_HEIGHT, &mHeight);
							GetPanelAttribute(panel, ATTR_TOP, &top);
							GetPanelAttribute(panel, ATTR_LEFT, &left);
							GetPanelAttribute(fanartPanHandle, ATTR_TOP, &fTop); 
							GetPanelAttribute(fanartPanHandle, ATTR_LEFT, &fLeft);
							if (maxSize > mHeight - 25) {
								maxSize = mHeight - 25;	// don't let the maxImage size extend past the monitor bounds
							}
							if (maxSize > mWidth - 4) {
								maxSize = mWidth - 4;
							}
							SetCtrlAttribute(hdPreviewHandle, HDPREVIEW_PREVIEW, ATTR_WIDTH, maxSize);
							SetCtrlAttribute(hdPreviewHandle, HDPREVIEW_PREVIEW, ATTR_HEIGHT, maxSize);
							pTop = y+top+fTop-65-offset;
							pLeft = x+left+fLeft-8;
							if (pTop + maxSize > mHeight) pTop -= ((pTop + maxSize + 25) - mHeight);
							if (pLeft + maxSize > mWidth) pLeft -= ((pLeft + maxSize + 4) - mWidth);
							SetPanelPos(hdPreviewHandle, pTop, pLeft);
							SetPanelSize(hdPreviewHandle, maxSize + 25, maxSize + 4);
							SetCtrlAttribute(cdartPanHandle, CDART_PREVIEWTIMER, ATTR_ENABLED, 0);	// disable while panel is displayed
							SetCtrlAttribute(hdlogoPanHandle, HDLOGO_PREVIEWTIMER, ATTR_ENABLED, 0);
							InstallPopup(hdPreviewHandle);
							SetCtrlAttribute(hdPreviewHandle, HDPREVIEW_TIMER, ATTR_ENABLED, 1);
						}
						break;
					}
				}
			}
			lastX = x;
			lastY = y;
			break;
	}
	return 0;
}

int CVICALLBACK PreviewDisplayTimerCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int x, y;
	int width, height;
	
	switch (event)
	{
		case EVENT_TIMER_TICK:
			GetRelativeMouseState(panel, 0, &x, &y, 0, 0, 0);
			GetPanelAttribute(panel, ATTR_WIDTH, &width);
			GetPanelAttribute(panel, ATTR_HEIGHT, &height);
			if (x < -2 || y < -2 || x>width+2 || y>height+2) {
				SetCtrlAttribute(hdPreviewHandle, HDPREVIEW_TIMER, ATTR_ENABLED, 0);
				RemovePopup(0);
				SetCtrlAttribute(hdlogoPanHandle, HDLOGO_PREVIEWTIMER, ATTR_ENABLED, 1);
				SetCtrlAttribute(cdartPanHandle, CDART_PREVIEWTIMER, ATTR_ENABLED, 1);
			}
			break;
	}
	return 0;
}

int CVICALLBACK ColorCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
		
	switch (event)
	{
		case EVENT_MOUSE_POINTER_MOVE:
			if (control == HDPREVIEW_PREVIEW)
				val = VAL_TRANSPARENT;
			else
				GetCtrlAttribute(panel, control, ATTR_PICT_BGCOLOR, &val);
			SetCtrlAttribute(panel, HDPREVIEW_PREVIEW, ATTR_PICT_BGCOLOR, val);
			break;
	}
	return 0;
}

int CVICALLBACK OpenArtistPageCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char url[100], artistMBID[38];
	
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			GetCtrlVal(tab3Handle, TAB3_ARTISTMBID, artistMBID);
			sprintf(url, "%s%s", kArtistPage, artistMBID);
			OpenDocumentInDefaultViewer(url, VAL_NO_ZOOM);
			break;
	}
	return 0;
}

int CVICALLBACK FanartPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_KEYPRESS:
			if (eventData1 != VAL_ESC_VKEY)
				break;
			/* fall through on escape */
		case EVENT_CLOSE:
			SetCtrlAttribute(hdlogoPanHandle, HDLOGO_PREVIEWTIMER, ATTR_ENABLED, 0);
			SetCtrlAttribute(cdartPanHandle, CDART_PREVIEWTIMER, ATTR_ENABLED, 0);
			RemovePopup(0);
			break;
		case EVENT_GOT_FOCUS:
			hoverEnabled = 1;
			break;
		case EVENT_LOST_FOCUS:
			hoverEnabled = 0;	// don't enable hover if panel isn't on top
			break;
	}
	return 0;
}

int CVICALLBACK DeleteXMLFilesCB (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DeleteOldXMLandJSONFiles(TRUE);
			break;
	}
	return 0;
}
