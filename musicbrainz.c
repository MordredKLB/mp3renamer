#include "progressbar.h"
#include "cvixml.h"
#include <formatio.h>
#include <cvirte.h>		
#include <userint.h>
#include "mp3renamer.h"
#include "ID3v2.h"
#include "xmlHelper.h"
#include "globals.h"
#include <wininet.h>

void GetMetaData(int panel, int control);
void PopulateMetaData(int panel);
void PopulateRingCtrls(int panel);
void SetCheckMarks(int panel);
void FreeAlbumInfo(void);
int  GetXMLAndPopulateTrackTree(int panel, int albumTree, int trackTree, int albumIndex);
void BuildFileNameFromQuery(char *query, char *fileName);
int GetArtistName(CVIXMLElement *curElem, int index, char *artist, char *artistID, int replaceApostrophe, int sortName, int addJoinPhrase);


int CVICALLBACK AlbumTreeSortCI(int panel, int control, int item1, int item2, int keyCol, void *callbackData);
int CVICALLBACK TrackTreeSortCI(int panel, int control, int item1, int item2, int keyCol, void *callbackData);

static int CVICALLBACK TrackThreadFunc (void *functionData);

extern void RetrieveFileFromURL(HINTERNET connection, char *url, char *fileName, int binary);


/* sample XML queries:
Tracks:  http://musicbrainz.org/ws/2/recording/?limit=100&query=reid:b0df6008-679f-4134-ae7a-92628dd3cf14
Release: http://musicbrainz.org/ws/2/release/?limit=100&query=artist:"Opeth" AND release:"In Live Concert" AND mediums:2
Artist:  http://www.musicbrainz.org/ws/2/artist/?query=arid:65f4f0c5-ef9e-490c-aee3-909e7ae6b2ab
*/

// I attempt various stages of matching to return the minimum number of album results to the user
#define kNumRelRequests	5

#define kTrackQuery		"http://musicbrainz.org/ws/2/release/%s?inc=artist-credits+recordings"

// kArtistQuery is used to retrieve the country code
#define kArtistQuery	"http://musicbrainz.org/ws/2/artist/%s"

// kDiscIDQuery is used to retrieve the disc subtitle
#define kDiscIDQuery	"http://musicbrainz.org/ws/2/release/%s?inc=discids"

#define kAreaIDQuery	"http://musicbrainz.org/ws/2/area/%s?inc=area-rels"

#define kRelQueryBase	"http://musicbrainz.org/ws/2/release/?limit=100&offset=%d&query="
#define kRelArtist		"artist:\"%s\""
#define kRelAlbum		"release:\"%s\""
#define kRelArtistFuzzy	"artist:(%s*)"
#define kRelAlbumFuzzy	"release:(%s*)"
#define kRelYear		"date:%s*"
#define kRelTracks		"tracks:\"%s\""
#define kRelDiscs		"mediums:\"%s\""
#define kRelType		"type:%s"
#define kRelStatus		"status:%s"
#define kRelFormat		"format:\"%s\""
#define kRelCountry		"country:%s"
#define kRelAnd			" AND "

#define kAttrCountStr	"count"
#define kElemTitleStr	"title"
#define kElemArtistStr	"artist"
#define kElemNameStr	"name"


typedef struct {
	int		val;
	char	string[20];
} dataRingVal;

#define kNumValTypes	7
dataRingVal ValTypesList[kNumValTypes] = {
	{1, "Album"},
	{2, "Single"},
	{3, "EP"},
	{4, "Compilation"},
	{5, "Soundtrack"},
	{6, "Live"},
	{7, "Remix"},
};

#define kNumStatusTypes	3
dataRingVal ValStatusList[kNumStatusTypes] = {
	{1, "Official"},
	{2, "Bootleg"},
	{3, "Promotion"},
};

#define kNumFormatTypes	5
dataRingVal	ValFormatsList[kNumFormatTypes] = {
	{1, "CD"},
	{2, "Vinyl"},
	{3, "DVD"},
	{4, "Digital Media"},
	{5, "Cassette"},
};

#define kNumCountryTypes 22
dataRingVal ValCountryList[kNumCountryTypes] = {
	{1, "US"}, {2, "GB"},
	{3, "JP"}, {4, "AU"},
	{5, "BR"}, {6, "CA"},
	{7, "DE"}, {8, "ES"},
	{9, "FI"}, {10, "FR"},
	{11, "GR"},{12, "IN"},
	{13, "IT"},{14, "HK"},
	{15, "MX"},{16, "NL"},
	{17, "NO"},{18, "NZ"},
	{19, "PT"},{20, "RU"},
	{21, "SE"},{22, "ZA"},
};

/*****************************************************************************/
/* Structures                                                                */
/*****************************************************************************/

typedef struct {
	char	trackNum[5];
	char	discNum[5];
	char 	*title;
	char	*artist;
	char	*artistFilter;
	char 	time[10];
} TrackInfoStruct;

typedef struct {
	int 	numTracks;
	TrackInfoStruct *tracks;
} AlbumInfoStruct;

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

static int	gNeedAnd;
static int	offset = 0;
static int	trackThreadFuncId = 0;
static int 	gExiting = 0;
static int	gSelectedAlbum = -1;
static int	lastAlbumSelected = -1;
static AlbumInfoStruct *gAlbumInfo;
static int	gAlbumCount = 0;

/*****************************************************************************/
/* Functions                                                                 */
/*****************************************************************************/
int CVICALLBACK GetMetaDataCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			PopulateMetaData(panel);
			SetCheckMarks(albumPanHandle);
			InstallPopup(albumPanHandle);
			DeleteOldXMLandJSONFiles(FALSE);
			gExiting = 0;
			CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, TrackThreadFunc, NULL, &trackThreadFuncId);
			RefreshReleasesCB(albumPanHandle, ALBUMPANEL_REFRESHRELEASES, EVENT_COMMIT, NULL, 0, 0);
			break;
		}
	return 0;
}

void PopulateMetaData(int panel)
{
	char	val[512], num[4];
	int		numTracks;
	
	GetCtrlVal(panel, 			PANEL_ARTIST, val);
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_ARTIST, val);
	GetCtrlVal(panel, 			PANEL_ALBUM, val);
	RemoveAlbumEditions(val);	
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_ALBUM, val);
	GetCtrlVal(tab1Handle,		TAB1_YEAR, val);
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_YEAR, val);
	GetNumListItems(panel, PANEL_TREE, &numTracks);
	sprintf(num, "%d", numTracks);
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_TRACKS, num);
	SetCtrlVal(albumPanHandle, 	ALBUMPANEL_DISCS, "1");	// TODO: actually do this
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_TYPE, 0);
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_FORMAT, 0);
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_STATUS, 0);
	SetCtrlVal(albumPanHandle,	ALBUMPANEL_COUNTRY, 0);
	PopulateRingCtrls(albumPanHandle);
	SetCtrlVal(albumPanHandle, ALBUMPANEL_TXTTYPE, "");
	SetCtrlVal(albumPanHandle, ALBUMPANEL_TXTLABEL, "");
	SetCtrlVal(albumPanHandle, ALBUMPANEL_TXTCATALOG, "");
	SetCtrlVal(albumPanHandle, ALBUMPANEL_TXTBARCODE, "");
	SetCtrlVal(albumPanHandle, ALBUMPANEL_TXTASIN, "");
}

void PopulateRingCtrls(int panel)
{
	int i;
	
	GetNumListItems(panel, ALBUMPANEL_TYPE, &i);
	if (i > 1)
		return;
	for (i=0;i<kNumValTypes;i++)
		InsertListItem(panel, ALBUMPANEL_TYPE, -1, ValTypesList[i].string, ValTypesList[i].val);
	for (i=0;i<kNumStatusTypes;i++)
		InsertListItem(panel, ALBUMPANEL_STATUS, -1, ValStatusList[i].string, ValStatusList[i].val);
	for (i=0;i<kNumFormatTypes;i++)
		InsertListItem(panel, ALBUMPANEL_FORMAT, -1, ValFormatsList[i].string, ValFormatsList[i].val);
	for (i=0;i<kNumCountryTypes;i++)
		InsertListItem(panel, ALBUMPANEL_COUNTRY, -1, ValCountryList[i].string, ValCountryList[i].val);
}


void SetCheckMarks(int panel)
{
	SetCtrlVal(panel, ALBUMPANEL_USEARTIST, 1);
	SetCtrlVal(panel, ALBUMPANEL_USEALBUM, 1);
	SetCtrlVal(panel, ALBUMPANEL_USEYEAR, 0);
	SetCtrlVal(panel, ALBUMPANEL_USETRACKS, 0);
	SetCtrlVal(panel, ALBUMPANEL_USEDISCS, 0);
	SetCtrlVal(panel, ALBUMPANEL_USETYPE, 0);
	SetCtrlVal(panel, ALBUMPANEL_USEFORMAT, 0);
	SetCtrlVal(panel, ALBUMPANEL_USESTATUS, 0);
	SetCtrlVal(panel, ALBUMPANEL_USECOUNTRY, 0);
}

int CVICALLBACK RefreshReleasesCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DeleteListItem(albumPanHandle, ALBUMPANEL_ALBUMTREE, 0, -1);
			offset = 0;
			gSelectedAlbum = -1;
			GetMetaData(panel, control);
			break;
	}
	return 0;
}

int GetRingValStr(int panel, int control, char *str)
{
	int val, returnVal=0;
	
	
	GetCtrlVal(panel, control, &val);
	if (val != 0) {
		val--;
		switch(control)
		{
			case ALBUMPANEL_TYPE:
				sprintf(str, "%s\0", ValTypesList[val].string);
				break;
			case ALBUMPANEL_FORMAT:
				sprintf(str, "%s\0", ValFormatsList[val].string);
				break;
			case ALBUMPANEL_STATUS:
				sprintf(str, "%s\0", ValStatusList[val].string);
				break;
			case ALBUMPANEL_COUNTRY:
				sprintf(str, "%s\0", ValCountryList[val].string);
				break;
		}
		returnVal = 1;
	}
	return returnVal;
}

char* GetQueryItem(int panel, int control, int useField, char* fieldFormat)
{
	int 		use, fuzzy;
	static char str[612];
	char		field[512], val[412];
	
	str[0] = '\0';
	GetCtrlVal(panel, useField, &use);
	if (use)
	{
		switch(control) {
			case ALBUMPANEL_TYPE:
			case ALBUMPANEL_FORMAT:
			case ALBUMPANEL_STATUS:
			case ALBUMPANEL_COUNTRY:
				if (GetRingValStr(panel, control, val)) {
					if (gNeedAnd)
						sprintf(field, "%s%s", kRelAnd, fieldFormat);
					else
						sprintf(field, "%s", fieldFormat);
					RemoveSurroundingWhiteSpace(val);
					sprintf(str, field, val);
					gNeedAnd = 1;
				}
				break;
			default:
				GetCtrlVal(panel, control, val);
				GetCtrlVal(panel, ALBUMPANEL_USEFUZZYSEARCH, &fuzzy);
				ReplaceDiacritics(val);
				ReplaceAmpersands(val);
				if (fuzzy) {
					StringLowerCase(val);
					ReplaceCommonWords(val);
				}
				if (gNeedAnd)
					sprintf(field, "%s%s", kRelAnd, fieldFormat);
				else
					sprintf(field, "%s", fieldFormat);
				RemoveSurroundingWhiteSpace(val);
				sprintf(str, field, val);
				gNeedAnd = 1;
				break;
		}
	}
	return str;	
}

void BuildQueryString(int panel, char **queryBuf, char *fileName)
{
	int 	len, fuzzy;
	size_t	numChars = 0;

	gNeedAnd = 0;
	GetCtrlAttribute(panel, ALBUMPANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &len);
	numChars += len;
	GetCtrlAttribute(panel, ALBUMPANEL_ALBUM, ATTR_STRING_TEXT_LENGTH, &len);
	numChars += len;
	GetCtrlAttribute(panel, ALBUMPANEL_YEAR, ATTR_STRING_TEXT_LENGTH, &len);
	numChars += len;
	GetCtrlAttribute(panel, ALBUMPANEL_TRACKS, ATTR_STRING_TEXT_LENGTH, &len);
	numChars += len;
	GetCtrlAttribute(panel, ALBUMPANEL_DISCS, ATTR_STRING_TEXT_LENGTH, &len);
	numChars += len;
	numChars += strlen(kRelQueryBase);
	numChars += strlen(kRelArtist);
	numChars += strlen(kRelAlbumFuzzy);
	numChars += strlen(kRelYear);
	numChars += strlen(kRelTracks);
	numChars += strlen(kRelDiscs);
	numChars += strlen("Compilation");	// for release-type
	numChars += strlen("Promotion");	// for release-status
	numChars += strlen("Digital Media");
	numChars += strlen(kRelAnd) * 8;
	
	GetCtrlVal(panel, ALBUMPANEL_USEFUZZYSEARCH, &fuzzy);
	*queryBuf = malloc(sizeof(char) * numChars + 1);
	sprintf(*queryBuf, kRelQueryBase, offset);
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_ARTIST, ALBUMPANEL_USEARTIST, fuzzy ? kRelArtistFuzzy : kRelArtist));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_ALBUM,  ALBUMPANEL_USEALBUM,  fuzzy ? kRelAlbumFuzzy : kRelAlbum));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_YEAR,   ALBUMPANEL_USEYEAR,   kRelYear));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_TRACKS, ALBUMPANEL_USETRACKS, kRelTracks));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_DISCS,  ALBUMPANEL_USEDISCS,  kRelDiscs));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_TYPE,   ALBUMPANEL_USETYPE,   kRelType));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_FORMAT, ALBUMPANEL_USEFORMAT, kRelFormat));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_STATUS, ALBUMPANEL_USESTATUS, kRelStatus));
	strcat(*queryBuf, GetQueryItem(panel, ALBUMPANEL_COUNTRY,ALBUMPANEL_USECOUNTRY,kRelCountry));
}

void BuildFileNameFromQuery(char *query, char *fileName) {
	int 	i, j=0;
	char 	*ptr, *temp;
	
	strcpy(fileName, "tempFanart\\");
	j=strlen(fileName);
	ptr = strstr(query, "query=");
	if (ptr) {
		for (i=strlen("query=")-1;i<strlen(ptr);i++) {
			if (isalnum(ptr[i]) || ptr[i] == '-') {
				fileName[j++] = ptr[i];
			}
		}
		fileName[j] ='\0';
	} else {
		// direct search, not using a query:
		temp = calloc(strlen(query) + 1, sizeof(char));
		strcpy(temp, query);
		ptr = strrchr(temp, '/');
		if (ptr) {
			ptr[0] = '-';
		}
		while (ptr = strchr(temp, '?')) {
			ptr[0] = '-';
		}
		ptr = strrchr(temp, '/');	// find 2nd to last '/'
		strcat(fileName, ptr + 1);
		free(temp);
	}
	strcat(fileName, ".xml");
}

#define kSearchMilestone	50.0
#define kRetrieveMilestone	90.0

void GetMetaData(int panel, int control)
{
	HRESULT 		error = S_OK;
	char			val[512], artist[512], artistID[38], *queryBuf = NULL, dateStr[5], discStr[10], fileName[MAX_PATHNAME_LEN] = "";
	int				totalCount=0, numTracks, count=0, i, numArtists, index;
	double			percent, time, delta;
	CVIXMLElement   curElem = 0;
	CVIXMLDocument	doc = 0;

	
	SetCtrlAttribute(panel, ALBUMPANEL_ALBUMTREE, ATTR_DIMMED, 0);
	SetCtrlAttribute(panel, ALBUMPANEL_TEXTBOX, ATTR_VISIBLE, 0);
	SetCtrlAttribute(panel, ALBUMPANEL_TXTMESSAGE, ATTR_VISIBLE, 0);
	SetCtrlAttribute(panel, ALBUMPANEL_MANUALSEARCH, ATTR_VISIBLE, 0);
	SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_VISIBLE, 0);
	CA_InitActiveXThreadStyleForCurrentThread(0, COINIT_APARTMENTTHREADED);
	ProgressBar_SetAttribute(panel, ALBUMPANEL_PROGRESSBAR, ATTR_PROGRESSBAR_UPDATE_MODE, VAL_PROGRESSBAR_AUTO_MODE);
	ProgressBar_SetMilestones(panel, ALBUMPANEL_PROGRESSBAR, kSearchMilestone, kRetrieveMilestone, 0.0);
	ProgressBar_SetTotalTimeEstimate(panel, ALBUMPANEL_PROGRESSBAR, 10.0);
	ProgressBar_Start(panel, ALBUMPANEL_PROGRESSBAR, "");
	SetCtrlAttribute(panel, ALBUMPANEL_PROGRESSBAR, ATTR_VISIBLE, 1);
	SetCtrlAttribute(panel, ALBUMPANEL_TEXTMSG, ATTR_VISIBLE, 0);
	SetCtrlAttribute(panel, ALBUMPANEL_TRACKTREE, ATTR_DIMMED, 1);
	DeleteListItem(panel, ALBUMPANEL_TRACKTREE, 0, -1);
	gSelectedAlbum = -1;
	ProcessDrawEvents();

	if (control != ALBUMPANEL_MANUALSEARCH)
		BuildQueryString(panel, &queryBuf, fileName);
	else {
		GetCtrlAttribute(panel, ALBUMPANEL_TEXTBOX, ATTR_STRING_TEXT_LENGTH, &totalCount);
		queryBuf = malloc(++totalCount * sizeof(char));
		GetCtrlVal(panel, ALBUMPANEL_TEXTBOX, queryBuf);
		}
	BuildFileNameFromQuery(queryBuf, fileName);	// create filename to save XML in
	time = Timer();

	DownloadFileIfNotExists(queryBuf, fileName);
	error = CVIXMLLoadDocument(fileName, &doc);
	delta = Timer() - time;
	if (error==S_OK) {
		hrChk (CVIXMLGetRootElement (doc, &curElem));
		CVIXMLGetElementTag(curElem, val);
		if (!strcmp(val, "error")) {
			if (!GetChildElementByTag(&curElem, "text")) {
				CVIXMLGetElementValue(curElem, val);
				SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_LABEL_TEXT, val);
			}
			SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_VISIBLE, 1);
			DeleteFile(fileName);
		} else {
			CVIXMLGetNumChildElements(curElem, &count);
			if (count > 0) {
				GetChildElementByIndex(&curElem, 0);	// release-list
				GetAttributeByName(curElem, kAttrCountStr, val);
				totalCount = strtol(val, NULL, 10);
				if (totalCount-offset > 100)
					count = 100;
				else
					count = totalCount-offset;
			}
			if (totalCount > 100)
				sprintf(val, "Displaying recordings %d-%d of %d results\0", offset, offset+100, totalCount);
			else
				sprintf(val, "%d results\0", count);

			if (offset > 0)
				SetCtrlAttribute(panel, ALBUMPANEL_PREV, ATTR_DIMMED, 0);
			else
				SetCtrlAttribute(panel, ALBUMPANEL_PREV, ATTR_DIMMED, 1);
			if (totalCount-offset < 100)
				SetCtrlAttribute(panel, ALBUMPANEL_NEXT, ATTR_DIMMED, 1);
			else
				SetCtrlAttribute(panel, ALBUMPANEL_NEXT, ATTR_DIMMED, 0);

			SetCtrlVal(panel, ALBUMPANEL_TEXTMSG, val);
		}
	} else {
		SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_LABEL_TEXT, "An error occurred");
		SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_VISIBLE, 1);
		DeleteFile(fileName);	// an error occurred so don't save file
	}
	if (count == 0) {
		SetCtrlAttribute(panel, ALBUMPANEL_ALBUMTREE, ATTR_DIMMED, 1);
		DeleteTextBoxLines(panel, ALBUMPANEL_TEXTBOX, 0, -1);
		SetCtrlVal(panel, ALBUMPANEL_TEXTBOX, queryBuf);
		SetCtrlAttribute(panel, ALBUMPANEL_TXTMESSAGE, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, ALBUMPANEL_TEXTBOX, ATTR_VISIBLE, 1);
		SetCtrlAttribute(panel, ALBUMPANEL_MANUALSEARCH, ATTR_VISIBLE, 1);
	}
	ProgressBar_GetPercentage(panel, ALBUMPANEL_PROGRESSBAR, &percent);
	if (percent < kSearchMilestone)
		ProgressBar_AdvanceMilestone(panel, ALBUMPANEL_PROGRESSBAR, 0);

	if (count > 0) {
		FreeAlbumInfo();
		gAlbumInfo = calloc(count, sizeof(AlbumInfoStruct));
		gAlbumCount = count;
		for (i=0;i<count;i++) {
			InsertTreeItem(albumPanHandle, ALBUMPANEL_ALBUMTREE, VAL_SIBLING, i-1, VAL_NEXT, "", NULL, NULL, i);
			GetChildElementByIndex(&curElem, i);	// <-- release
			
			GetAttributeByName(curElem, "id", val);
			SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColREID, ATTR_LABEL_TEXT, val);
			GetChildElementByTag(&curElem, kElemTitleStr);
			hrChk(CVIXMLGetElementValue(curElem, val));
			SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColAlbum, ATTR_LABEL_TEXT, val);
			GetParentElement(&curElem);	/* title */
			
			if (!GetChildElementByTag(&curElem, "artist-credit")) {
				CVIXMLGetNumChildElements(curElem, &numArtists);
				strcpy(artist, "");
				for (index=0; index<numArtists; index++) {
					GetArtistName(&curElem, index, val, artistID, true, false, true);
					strcat(artist, val);
				}
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColArtistID, ATTR_LABEL_TEXT, artistID);
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColArtist, ATTR_LABEL_TEXT, artist);
				GetArtistName(&curElem, 0, val, NULL, true, true, false); // get sort-name without joinphrase
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColArtistSortOrder, ATTR_LABEL_TEXT, val);
				GetParentElement(&curElem); 	// artist-credit
			}

			if (!GetChildElementByTag(&curElem, "release-group")) {
				GetAttributeByName(curElem, "id", val);	   // release-group id
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColRelGroupID, ATTR_LABEL_TEXT, val);

				GetAttributeByName(curElem, "type", val);
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColType, ATTR_LABEL_TEXT, val);
				GetParentElement(&curElem); /* release-group */
				}
			if (!GetChildElementByTag(&curElem, "date")) {
				hrChk(CVIXMLGetElementValue(curElem, val));
				strncpy(dateStr, val, 4);
				dateStr[4] = '\0';
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColDate, ATTR_LABEL_TEXT, dateStr);
				GetParentElement(&curElem);	/* date */
				}
			if (!GetChildElementByTag(&curElem, "country")) {
				hrChk(CVIXMLGetElementValue(curElem, val));
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColCountry, ATTR_LABEL_TEXT, val);
				GetParentElement(&curElem);	/* country */
				}
			if(!GetChildElementByTag(&curElem, "barcode")) {
				hrChk(CVIXMLGetElementValue(curElem, val));
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColBarcode, ATTR_LABEL_TEXT, val);
				GetParentElement(&curElem);
			}
			if(!GetChildElementByTag(&curElem, "asin")) {
				hrChk(CVIXMLGetElementValue(curElem, val));
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColASIN, ATTR_LABEL_TEXT, val);
				GetParentElement(&curElem);
			}
			if(!GetChildElementByTag(&curElem, "label-info-list")) {
				GetChildElementByTag(&curElem, "label-info");
				if (!GetChildElementByTag(&curElem, "catalog-number")) {
					hrChk(CVIXMLGetElementValue(curElem, val));
					SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColCatalog, ATTR_LABEL_TEXT, val);
					GetParentElement(&curElem);	/* catalog-number */
				}
				if(!GetChildElementByTag(&curElem, "label")) {
					if (!GetChildElementByTag(&curElem, "name")) {
						hrChk(CVIXMLGetElementValue(curElem, val));
						SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColLabel, ATTR_LABEL_TEXT, val);
						GetParentElement(&curElem);	/* name */
					}
					GetParentElement(&curElem);	/* label */
				}
				GetParentElement(&curElem);	/* label-info */
				GetParentElement(&curElem);	/* label-info-list */
			}
			GetChildElementByTag(&curElem, "medium-list");
			GetAttributeByName(curElem, "count", discStr);
			GetChildElementByTag(&curElem, "track-count");
			hrChk(CVIXMLGetElementValue(curElem, val));
			if (discStr[0] != '1') {
				strcat(val, " / ");
				strcat(val, discStr);
			}
			GetNumListItems(panelHandle, PANEL_TREE, &numTracks);
			SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColNumTracks, ATTR_LABEL_TEXT, val);
			if (strtol(val, NULL, 10) != numTracks) {
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColNumTracks, ATTR_LABEL_COLOR, VAL_RED);
				SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColNumTracks, ATTR_LABEL_BOLD, 1);
			}
			SetTreeCellAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, i, kAlbTreeColNumDiscs, ATTR_LABEL_TEXT, discStr);	// hidden
			GetParentElement(&curElem);	/* track-count */
			//GetParentElement(&curElem);	/* medium */
			GetParentElement(&curElem);	/* medium-list */
			GetParentElement(&curElem); /* release */
			ProgressBar_SetPercentage(panel, ALBUMPANEL_PROGRESSBAR, kSearchMilestone + kRetrieveMilestone * ((double)(i+1)/count), 0); 
			}
		ProgressBar_GetPercentage(panel, ALBUMPANEL_PROGRESSBAR, &percent);
		if (percent < kRetrieveMilestone)
			ProgressBar_AdvanceMilestone(panel, ALBUMPANEL_PROGRESSBAR, 0);
		SortTreeItems(albumPanHandle, ALBUMPANEL_ALBUMTREE, 0, 0, 0, 0, AlbumTreeSortCI, NULL);
		SetCtrlIndex(albumPanHandle, ALBUMPANEL_ALBUMTREE, 0);
		gSelectedAlbum = 0;	// should force the album to be listed below
		CVIXMLDiscardElement (curElem);
		CVIXMLDiscardDocument (doc);
		curElem = doc = 0;
		SetActiveCtrl(albumPanHandle, ALBUMPANEL_ALBUMTREE);
	}		

Error:
	ProgressBar_End(albumPanHandle, ALBUMPANEL_PROGRESSBAR, NULL, NULL);
	SetCtrlAttribute(panel, ALBUMPANEL_PROGRESSBAR, ATTR_VISIBLE, 0);
	SetCtrlAttribute(panel, ALBUMPANEL_TEXTMSG, ATTR_VISIBLE, 1);
	if (curElem)
		CVIXMLDiscardElement (curElem);
	if (doc)
    	CVIXMLDiscardDocument (doc);
	if (queryBuf)
		free(queryBuf);
	queryBuf = NULL;
	return;
}

void GetDiscSubtitles(int panel, int albumIndex, char **subtitles)
{
	HRESULT			loadError = S_OK, error = S_OK;
	char			val[512], reid[40], queryBuf[512], fileName[MAX_PATHNAME_LEN];
	int				numDiscs = 0;
	CVIXMLElement   curElem = 0;
	CVIXMLDocument	doc = 0;
	
	GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColREID, ATTR_LABEL_TEXT, reid);
	
	sprintf(queryBuf, kDiscIDQuery, reid);	// artist
	BuildFileNameFromQuery(queryBuf, fileName);	// create filename to save XML in
	DownloadFileIfNotExists(queryBuf, fileName);
	loadError = CVIXMLLoadDocument(fileName, &doc);
	if (loadError == S_OK) {
		hrChk (CVIXMLGetRootElement (doc, &curElem));
		CVIXMLGetElementTag(curElem, val);
		if (!strcmp(val, "error")) {
			loadError = S_FALSE;
		} else {
			GetChildElementByTag(&curElem, "release");
			if (!GetChildElementByTag(&curElem, "disambiguation")) {
				hrChk(CVIXMLGetElementValue(curElem, val));
				SetCtrlVal(tab1Handle, TAB1_EDITION, val);
				GetParentElement(&curElem);
			}
			GetChildElementByTag(&curElem, "medium-list");
			GetAttributeByName(curElem, "count", val);
			numDiscs = strtol(val, NULL, 10);	
			for (int i=0; i<numDiscs; i++) {
				GetChildElementByIndex(&curElem, i);	// "medium"
				if (!GetChildElementByTag(&curElem, "title")) {
					hrChk(CVIXMLGetElementValue(curElem, val));
					subtitles[i] = malloc(strlen(val) + 1);
					strcpy(subtitles[i], val);
					GetParentElement(&curElem);	// "title"
					if (numDiscs > 1 && i && strcmp(subtitles[0], subtitles[i])) {
						// if subtitles are different enable conflict LED
						SetCtrlVal(tab1Handle, TAB1_DISCSUBTITLELED, 1);
					}
					gUseMetaDataDiscSubtitleVal = TRUE;
				} else {
					subtitles[i] = malloc(1);
					strcpy(subtitles[i], "");
				}
				GetParentElement(&curElem);	// "medium"
			}
		}
	}
Error:
	if (curElem)
		CVIXMLDiscardElement (curElem);
	if (doc)
    	CVIXMLDiscardDocument (doc);
	return;
}

int GetCountryCode(CVIXMLElement *curElem, char *val)
{
	HRESULT error = S_OK;
	int 	found = 0;

	if (!GetChildElementByTag(curElem, "iso-3166-1-code-list") || !GetChildElementByTag(curElem, "iso-3166-2-code-list")) {
		GetChildElementByTag(curElem, "iso-3166-1-code");
		GetChildElementByTag(curElem, "iso-3166-2-code");	// both won't exist, so this should be safe because the first won't advance the pointer if it doesn't exist
		hrChk(CVIXMLGetElementValue(*curElem, val));			// 2 character country value
		val[2] = '\0';	// iso-3166-2 tags are of the form GB-NET, and we don't care about the exact area so lop off everything after the first two chars
		GetParentElement(curElem);	// iso-3166-1/2-code
		GetParentElement(curElem);	// iso-3166-1/2-code-list
		found = 1;
	}

Error:
	return found;
}

int GetAreaData(char *areaREID, char *val)
{
	HRESULT 		loadError = S_OK, error = S_OK;
	char 			area[512], queryBuf[512], fileName[MAX_PATHNAME_LEN], direction[512];
	int				found = 0, i = 0;
	CVIXMLElement   curElem = 0;
	CVIXMLDocument	doc = 0;

	sprintf(queryBuf, kAreaIDQuery, areaREID);	// artist
	BuildFileNameFromQuery(queryBuf, fileName);	// create filename to save XML in
	DownloadFileIfNotExists(queryBuf, fileName);
	loadError = CVIXMLLoadDocument(fileName, &doc);
	if (loadError == S_OK) {
		hrChk (CVIXMLGetRootElement (doc, &curElem));
		GetChildElementByIndex(&curElem, 0);	// area
		if (!(found = GetCountryCode(&curElem, val))) {
			if (!GetChildElementByTag(&curElem, "relation-list")) {
				while (!GetChildElementByIndex(&curElem, i)) {	// relation
					if (!GetChildElementByTag(&curElem, "direction")) {
						hrChk(CVIXMLGetElementValue(curElem, direction));
						GetParentElement(&curElem);			// direction	
						if (!strcmp(direction, "backward")) {
							if (!GetChildElementByTag(&curElem, "area")) {
								if (!GetAttributeByName(curElem, "id", area)) {
									if (found = GetAreaData(area, val)) {
										break;
									}
								}
								GetParentElement(&curElem);	// area
							}
						}
					}
					i++;
					GetParentElement(&curElem);				// relation
				}
			}
		}
	}
Error:
	if (curElem)
		CVIXMLDiscardElement (curElem);
	if (doc)
    	CVIXMLDiscardDocument (doc);
	return found;
}

void GetCountryFromArtist(char *reid) {
	HRESULT 		loadError = S_OK, error = S_OK;
	char 			val[512], area[512], queryBuf[512], fileName[MAX_PATHNAME_LEN];
	CVIXMLElement   curElem = 0;
	CVIXMLDocument	doc = 0;

	
	SetCtrlAttribute(tab1Handle, TAB1_COUNTRYERROR, ATTR_VISIBLE, FALSE);
	SetCountryName("--");	// clear country
	
	sprintf(queryBuf, kArtistQuery, reid);	// artist
	BuildFileNameFromQuery(queryBuf, fileName);	// create filename to save XML in
	DownloadFileIfNotExists(queryBuf, fileName);
	loadError = CVIXMLLoadDocument(fileName, &doc);
	if (loadError == S_OK) {
		hrChk (CVIXMLGetRootElement (doc, &curElem));
		CVIXMLGetElementTag(curElem, val);
		if (!strcmp(val, "error")) {
			loadError = S_FALSE;
		} else {
			GetChildElementByIndex(&curElem, 0);	// artist
			if (!GetChildElementByTag(&curElem, "country")) {	// does "country" exist?
				hrChk(CVIXMLGetElementValue(curElem, val));	// 2 character country value
				SetCountryName(val);
				GetParentElement(&curElem);
			} else {
				const char * tags[] = {"area", "begin-area"};
				for (int i=0; i < (sizeof (tags) / sizeof (const char *)); i++) {
					if (!GetChildElementByTag(&curElem, tags[i])) {
						if (GetCountryCode(&curElem, val)) {
							SetCountryName(val);
							GetParentElement(&curElem);	// area/begin-area
							break;
						} else {
							GetAttributeByName(curElem, "id", area);
							if (GetAreaData(area, val)) {
								SetCountryName(val);
								GetParentElement(&curElem);	// area/begin-area
								break;
							}
						}
					}
				}
			}
			GetParentElement(&curElem);
		}
	}
	if (loadError != S_OK) {
		SetCtrlAttribute(tab1Handle, TAB1_COUNTRYERROR, ATTR_TEXT_COLOR, VAL_RED);
		SetCtrlAttribute(tab1Handle, TAB1_COUNTRYERROR, ATTR_VISIBLE, TRUE);
		DeleteFile(fileName);
	}
Error:
	if (curElem)
		CVIXMLDiscardElement (curElem);
	if (doc)
    	CVIXMLDiscardDocument (doc);
	return;
}

/* Copies MetaData from the AlbumPanel to the main panel and populates the TrackTitle Tree */
void GetMetaTrackData(int panel, int albumIndex)
{
	char			val[512], reid[40], discNum[10], offset[10];
	char			*discSubtitles[99] = { NULL };
	int				len, releaseDiscs, i, k, count, numTracks, disc, trackNum;
	int 			oldYear, newYear, replaceUnicodeApostrophe;
	CVIXMLElement   curElem = 0;
	CVIXMLDocument	doc = 0;
	
	if (albumIndex != -1) {
		GetNumListItems(panelHandle, PANEL_TREE, &numTracks);
		GetCtrlVal(configHandle, OPTIONS_REPLACEAPOSTROPHE, &replaceUnicodeApostrophe);
		
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColArtist, ATTR_LABEL_TEXT, val);
		if (replaceUnicodeApostrophe)
			ReplaceUnicodeApostrophe(val);
		SetCtrlVal(panelHandle, PANEL_ARTIST, val);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColAlbum, ATTR_LABEL_TEXT, val);
		if (replaceUnicodeApostrophe)
			ReplaceUnicodeApostrophe(val);
		SetCtrlVal(panelHandle, PANEL_ALBUM, val);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColLabel, ATTR_LABEL_TEXT, val);
		if (replaceUnicodeApostrophe)
			ReplaceUnicodeApostrophe(val);
		SetCtrlVal(tab1Handle, TAB1_PUBLISHER, val);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColType, ATTR_LABEL_TEXT, val);
		SetTableCellVal(tab1Handle, TAB1_RELTYPE, MakePoint(1,1), val);
		
		GetCtrlAttribute(tab1Handle, TAB1_YEAR, ATTR_STRING_TEXT_LENGTH, &len);
		if (len) {	// check if the year we got from musicbrainz is less than the year already in the mp3
			GetCtrlVal(tab1Handle, TAB1_YEAR, val);
			oldYear = strtol(val, NULL, 10);
		}
		else {
			oldYear = 9999;
		}
		GetCtrlVal(tab1Handle, TAB1_DISCNUM, discNum);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColDate, ATTR_LABEL_TEXT, val);
		newYear = strtol(val, NULL, 10);
		if (newYear && newYear < oldYear) {
			SetCtrlVal(tab1Handle, TAB1_YEAR, val);
		}
			
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColNumDiscs, ATTR_LABEL_TEXT, val);
		releaseDiscs = strtol(val, NULL, 10);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColRelGroupID, ATTR_LABEL_TEXT, reid);
		SetCtrlVal(tab3Handle, TAB3_REID, reid);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColArtistID, ATTR_LABEL_TEXT, reid);
		SetCtrlVal(tab3Handle, TAB3_ARTISTMBID, reid);
		SetCtrlAttribute(panelHandle, PANEL_FANART, ATTR_DIMMED, 0);

		GetCountryFromArtist(reid);
		
		discSubtitles[0] = "";	// in case GetDiscSubtitles fails we don't want the program to crash
		GetDiscSubtitles(panel, albumIndex, discSubtitles);
		SetCtrlVal(tab1Handle, TAB1_DISCSUBTITLE, discSubtitles[0]);
		
		for (i=0;i<gAlbumInfo[albumIndex].numTracks;i++) {
			char *ptr;
			count = strtol(gAlbumInfo[albumIndex].tracks[i].trackNum, NULL, 10);
			disc = strtol(gAlbumInfo[albumIndex].tracks[i].discNum, NULL, 10);
			
			for (k=0;k<numTracks;k++) {
				GetTreeCellAttribute(panelHandle, PANEL_TREE, k, kTreeColTrackNum, ATTR_LABEL_TEXT_LENGTH, &len);
				if (len) {
					GetTreeCellAttribute(panelHandle, PANEL_TREE, k, kTreeColTrackNum, ATTR_LABEL_TEXT, offset);
					trackNum = strtol(offset, NULL, 10);
				} else if (dataHandle.trackNumPtr[k]) {
					trackNum = strtol(dataHandle.trackNumPtr[k], NULL, 10);
				} else {
					trackNum = k + 1;
				}
				
				if (dataHandle.discPtr[k] && gUseMetaDataDiscVal) {
					ptr = dataHandle.discPtr[k];
				} else if (discNum && isdigit(discNum[0])) {
					ptr = discNum;
				} else {
					ptr = "1";
				}
				if (trackNum == count && !strncmp(ptr, gAlbumInfo[albumIndex].tracks[i].discNum, strlen(gAlbumInfo[albumIndex].tracks[i].discNum))) {
					// We found the correct track, now save the values for it
					if (dataHandle.discSubtitlePtr[k])
						free(dataHandle.discSubtitlePtr[k]);
					dataHandle.discSubtitlePtr[k] = malloc(strlen(discSubtitles[disc-1]) + 1);
					strcpy(dataHandle.discSubtitlePtr[k], discSubtitles[disc-1]);
					SetTreeCellAttribute(panelHandle, PANEL_TREE, k, kTreeColTrackName, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].title);
					SetTreeCellAttribute(panelHandle, PANEL_TREE, k, kTreeColArtistName, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].artist);
					if (releaseDiscs > 1) {		// get an accurate value while we can here because it's used in file renaming
						if (dataHandle.discPtr[k])
							free(dataHandle.discPtr[k]);
						dataHandle.discPtr[k] = calloc(6, sizeof(char));	// allows for "99/99\0"
						sprintf(dataHandle.discPtr[k], releaseDiscs > 9 ? "%02d/%d\0" : "%d/%d" ,disc, releaseDiscs);
					}
					// copy artistFilter
					if (dataHandle.artistFilterPtr[k])
						free(dataHandle.artistFilterPtr[k]);
					GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColArtistSortOrder, ATTR_LABEL_TEXT, val);
					if (!strcmp(val, gAlbumInfo[albumIndex].tracks[i].artistFilter)) {
						strcpy(gAlbumInfo[albumIndex].tracks[i].artistFilter, "");
					}	
					StoreDataVals(tab1Handle, TAB1_ARTISTFILTER, gAlbumInfo[albumIndex].tracks[i].artistFilter, k);
					gUseMetaArtistFilter = true;
					SetCtrlVal(tab1Handle, TAB1_ARTISTFILTER, gAlbumInfo[albumIndex].tracks[i].artistFilter);
					if (strcmp(gAlbumInfo[albumIndex].tracks[0].artistFilter, gAlbumInfo[albumIndex].tracks[i].artistFilter) && 
					    strlen(gAlbumInfo[albumIndex].tracks[i].artistFilter)) {
						// compare current track artistFilter with first artistFilter and if they differ set conflict
						SetCtrlVal(tab1Handle, TAB1_ARTISTFILTERLED, true);
					}
					break;
				}
			}
		}
		for (i=0; i<99; i++) {
			if (discSubtitles[i] != NULL) {
				free(discSubtitles[i]);
			}
		}
		SetConflictTooltips(panelHandle);	// we just updated so show "conflicts"
	}

Error:
	if (curElem)
		CVIXMLDiscardElement (curElem);
	if (doc)
    	CVIXMLDiscardDocument (doc);
	return;
}


/*** Retrieves and formats the album track listing for the currently selected album in the album panel ***/
int GetAlbumTrackListing(int panel, int albumTree, int trackTree, int albumIndex)
{
	HRESULT error = S_OK;
	char	val[512];
	int		i, releaseTracks, len;
	
	DeleteListItem(panel, trackTree, 0, -1);
	if (albumIndex != -1) {
		SetCtrlAttribute(panel, trackTree, ATTR_DIMMED, 0);

		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColNumTracks, ATTR_LABEL_TEXT, val);
		releaseTracks = strtol(val, NULL, 10);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColType, ATTR_LABEL_TEXT, val);
		SetCtrlAttribute(panel, ALBUMPANEL_TXTTYPE, ATTR_CTRL_VAL, val);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColASIN, ATTR_LABEL_TEXT, val);
		SetCtrlAttribute(panel, ALBUMPANEL_TXTASIN, ATTR_CTRL_VAL, val);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColBarcode, ATTR_LABEL_TEXT, val);
		SetCtrlAttribute(panel, ALBUMPANEL_TXTBARCODE, ATTR_CTRL_VAL, val);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColCatalog, ATTR_LABEL_TEXT, val);
		SetCtrlAttribute(panel, ALBUMPANEL_TXTCATALOG, ATTR_CTRL_VAL, val);
		GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColLabel, ATTR_LABEL_TEXT, val);
		SetCtrlAttribute(panel, ALBUMPANEL_TXTLABEL, ATTR_CTRL_VAL, val);
		
		if (!gAlbumInfo[albumIndex].tracks) {
			hrChk(GetXMLAndPopulateTrackTree(panel, albumTree, trackTree, albumIndex));
			
			if (error == S_OK) {
				// store information from tree in gAlbumInfo
				gAlbumInfo[albumIndex].numTracks = releaseTracks;
				gAlbumInfo[albumIndex].tracks = calloc(releaseTracks, sizeof(TrackInfoStruct));
				for (i=0;i<releaseTracks;i++) {
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackNum, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].trackNum);
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackDisc, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].discNum);
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackLength, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].time);
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackName, ATTR_LABEL_TEXT_LENGTH, &len);
					gAlbumInfo[albumIndex].tracks[i].title = malloc(sizeof(char) * (len + 1));
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackName, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].title);
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackArtist, ATTR_LABEL_TEXT_LENGTH, &len);
					gAlbumInfo[albumIndex].tracks[i].artist = malloc(sizeof(char) * (len + 1));
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackArtist, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].artist);
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackArtistFilter, ATTR_LABEL_TEXT_LENGTH, &len);
					gAlbumInfo[albumIndex].tracks[i].artistFilter = malloc(sizeof(char) * (len + 1));
					GetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackArtistFilter, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].artistFilter);
				}
			}
		}
		else {	// we've already saved everything, so load it up into the tree
			SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_VISIBLE, FALSE);
			for (i=0;i<gAlbumInfo[albumIndex].numTracks;i++) {
				InsertTreeItem(panel, trackTree, VAL_SIBLING, i-1, VAL_LAST, gAlbumInfo[albumIndex].tracks[i].discNum, NULL, NULL, i);
				SetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackNum, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].trackNum);
				SetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackName, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].title);
				SetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackLength, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].time);
				SetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackArtist, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].artist);
				SetTreeCellAttribute(panel, trackTree, i, kTrackTreeColTrackArtistFilter, ATTR_LABEL_TEXT, gAlbumInfo[albumIndex].tracks[i].artistFilter);
			}
		}
	}
	else
		SetCtrlAttribute(panel, trackTree, ATTR_DIMMED, 1);
Error:
	return albumIndex;
}

int GetArtistName(CVIXMLElement *curElem, int index, char *artist, char *artistID, int replaceApostrophe, int sortName, int addJoinPhrase)
{
	char joinphrase[25];
	HRESULT error = S_OK;

	hrChk(GetChildElementByIndex(curElem, index));	// name-credit
	hrChk(GetChildElementByTag(curElem, "artist"));
	if (artistID) {	// can be NULL
		GetAttributeByName(*curElem, "id", artistID);
	}
	if (sortName) {
		hrChk(GetChildElementByTag(curElem, "sort-name"));
	} else {
		hrChk(GetChildElementByTag(curElem, "name"));
	}
	hrChk(CVIXMLGetElementValue(*curElem, artist));
	if (replaceApostrophe)
		ReplaceUnicodeApostrophe(artist);
	GetParentElement(curElem);		// name or sort-name
	GetParentElement(curElem);		// artist
	if (addJoinPhrase) {
		strcpy(joinphrase, "");
		GetAttributeByName(*curElem, "joinphrase", joinphrase);
		strcat(artist, joinphrase);
	}
	GetParentElement(curElem); 		// name-credit

Error:
	return error;
}


/* Requests and parses the XML file and then populates trackTree with the individual track information */
int GetXMLAndPopulateTrackTree(int panel, int albumTree, int trackTree, int albumIndex)
{
	HRESULT error = S_OK;
	char	val[512], title[512], reid[40], queryBuf[512], artist[512], discStr[4], time[8], fileName[MAX_PATHNAME_LEN];
	int		i, j, numTracks, mins, secs, replaceUnicodeApostrophe=0;
	char	position[5], artistFilter[512];
	int		numDiscs, count = 0, numArtists = 0, index;
	CVIXMLDocument	doc = 0;
	CVIXMLElement 	curElem = 0;

	SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_VISIBLE, FALSE);
	GetCtrlVal(configHandle, OPTIONS_REPLACEAPOSTROPHE, &replaceUnicodeApostrophe);
	GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColNumTracks, ATTR_LABEL_TEXT, val);
	GetTreeCellAttribute(panel, ALBUMPANEL_ALBUMTREE, albumIndex, kAlbTreeColREID, ATTR_LABEL_TEXT, reid);
	sprintf(queryBuf, kTrackQuery, reid);
	BuildFileNameFromQuery(queryBuf, fileName);	// create filename to save XML in
	DownloadFileIfNotExists(queryBuf, fileName);

	error = CVIXMLLoadDocument(fileName, &doc);
	if (error == S_OK) {
		hrChk (CVIXMLGetRootElement (doc, &curElem));
		CVIXMLGetElementTag(curElem, val);
		if (!strcmp(val, "error")) {
			if (!GetChildElementByTag(&curElem, "text")) {
				CVIXMLGetElementValue(curElem, val);
				SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_LABEL_TEXT, val);
			}
			SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_VISIBLE, TRUE);
			error = S_FALSE;
			DeleteFile(fileName);
		} else {
			hrChk (GetChildElementByIndex(&curElem, 0));	// release
			hrChk (GetChildElementByTag(&curElem, "medium-list"));
			GetAttributeByName(curElem, "count", val);
			numDiscs = strtol(val, NULL, 10);
			for (i=0;i<numDiscs;i++) {
				hrChk (GetChildElementByIndex(&curElem, i));	// medium
				GetChildElementByTag(&curElem, "position");
				CVIXMLGetElementValue(curElem, discStr);		// get the disc #
				GetParentElement(&curElem);						// position
				hrChk (GetChildElementByTag(&curElem, "track-list"));
				GetAttributeByName(curElem, "count", val); 
				numTracks = strtol(val, NULL, 10);
				
				for (j=0;j<numTracks;j++) {
					hrChk(GetChildElementByIndex(&curElem, j));	// track
					hrChk(GetChildElementByTag(&curElem, "position"));	// position
					hrChk(CVIXMLGetElementValue(curElem, position));
					GetParentElement(&curElem);
					
					hrChk(GetChildElementByTag(&curElem, "recording"));
					if (!GetChildElementByTag(&curElem, "artist-credit")) {
						CVIXMLGetNumChildElements(curElem, &numArtists);
						GetArtistName(&curElem, 0, artist, NULL, replaceUnicodeApostrophe, false, false);
						strcpy(artistFilter, "");
						for (index=0; index<numArtists; index++) {
							GetArtistName(&curElem, index, val, NULL, replaceUnicodeApostrophe, true, false);
							strcat(artistFilter, val);
							if (index+1 < numArtists) {
								strcat(artistFilter, "; ");
							}
						}
						GetParentElement(&curElem); 	// artist-credit
					}
					mins = secs = 0; 
					if (!GetChildElementByTag(&curElem, "length")) {
						hrChk(CVIXMLGetElementValue(curElem, val));
						GetParentElement(&curElem); /* length */
						secs = (strtol(val, NULL, 10) + 500) / 1000;
						mins = secs/60;
						secs = secs%60;
					}
					if (!GetChildElementByTag(&curElem, "title")) {
						hrChk(CVIXMLGetElementValue(curElem, title));
						if (replaceUnicodeApostrophe)
							ReplaceUnicodeApostrophe(title);
						GetParentElement(&curElem); /* title */
					}
					GetParentElement(&curElem); // recording
					GetParentElement(&curElem);

					InsertTreeItem(panel, trackTree, VAL_SIBLING, count-1, VAL_LAST, "", NULL, NULL, count);
					SetTreeCellAttribute(panel, trackTree, count, kTrackTreeColTrackNum, ATTR_LABEL_TEXT, position);
					SetTreeCellAttribute(panel, trackTree, count, kTrackTreeColTrackDisc, ATTR_LABEL_TEXT, discStr);
					SetTreeCellAttribute(panel, trackTree, count, kTrackTreeColTrackName, ATTR_LABEL_TEXT, title);
					SetTreeCellAttribute(panel, trackTree, count, kTrackTreeColTrackArtist, ATTR_LABEL_TEXT, artist);
					SetTreeCellAttribute(panel, trackTree, count, kTrackTreeColTrackArtistFilter, ATTR_LABEL_TEXT, artistFilter);
					sprintf(time, "%d:%02d\0", mins, secs);
					SetTreeCellAttribute(panel, trackTree, count, kTrackTreeColTrackLength, ATTR_LABEL_TEXT, time);
					count++;
				}
				
				GetParentElement(&curElem);	// track-list 
				GetParentElement(&curElem);	// medium
			}
			GetParentElement(&curElem);	// medium-list
			GetParentElement(&curElem);	// release
		}
	} else {
		SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_LABEL_TEXT, "An error occurred");
		SetCtrlAttribute(panel, ALBUMPANEL_ERROR_ICON, ATTR_VISIBLE, 1);
		DeleteFile(fileName);	// an error occurred so don't save file
	}

Error:
	if (curElem)
		CVIXMLDiscardElement (curElem);
	if (doc)
    	CVIXMLDiscardDocument (doc);
	return error;
}


int CVICALLBACK AlbumTreeSortCI(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	char *str1=NULL, *str2=NULL;
	int len, val=0, i;

	for (i=kAlbTreeColArtist;i<kAlbTreeColDate;i++)
		{
		GetTreeCellAttribute(panel, control, item1, i, ATTR_LABEL_TEXT_LENGTH, &len);
		str1 = malloc(sizeof(char) * len+1);
		GetTreeCellAttribute(panel, control, item1, i, ATTR_LABEL_TEXT, str1);

		GetTreeCellAttribute(panel, control, item2, i, ATTR_LABEL_TEXT_LENGTH, &len);
		str2 = malloc(sizeof(char) * len+1);
		GetTreeCellAttribute(panel, control, item2, i, ATTR_LABEL_TEXT, str2);
	
		val = CompareStrings (str1, 0, str2, 0, 0);
	
		free(str1);
		free(str2);
		
		if (val)
			break;
		}

	return val;
}

int CVICALLBACK TrackTreeSortCI(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	char str1[5], str2[5];
	int val=0, i, num1, num2;

	for (i=kTrackTreeColTrackDisc;i<kTrackTreeColTrackName;i++)
		{
		GetTreeCellAttribute(panel, control, item1, i, ATTR_LABEL_TEXT, str1);
		GetTreeCellAttribute(panel, control, item2, i, ATTR_LABEL_TEXT, str2);
		num1 = strtol(str1, NULL, 10);
		num2 = strtol(str2, NULL, 10);
	
		if (num1 < num2) {
			val = -1;
			break;
			}
		else if (num1 > num2) {
			val = 1;
			break;
			}
		}

	return val;
}

void FreeAlbumInfo(void)
{
	for (int i=0;i<gAlbumCount;i++) {
		if (gAlbumInfo[i].tracks)  {
			for (int j=0;j<gAlbumInfo[i].numTracks;j++) {
				free (gAlbumInfo[i].tracks[j].title);
				free (gAlbumInfo[i].tracks[j].artist);
			}
			free (gAlbumInfo[i].tracks);
		}
	}
	free (gAlbumInfo);
	gAlbumInfo = NULL;
	gAlbumCount = 0;
	
	return;
}

/************************************************************************/
// Threading stuff
static int CVICALLBACK TrackThreadFunc (void *functionData)
{
	while (!gExiting) {
		if (gSelectedAlbum != -1 && gSelectedAlbum != lastAlbumSelected) {
			lastAlbumSelected = GetAlbumTrackListing(albumPanHandle, ALBUMPANEL_ALBUMTREE, ALBUMPANEL_TRACKTREE, 
				gSelectedAlbum);
		}
		else if (gSelectedAlbum == -1)
			lastAlbumSelected = -1;
		Delay(0.15);
	}
	
	return 0;
}

void ExitDialog()
{
	gExiting = 1;
	/* Wait for the thread functions to finish executing */
	CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE, trackThreadFuncId, 
		OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	/* Release thread functions */
	CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, trackThreadFuncId);
	trackThreadFuncId = 0;
	FreeAlbumInfo();
	
	RemovePopup(0);
}

/************************************************************************/

int CVICALLBACK AlbumTreeCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Point	point;
	int		selectedItem, index, column, area;
	static 	lastSortedCol=0;
	
	switch (event)
		{
		case EVENT_LEFT_CLICK:
			point.x = eventData2;
			point.y = eventData1;
			GetIndexFromPoint(panel, control, point, &index, &area, &column);
			if (area == VAL_COLUMN_LABEL_AREA) {
				gSelectedAlbum = -1;
				SortTreeItems(panel, control, 0, column, lastSortedCol == column ? TRUE : FALSE, TRUE, NULL, NULL);
				if (lastSortedCol == column)
					lastSortedCol = 0;
				else
					lastSortedCol = column;
				SetCtrlIndex(albumPanHandle, ALBUMPANEL_ALBUMTREE, 0);
				gSelectedAlbum = 0;		// should force the tracks to update
			}
//			else if (area == VAL_CELL_AREA && gSelectedAlbum == -1 && index == 1)	// they can click on the first
//				gSelectedAlbum = index;
			break;
		case EVENT_VAL_CHANGED:
			if (eventData1 == ACTIVE_ITEM_CHANGE) {
				GetCtrlVal(panel, control, &selectedItem);
				GetIndexFromValue(panel, control, &gSelectedAlbum, selectedItem);
			}
			break;
		case EVENT_LEFT_DOUBLE_CLICK:
			point.x = eventData2;
			point.y = eventData1;
			GetIndexFromPoint(panel, control, point, &index, &area, &column);
			if (area == VAL_CELL_AREA) {
				GetMetaTrackData(panel, index);
				ExitDialog();
				}
			break;
		}
	return 0;
}

int CVICALLBACK CancelCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			lastAlbumSelected = -1;
			ExitDialog();
			break;
		}
	return 0;
}

int CVICALLBACK OffsetCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if (control == ALBUMPANEL_NEXT)
				offset += 100;
			else
				offset -= 100;
			if (offset < 0)
				offset = 0;
			DeleteListItem(albumPanHandle, ALBUMPANEL_ALBUMTREE, 0, -1);
			GetMetaData(panel, control);			
			break;
	}
	return 0;
}

int CVICALLBACK AlbumPanelOKCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int count, index, selectedItem;
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetNumListItems(panel, ALBUMPANEL_ALBUMTREE, &count);
			if (count > 0) {
				GetCtrlVal(panel, ALBUMPANEL_ALBUMTREE, &selectedItem);
				GetIndexFromValue(panel, ALBUMPANEL_ALBUMTREE, &index, selectedItem);
				GetMetaTrackData(panel, index);
			}
			lastAlbumSelected = -1;
			ExitDialog();
			break;
	}
	return 0;
}

int CVICALLBACK MusicBrainzPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_KEYPRESS:
			if (eventData1 != VAL_ESC_VKEY)
				break;
			/* fall through on escape */
		case EVENT_CLOSE:
			ExitDialog();
			break;
	}
	return 0;
}

int CVICALLBACK LaunchMBCB (int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	char url[100], mbid[38];
	
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			switch (control) {
				case TAB3_MUSICBRAINZ_ARTIST:
					GetCtrlVal(tab3Handle, TAB3_ARTISTMBID, mbid);
					sprintf(url, "http://musicbrainz.org/artist/%s", mbid);
					break;
				case TAB3_MUSICBRAINZ_RELEASE:
				default:
					GetCtrlVal(tab3Handle, TAB3_REID, mbid);
					sprintf(url, "http://musicbrainz.org/release-group/%s", mbid);
					break;
			}
			OpenDocumentInDefaultViewer(url, VAL_NO_ZOOM);
			break;
	}
	return 0;
}
