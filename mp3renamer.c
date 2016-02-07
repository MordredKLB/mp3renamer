#include "pathctrl.h"
#include "string.h"
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>
#include "mp3renamer.h"
#include "ID3v2.h"
#include "apev2.h"
#include "genreList.h"
#include "titleformatting.h"
#include "progressbar.h"
#include "inifile.h"
#include "combobox.h"
#include "globals.h"

/***************************/
/*** Function Prototypes ***/
/***************************/
void ClearFileStruct(void);
void ClearFileList(void);
void ClearID3DataStruct(int numSongs);
void ClearFields(void);
void ClearLEDs(void);
void CheckUpdateBoxes(void);
void initID3DataStruct(int numSongs);
int GetGenreIndex(char *genre);
void RenameFiles(void);
void RemoveUnderscores(void);
void RemoveYear(char *filename);
void SortFileNames(void);
void ReplaceUnusableChars(char *string);
int  GetNumericTrackNum(char *filename, int index);
void BuildTreeContextMenu(int panel, int control);
void PopulateTrackData(void);
void SetupSplitters(int panel);
int  isSpecialChar(char ch);
void SetMetaDataButtonDimming(int panel, int undim);
void ClearRedundantFieldsIfNeeded(int panel);

void ParseRenameString(char *filename, char *formatStr, int index);

int SetID3v1Tag(int panel, char *filename, char *newname, int index);
int GetID3v1Tag(int panel, char *filename);

/***************************/
/*** Callbacks           ***/
/***************************/
void CVICALLBACK CheckAllMenuItems (int panel, int control, int MenuItemID, void *callbackData);
void CVICALLBACK UncheckAllMenuItems (int panel, int control, int MenuItemID, void *callbackData);
void CVICALLBACK CheckInverseMenuItems (int panel, int control, int MenuItemID, void *callbackData);
int CVICALLBACK TreeSortCI(int panel, int control, int item1, int item2, int keyCol, void *callbackData);

/***************************/
/*** Globals             ***/
/***************************/

int firstFile = 0;

static char **fileList = NULL;
extern char *genreList[kNumWinampGenres];
extern const Point tagCell;
static int gRetrievedFolderJpgData = 0;

char panelLEDList[2] = {PANEL_ARTISTLED, PANEL_ALBUMLED};
char tab1LEDList[kNumTab1Controls] = {TAB1_GENRELED, TAB1_COMMENTLED, TAB1_YEARLED, TAB1_DISCNUMLED, TAB1_COMPOSERLED, 
						TAB1_PUBLISHERLED, TAB1_EDITIONLED, TAB1_COUNTRYLED, TAB1_RELTYPELED, TAB1_ALBUMARTISTLED, 
						TAB1_ARTISTFILTERLED, TAB1_PERFSORTLED, TAB1_ALBUMGAINLED, TAB1_ALBUMSORTLED};
char tab2LEDList[kNumTab2Controls] = {TAB2_ORIGARTISTLED, TAB2_URLLED, TAB2_COPYRIGHTLED, TAB2_ENCODEDLED};

char panelUpdateList[2] = {PANEL_UPDATEARTIST, PANEL_UPDATEALBUM};
char tab1UpdateList[kNumTab1Controls] = {TAB1_UPDATEGENRE, TAB1_UPDATECOMMENT, TAB1_UPDATEYEAR, TAB1_UPDATEDISCNUM, TAB1_UPDATECOMPOSER, 
										 TAB1_UPDATEPUBLISHER, TAB1_UPDATEALBUMARTIST, TAB1_UPDATEARTISTFILTER, TAB1_UPDATEEDITION, 
										 TAB1_UPDATECOUNTRY, TAB1_UPDATERELTYPE, TAB1_UPDATEPERFSORT, TAB1_UPDATEALBUMGAIN, TAB1_UPDATEALBUMSORT};
char tab2UpdateList[kNumTab2Controls] = {TAB2_UPDATEORIGARTIST, TAB2_UPDATEURL, TAB2_UPDATECOPYRIGHT, TAB2_UPDATEENCODED};
char tab3UpdateList[kNumTab3Controls] = {TAB3_UPDATEMBID, TAB3_UPDATEREID};

#define kNumWords			21
#define kNumCommonWords		5
char wordList[kNumWords][20] = {
				"-\0", "a\0","an\0","the\0","and\0","of\0",	// these first five words are also used by the musicbrainz fuzzy search in ReplaceCommonWords()
				"but\0","as\0","or\0","for\0","nor\0","at\0","by\0",
				"etc.\0","in\0","n'\0","o'\0","on\0",
				"to\0","vs\0","vs.\0"};

#define kNumEditions		23
char albumEditions[kNumEditions][25] = {
				"10th Anniversary Edition\0", "20th Anniversary Edition\0", "25th Anniversary Edition\0",
				"7\" Vinyl\0", "Bonus Disc\0", "Deluxe Edition\0", "Disc 1\0", "Disc 2\0", "Disc 3\0", "Disc 4\0",
				"Disc One\0", "Disc Two\0", "Disc Three\0", "Disc Four\0", "Expanded Edition\0", 
				"Japan Edition\0", "Japanese Edition\0", "Limited Edition\0", "Ltd. Edition\0", 
				"Remastered Edition\0", "Special Edition\0", "UK Edition\0", "Vinyl Edition\0"}; 

TreeCellCompareCallbackPtr TreeCellComparePtr;

/***************************/

int main (int argc, char *argv[])
{
	char path[MAX_PATHNAME_LEN];
	
	
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "mp3renamer.uir", PANEL)) < 0)
		return -1;
	SetupSplitters(panelHandle);
	progressHandle = LoadPanel (0, "mp3renamer.uir", PROGRESS);
	configHandle = LoadPanel (0, "mp3renamer.uir", OPTIONS);
	//correctionHandle = LoadPanel (0, "mp3renamer.uir", SPELLING);
	albumPanHandle = LoadPanel (0, "mp3renamer.uir", ALBUMPANEL);
	fanartPanHandle = LoadPanel (0, "mp3renamer.uir", FANART);
	hdlogoPanHandle = LoadPanel (fanartPanHandle, "mp3renamer.uir", HDLOGO);
	cdartPanHandle	= LoadPanel (fanartPanHandle, "mp3renamer.uir", CDART);
	hdPreviewHandle = LoadPanel (0, "mp3renamer.uir", HDPREVIEW);
	pMenuHandle = LoadMenuBar(0, "mp3renamer.uir", PMENU);
	numFiles = 0;
	
	if (argc == 2) {
		GetProjectDir(path);
		SetDir(path);
		}
	
	ProgressBar_ConvertFromSlide(progressHandle, PROGRESS_PROGRESSBAR);
	ProgressBar_ConvertFromSlide(albumPanHandle, ALBUMPANEL_PROGRESSBAR);
	ProgressBar_ConvertFromSlide(fanartPanHandle, FANART_PROGRESSBAR);
	ReadConfigOptions(FALSE);	// load from MP3Renamer.ini
	GetPanelHandleFromTabPage(panelHandle, PANEL_TAB, 0, &tab1Handle);
	GetPanelHandleFromTabPage(panelHandle, PANEL_TAB, 1, &tab2Handle);
	GetPanelHandleFromTabPage(panelHandle, PANEL_TAB, 2, &tab3Handle);
	InitUIAttrs();			// sets attributes for UI Controls
	BuildTreeContextMenu(panelHandle, PANEL_TREE);
	BuildTreeContextMenu(tab3Handle, TAB3_EXTENDEDTAGS);
	ShowLengthCB (configHandle, OPTIONS_SHOWLENGTH, EVENT_COMMIT, NULL, 0, 0);	// force updating tree ctrl
	ID3v1CB (panelHandle, PANEL_DOID3V1, EVENT_COMMIT, NULL, 0, 0);
	CapitalizeCB(panelHandle, PANEL_CAPITALIZE, EVENT_COMMIT, NULL, 0, 0);
#if !_CVI_DEBUG_	
	SetTreeColumnAttribute(panelHandle, PANEL_TREE, kTreeColID, ATTR_COLUMN_VISIBLE, 0);		// hide index numbers
#endif
	SetToolTipInfo();
	DisplayPanel (panelHandle);
	SetupMetaDataStruct();
	SetupFormatFunctions();
	AddToFilePopupDirHistory(startFolder);
	
	if (argc == 2) {
		strncpy(path, argv[1], strlen(argv[1]) - strlen(strrchr(argv[1],'\\'))+1);
		path[strlen(argv[1]) - strlen(strrchr(argv[1],'\\'))+1]='\0';
		BrowseCB(panelHandle, PANEL_BrowseButton, EVENT_COMMIT, &path, 0, 0);
	}
	
	RunUserInterface ();
	
	ProgressBar_Revert(progressHandle, PROGRESS_PROGRESSBAR);
	ProgressBar_Revert(albumPanHandle, ALBUMPANEL_PROGRESSBAR);
	DiscardMenuBar (pMenuHandle);
	DiscardPanel (panelHandle);
	DiscardPanel (progressHandle);
	DiscardPanel (configHandle);
	DiscardPanel (albumPanHandle);
	DiscardPanel (hdlogoPanHandle);
	DiscardPanel (cdartPanHandle);
	DiscardPanel (fanartPanHandle);
	DiscardPanel (hdPreviewHandle);
	return 0;
}

int CVICALLBACK QuitCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			OptionsOKCB(configHandle, control, EVENT_COMMIT, NULL, 0, 0); 	// commit front panel options
			ClearID3DataStruct(numFiles);
			ClearFileList();	// clear after ClearID3DataStructure, because this resets numFiles
			QuitUserInterface (0);
			break;
		}
	return 0;
}


/****************************************/

void BuildTreeContextMenu(int panel, int control)
{
	NewCtrlMenuItem(panel, control, "Uncheck All", VAL_FIND, UncheckAllMenuItems, NULL);
	NewCtrlMenuItem(panel, control, "Check All", VAL_FIND, CheckAllMenuItems, NULL);
	NewCtrlMenuItem(panel, control, "Check Inverse", VAL_FIND, CheckInverseMenuItems, NULL);
	HideBuiltInCtrlMenuItem(panel, control, VAL_SORT);
	HideBuiltInCtrlMenuItem(panel, control, VAL_EXPAND_SUBTREE);
	HideBuiltInCtrlMenuItem(panel, control, VAL_COLLAPSE_SUBTREE);
	HideBuiltInCtrlMenuItem(panel, control, VAL_EXPAND_ALL);
	HideBuiltInCtrlMenuItem(panel, control, VAL_COLLAPSE_ALL);
	
}

void CVICALLBACK CheckAllMenuItems (int panel, int control, int MenuItemID, void *callbackData)
{
	int count, i;
	
	GetNumListItems(panel, control, &count);
	for (i=0;i<count;i++)
		CheckListItem(panel, control, i, 1);
}

void CVICALLBACK UncheckAllMenuItems (int panel, int control, int MenuItemID, void *callbackData)
{
	int count, i;
	
	GetNumListItems(panel, control, &count);
	for (i=0;i<count;i++)
		CheckListItem(panel, control, i, 0);
}

void CVICALLBACK CheckInverseMenuItems (int panel, int control, int MenuItemID, void *callbackData)
{
	int count, i, checked;
	
	GetNumListItems(panel, control, &count);
	for (i=0;i<count;i++) {
		IsListItemChecked(panel, control, i, &checked);
		CheckListItem(panel, control, i, !checked);
		}
}

/**********************************************************/

int AutoLoadFiles(char *path)
{
	int i=0;
	
	fileList[i] = malloc(sizeof(char) * MAX_FILENAME_LEN);
	GetFirstFile(path, 1, 0, 0, 0, 0, 0, fileList[i]);
	return 0;
}

/**********************************************************/

int CVICALLBACK BrowseCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int	status = 0;
	int i=0, getTag, len;
	size_t	subLen;
	char titleName[13 + MAX_PATHNAME_LEN];
	char searchFormat[MAX_PATHNAME_LEN + 10];

	switch (event)
		{
		case EVENT_COMMIT:
			ClearID3DataStruct(numFiles);
			ClearFileList();	// clear after ClearID3DataStructure, because this resets numFiles
			gUseMetaDataDiscVal = FALSE;
			if (callbackData) {
				char filename[MAX_FILENAME_LEN];
				
				strcpy(searchFormat, (char *)callbackData);
				strcat(searchFormat, "*.mp3");
				fileList = malloc(sizeof(char *) * kMaxFiles);
				fileList[i] = malloc(sizeof(char) * MAX_FILENAME_LEN + MAX_PATHNAME_LEN);
				status = GetFirstFile(searchFormat, 1, 0, 0, 0, 0, 0, filename);
				if (status>=0) {
					status = VAL_EXISTING_FILE_SELECTED;
					strcpy(fileList[i], (char *)callbackData);
					strcat(fileList[i], filename);
					numFiles++;
					while(!GetNextFile(filename)) {
						i++;
						fileList[i] = malloc(sizeof(char) * MAX_FILENAME_LEN + MAX_PATHNAME_LEN);
						strcpy(fileList[i], (char *)callbackData);
						strcat(fileList[i], filename);
						numFiles++;
						}
					}
				else
					free(fileList[i]);
				}
			else
#if _CVI_ <= 1001 
				status = MultiFileSelectPopup (startFolder, "*.mp3", "*.mp3;",
						   "Select Files to Rename", 0, 0, 1, &numFiles, &fileList);
#else
				status = MultiFileSelectPopupEx (startFolder, "*.mp3", "Audio Files (*.mp3;*.ac3;*.dts);*.*",
						   "Select Files to Rename", 0, 0, &numFiles, &fileList);
#endif
			if (status == VAL_EXISTING_FILE_SELECTED) {
				subLen = strlen(strrchr(fileList[0],'\\'));
				strncpy(startFolder,fileList[0],strlen(fileList[0])-subLen);
				startFolder[strlen(fileList[0])-subLen]='\0';
				strcpy(pathName,startFolder);	/* needed to remove path from Tree names */
				AddToFilePopupDirHistory(pathName);
				subLen = strlen(startFolder)-strlen(strrchr(startFolder,'\\'));
				startFolder[subLen]='\0';

				ClearFileStruct();
				DeleteListItem (panel, PANEL_TREE, 0, -1);
				for (i=0; i<numFiles; i++) {
					strcpy(fileStruct[i].origName, fileList[i]);
					strcpy(fileStruct[i].origFileName, fileList[i] + strlen(pathName)+1);
					strcpy(fileStruct[i].newName, fileList[i] + strlen(pathName) + 1);
					InsertTreeItem (panel, PANEL_TREE, VAL_SIBLING, i-1, VAL_NEXT,
						fileStruct[i].newName, "", NULL, i);
					SetTreeItemAttribute (panel, PANEL_TREE, i, ATTR_MARK_STATE, VAL_MARK_ON);
				}
				sprintf(titleName, "MP3Renamer - %s", pathName);
				SetPanelAttribute (panel, ATTR_TITLE, titleName);
				SetCtrlVal (panel, PANEL_REMOVESTRING, "");
				SetCtrlVal (panel, PANEL_REMOVENUM, 0);
				SetCtrlVal (panel, PANEL_REMOVENUMSTART, 0);
				SortFileNames();
				SortTreeItems (panel, PANEL_TREE, 0, 0, 0, 0, TreeSortCI, 0);
				ClearFields();
				ClearLEDs();
				CheckUpdateBoxes();
				GetCtrlVal(configHandle, OPTIONS_AUTOGETTAG, &getTag);
				if (getTag)
					GetID3Tag(panel, PANEL_GETID3BUTTON, EVENT_COMMIT, NULL, 0, 0);
				SetCtrlAttribute (panel, PANEL_SETBUTTON, ATTR_DIMMED, 0);
				SetCtrlAttribute (panel, PANEL_GETID3BUTTON, ATTR_DIMMED, 0);
				SetCtrlAttribute (panel, PANEL_ID3BUTTON, ATTR_DIMMED, 0);
				SetCtrlAttribute (tab1Handle, TAB1_GUESSBUTTON, ATTR_DIMMED, 0);
				GetCtrlAttribute (tab3Handle, TAB3_ARTISTMBID, ATTR_STRING_TEXT_LENGTH, &len);
				GetCtrlAttribute (tab3Handle, TAB3_REID, ATTR_STRING_TEXT_LENGTH, &subLen);
				SetCtrlAttribute (panel, PANEL_FANART, ATTR_DIMMED, (i==0 || subLen == 0));	// dimmed until we retrieve metadata if the tag didn't contain MBID data
				}
			else {
				ClearFileStruct();
				SetPanelAttribute (panel, ATTR_TITLE, "MP3 Renamer");
				DeleteListItem (panel, PANEL_TREE, 0, -1);
				SetCtrlAttribute (panel, PANEL_SETBUTTON, ATTR_DIMMED, 1);
				SetCtrlAttribute (panel, PANEL_GETID3BUTTON, ATTR_DIMMED, 1);
				SetCtrlAttribute (panel, PANEL_ID3BUTTON, ATTR_DIMMED, 1);
				SetCtrlAttribute (tab1Handle, TAB1_GUESSBUTTON, ATTR_DIMMED, 1);
				SetCtrlAttribute (panel, PANEL_FANART, ATTR_DIMMED, 1);
				ClearFields();
				ClearLEDs();
				}
			break;
		}
	return 0;
}

int CVICALLBACK SetNamesCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i;
	char new[MAX_PATHNAME_LEN + MAX_FILENAME_LEN];
	
	switch (event)
		{
		case EVENT_COMMIT:
			for (i=0;i<numFiles;i++) 
				if (IsItemChecked(i)) {
					sprintf(new, "%s\\%s", pathName, fileStruct[i].newName);
					sprintf(fileStruct[i].origFileName, "%s.tmp",fileStruct[i].origFileName);// temporarily rename so we can change uppercase/lowercase
					rename(fileStruct[i].origName, new);
					strcpy(fileStruct[i].origName, new);
					strcpy(fileStruct[i].origFileName, new + strlen(pathName) + 1);
					}
			break;
		}
	return 0;
}

int CVICALLBACK PreviewNames (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int autoRevert;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(configHandle, OPTIONS_AUTOREVERT, &autoRevert);
			if (autoRevert)
				RevertNames(panel, control, EVENT_COMMIT, callbackData, eventData1, eventData2);
			RenameFiles();
			SortFileNames();
			SortTreeItems (panel, PANEL_TREE, 0, 0, 0, 0, TreeSortCI, 0);
			ProcessDrawEvents();
			//PopulateTrackData();
			SetCtrlAttribute(panel, PANEL_RENAMEFOLDER, ATTR_DIMMED, 0);
			break;
		}
	return 0;
}

void ClearFileStruct()
{
	int i;
	
	for (i=0;i<kMaxFiles;i++) {
		strcpy(fileStruct[i].origName, "\0");
		strcpy(fileStruct[i].newName, "\0");
	}
}	  

void ClearFileList()
{
	int i;
	if (fileList) {
	     /* Using for loop to iterate through selected files */
	     for (i=0; i<numFiles; ++i) {
	       free (fileList [i]);
	       fileList [i] = NULL;
	     }
	     free (fileList);
	     fileList = NULL;
	}
	numFiles = 0;
}

void ClearFields()
{
	char buf[100];
	
	ClearID3Fields();
	sprintf(buf, kPictureSizeStr, 0, 0);
	SetCtrlVal(tab2Handle, TAB2_IMAGESIZEMSG, buf);
	DeleteListItem (tab3Handle, TAB3_EXTENDEDTAGS, 0, -1);	// clear extended tags tree
	SetCtrlVal(panelHandle, PANEL_IMAGERING, 0);
	ImageRingCB(panelHandle, PANEL_IMAGERING, EVENT_COMMIT, NULL, 1, 0); // pass eventData1 = 1 to prevent opening AlbumArtDownloader
	gRetrievedFolderJpgData = 0;
	SetCtrlAttribute(panelHandle, PANEL_DLARTWORKBUTTON, ATTR_DISABLE_CTRL_TOOLTIP, 1);
	SetCtrlAttribute(panelHandle, PANEL_IMAGERING, ATTR_DISABLE_CTRL_TOOLTIP, 1);
}

void ClearLEDs()
{
	int i;
	
	for (i=0;i<sizeof(panelLEDList);i++) {
		SetCtrlVal(panelHandle, panelLEDList[i], 0);
		SetCtrlAttribute(panelHandle, panelLEDList[i], ATTR_DISABLE_CTRL_TOOLTIP, 1);
		}
	for (i=0;i<sizeof(tab1LEDList);i++) {
		SetCtrlVal(tab1Handle, tab1LEDList[i], 0);
		SetCtrlAttribute(tab1Handle, tab1LEDList[i], ATTR_DISABLE_CTRL_TOOLTIP, 1);
		}
	for (i=0;i<sizeof(tab2LEDList);i++) {
		SetCtrlVal(tab2Handle, tab2LEDList[i], 0);
		SetCtrlAttribute(tab2Handle, tab2LEDList[i], ATTR_DISABLE_CTRL_TOOLTIP, 1);
		}
	SetCtrlVal(panelHandle, PANEL_TABVALS, 0);
	SetCtrlVal(panelHandle, PANEL_TABVALS2, 0);
}

void CheckUpdateBoxes()
{
	int i;
	for (i=0;i<sizeof(panelLEDList);i++) {
		SetCtrlVal(panelHandle, panelUpdateList[i], 1);
		}
	for (i=0;i<sizeof(tab1LEDList);i++) {
		SetCtrlVal(tab1Handle, tab1UpdateList[i], 1);
		}
	for (i=0;i<sizeof(tab2LEDList);i++) {
		SetCtrlVal(tab2Handle, tab2UpdateList[i], 1);
		}
	for (i=0;i<kNumTab3Controls;i++) {
		SetCtrlVal(tab3Handle, tab3UpdateList[i], 1);
		}
}

void initID3DataStruct(int numSongs)
{
	ClearID3DataStruct(numSongs); // do this first to make sure we don't have a memory leak
	dataHandle.artistPtr = calloc(numFiles, sizeof(char *));
	dataHandle.albumPtr = calloc(numFiles, sizeof(char *));
	dataHandle.trackNumPtr = calloc(numFiles, sizeof(char *));
	dataHandle.genrePtr = calloc(numFiles, sizeof(char *));
	dataHandle.commentPtr = calloc(numFiles, sizeof(char *));
	dataHandle.yearPtr = calloc(numFiles, sizeof(char *));
	dataHandle.discPtr = calloc(numFiles, sizeof(char *));
	dataHandle.composerPtr = calloc(numFiles, sizeof(char *));
	dataHandle.copyrightPtr = calloc(numFiles, sizeof(char *));
	dataHandle.urlPtr = calloc(numFiles, sizeof(char *));
	dataHandle.encodedPtr = calloc(numFiles, sizeof(char *));
	dataHandle.countryPtr = calloc(numFiles, sizeof(char *));
	dataHandle.relTypePtr = calloc(numFiles, sizeof(char *));
	dataHandle.origArtistPtr = calloc(numFiles, sizeof(char *));
	dataHandle.albumArtistPtr = calloc(numFiles, sizeof(char *));
	dataHandle.artistFilterPtr = calloc(numFiles, sizeof(char *));
	dataHandle.publisherPtr = calloc(numFiles, sizeof(char *));
	dataHandle.editionPtr = calloc(numFiles, sizeof(char *));
	dataHandle.albumGainPtr = calloc(numFiles, sizeof(char *));
	dataHandle.albSortOrderPtr = calloc(numFiles, sizeof(char *));
	dataHandle.perfSortOrderPtr = calloc(numFiles, sizeof(char *));
}

void ClearID3DataStruct(int numSongs)
{
	int i;

	if (!dataHandle.artistPtr)
		return;	// data structure has not been previously initialized
	for (i=0;i<numSongs;i++) {
		free(dataHandle.artistPtr[i]);
		free(dataHandle.albumPtr[i]);
		free(dataHandle.trackNumPtr[i]);
		free(dataHandle.genrePtr[i]);
		free(dataHandle.commentPtr[i]);
		free(dataHandle.yearPtr[i]);
		free(dataHandle.discPtr[i]);
		free(dataHandle.composerPtr[i]);
		free(dataHandle.copyrightPtr[i]);
		free(dataHandle.urlPtr[i]);
		free(dataHandle.encodedPtr[i]);
		free(dataHandle.countryPtr[i]);
		free(dataHandle.relTypePtr[i]);
		free(dataHandle.origArtistPtr[i]);
		free(dataHandle.albumArtistPtr[i]);
		free(dataHandle.artistFilterPtr[i]);
		free(dataHandle.publisherPtr[i]);
		free(dataHandle.editionPtr[i]);
		free(dataHandle.albumGainPtr[i]);
		free(dataHandle.albSortOrderPtr[i]);
		free(dataHandle.perfSortOrderPtr[i]);
		}
	free(dataHandle.artistPtr);
	free(dataHandle.albumPtr);
	free(dataHandle.trackNumPtr);
	free(dataHandle.genrePtr);
	free(dataHandle.commentPtr);
	free(dataHandle.yearPtr);
	free(dataHandle.discPtr);
	free(dataHandle.composerPtr);
	free(dataHandle.copyrightPtr);
	free(dataHandle.urlPtr);
	free(dataHandle.encodedPtr);
	free(dataHandle.countryPtr);
	free(dataHandle.relTypePtr);
	free(dataHandle.origArtistPtr);
	free(dataHandle.albumArtistPtr);
	free(dataHandle.artistFilterPtr);
	free(dataHandle.publisherPtr);
	free(dataHandle.editionPtr);
	free(dataHandle.albumGainPtr);
	free(dataHandle.albSortOrderPtr);
	free(dataHandle.perfSortOrderPtr);
	dataHandle.artistPtr = dataHandle.albumPtr = dataHandle.genrePtr = dataHandle.commentPtr = dataHandle.yearPtr =
		dataHandle.discPtr = dataHandle.composerPtr = dataHandle.copyrightPtr = dataHandle.publisherPtr = 
		dataHandle.urlPtr = dataHandle.encodedPtr = dataHandle.countryPtr = dataHandle.relTypePtr = 
		dataHandle.origArtistPtr = dataHandle.albumArtistPtr = dataHandle.trackNumPtr = dataHandle.albumGainPtr = 
		dataHandle.albSortOrderPtr = dataHandle.artistFilterPtr = dataHandle.perfSortOrderPtr = dataHandle.editionPtr = NULL;
}

int compareName(const void *element1, const void *element2)
{
	mainFileStruct *val1, *val2;
	
	val1 = element1;
	val2 = element2;
	
	return CompareStrings (val1->newName, 0, val2->newName, 0, 0);
}

void SortFileNames()																			  
{
	qsort (fileStruct, numFiles, sizeof(mainFileStruct), compareName);
}

int CVICALLBACK TreeSortCI(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	char *str1=NULL, *str2=NULL;
	int len, val;
	
	GetTreeCellAttribute(panel, control, item1, kTreeColFilename, ATTR_LABEL_TEXT_LENGTH, &len);
	str1 = calloc(len+1, sizeof(char));
	GetTreeCellAttribute(panel, control, item1, kTreeColFilename, ATTR_LABEL_TEXT, str1);

	GetTreeCellAttribute(panel, control, item2, kTreeColFilename, ATTR_LABEL_TEXT_LENGTH, &len);
	str2 = calloc(len+1, sizeof(char));
	GetTreeCellAttribute(panel, control, item2, kTreeColFilename, ATTR_LABEL_TEXT, str2);

	val = CompareStrings (str1, 0, str2, 0, 0);
	
	free(str1);
	free(str2);
	
	return val;
}



char* SkipToTrackNameStart(char *name) {
	char *val = name;
	int found=0;

start:
	// must point to an alphanumeric
	while (!isalnum (val[0]) && val[0] != '(' && val[0] != '.' && !isSpecialChar(val[0])) 
		val++;
	if (val[0] == '.' && val[1] == ' ' && !found) {
		val+=2;
		found=1;
		goto start;
		}
	
	return val;
}

int isSpecialChar(char ch)
{
	if (ch > '¿' && ch <= 'ÿ')
		return 1;
	else
		return 0;
}

/*******************************
Fixing the names is a multi Step Process:
	1. Remove Underscores
	2. Remove String from name
	3. Remove specified string
	4. Remove Year if desired
	5. Find and fix Track #
	6. Add Artist and Album Title
	7. Remove Chars from end of name
********************************/
void RenameFiles()
{
	int i, len=0, endlen=0, numLen, cut, removeAlbum;
	int	capitalize, smartcaps, ext, remYear;
	char *end = NULL, *songTitle = NULL, *cap = NULL, *formatString = NULL;
	char temp[255], album[255], artist[255], string[255], trackNum[5], treeTrack[5];
	
	if (!fileList)
		return;
	RemoveUnderscores();

	GetCtrlVal (panelHandle, PANEL_ALBUM, album);
	GetCtrlVal (panelHandle, PANEL_ARTIST, artist);
	GetCtrlVal (panelHandle, PANEL_CAPITALIZE, &capitalize);
	GetCtrlVal (panelHandle, PANEL_SMARTCAPS, &smartcaps);
	RemoveSurroundingWhiteSpace (artist);
	RemoveSurroundingWhiteSpace (album);
	GetCtrlVal (panelHandle, PANEL_UPDATEARTIST, &i);
	if (i)
		SetCtrlVal (panelHandle, PANEL_ARTIST, artist);
	GetCtrlVal (panelHandle, PANEL_UPDATEALBUM, &i);
	if (i)
		SetCtrlVal (panelHandle, PANEL_ALBUM, album);
	/* Replacing unusable chars needs to be done after we do smart caps because we CAN use these chars in ID3 tags */
	ReplaceUnusableChars(album);
	ReplaceUnusableChars(artist);

	for (i=0; i<numFiles;i++)
		if (IsItemChecked(i)) {
			GetTableCellValLength(panelHandle, PANEL_FORMATSTRING, MakePoint(1,1), &len);
			if (len) {
				formatString = malloc(sizeof(char) * len + 1);
				GetTableCellVal(panelHandle, PANEL_FORMATSTRING, MakePoint(1,1), formatString);
				ParseRenameString(fileStruct[i].newName, formatString, i);
				ReplaceUnusableChars(fileStruct[i].newName);
				free(formatString);
				GetCtrlVal(panelHandle, PANEL_REMOVESTRING, string);
				RemoveSpecifiedString(fileStruct[i].newName, string);
				ReplaceListItem (panelHandle, PANEL_TREE, i, fileStruct[i].newName, i);
				}
			else {
				GetCtrlVal(panelHandle, PANEL_REMOVESTRING, string);
				RemoveSpecifiedString(fileStruct[i].newName, string);
				GetCtrlVal(panelHandle, PANEL_REMOVEALBUM, &removeAlbum);
				if (removeAlbum)
					RemoveSpecifiedString(fileStruct[i].newName, album);
				GetCtrlVal(panelHandle, PANEL_REMOVEARTIST, &removeAlbum);
				if (removeAlbum)
					RemoveSpecifiedString(fileStruct[i].newName, artist);
			
				GetCtrlVal(panelHandle, PANEL_REMOVENUMSTART, &cut);
				if (cut) {
					len = (int)strlen(fileStruct[i].newName);
					end = strrchr(fileStruct[i].newName, '.');
					endlen = (int)strlen(end);
					strcpy(fileStruct[i].newName, fileStruct[i].newName + cut);
					}
				GetCtrlVal(panelHandle, PANEL_REMOVENUM, &cut);
				if (cut) {
					len = (int)strlen(fileStruct[i].newName);
					end = strrchr(fileStruct[i].newName, '.');
					endlen = (int)strlen(end);
					if (len > endlen + cut)
						strcpy(fileStruct[i].newName + len - endlen - cut, end);
					}
			
				GetCtrlVal(panelHandle, PANEL_STRIPYEAR, &remYear);
				if (remYear)
					RemoveYear(fileStruct[i].newName);
				
				songTitle = FindTrackNum(fileStruct[i].newName, i, &numLen, trackNum);
				if (!songTitle)
					songTitle = fileStruct[i].newName;
 		
				strcpy(temp, artist);
				if (strlen(temp) == 0)
					strcat(temp, "[");
				else 
					strcat(temp, " [");
				strcat(temp, album);
#if _CVI_ >= 910
				GetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackNum, ATTR_LABEL_TEXT, treeTrack);
				if (strlen(treeTrack))
					strncat(temp, treeTrack, strlen(treeTrack));
				else
#endif				
					strncat(temp, trackNum, numLen);
		
				songTitle = SkipToTrackNameStart(songTitle);
				if (capitalize) {	/* capitalize every first letter */
					cap = songTitle;
					while (cap[0] != songTitle[strlen(songTitle)]) {
						while (isspace(cap[0]) || cap[0] == '-' || cap[0] == '(')
							cap++;
						if (islower(cap[0]))
							cap[0] -= 32;
						cap++;
						while (cap[0] != ' ' && cap[0] != '-' && cap[0]!=songTitle[strlen(songTitle)])
							cap++;
						}
					}
				if (capitalize && smartcaps)
					DoSmartCaps(songTitle);
				//CheckSpelling(songTitle);
				strcat(temp, songTitle);
				
				strcpy(fileStruct[i].newName, temp);
											
				GetCtrlVal (configHandle, OPTIONS_EXTENSIONS, &ext);
				if (ext) {
					end = strrchr(fileStruct[i].newName, '.');
					if (end != NULL)
						while (end[0] != '\0') {
							if (isupper(end[0]))
								end[0] += 32;   
							end++;
							}
					}
				}
			}
	
	return;
}

/********************************/
void ParseRenameString(char *filename, char *formatStr, int index)
{
	char *newstr = NULL, *ptr, *dataVal = NULL, *ext = NULL, *ffPtr, *closeParen, *ffStr = NULL;
	int i, len, extensions;

	GetCtrlVal(configHandle, OPTIONS_EXTENSIONS, &extensions);
				
	ptr = strrchr(filename, '.');	// find last . to save extension
	if (ptr) {
		if (strlen(ptr+1))	// not last character
			ext = malloc(sizeof(char) * strlen(ptr) + 1);
			strcpy(ext, ptr);
		}
	newstr = malloc(sizeof(char) * (strlen(formatStr) + 1));
	strcpy(newstr, formatStr);
	for (i=0;i<kNumMetaDataVals;i++) {
		if (ptr = strstr(newstr, metaData[i].name)) {
			len = GetMetaDataValue(&dataVal, metaData[i], index);
			ReplaceFormatStringToken(&newstr, metaData[i].name, dataVal);
			free(dataVal);
			}
		}
	ReplaceFormatStringToken(&newstr, ",,", kDoubleCommaReplacement);
	ReplaceFormatStringToken(&newstr, ",)", kCommaCloseReplacement);
	
	while (ptr = strrchr(newstr, '(')) {
		for (i=0;i<kNumFormatFuncs;i++) {
			if (ptr-newstr >= strlen(formatFunc[i].name))	// don't go past start of string
				ffPtr = ptr-strlen(formatFunc[i].name);
			else
				continue;
			if (ffPtr >= newstr)
				if (!strncmp(ffPtr, formatFunc[i].name, strlen(formatFunc[i].name)) && (closeParen=strstr(ffPtr, ")"))) {
					ffStr = malloc(sizeof(char) * (++closeParen - ffPtr + 1));
					strncpy(ffStr, ffPtr, closeParen-ffPtr);
					ffStr[closeParen-ffPtr] = '\0';
					len = GetFunctionVal(ptr, formatFunc[i], index, &dataVal);
					ReplaceFormatStringToken(&newstr, ffStr, dataVal);
					free(dataVal);
					free(ffStr);
					break;
					}
			}
		if (i==kNumFormatFuncs && !strstr(ptr, ")"))	// no match or closing brace
			ReplaceFormatStringToken(&newstr, "(", kOpenParenReplacement);
		}

/*	ReplaceFormatStringToken(&newstr, kOpenParenReplacement, "(");
	ReplaceFormatStringToken(&newstr, kCloseParenReplacement, ")");
	ReplaceFormatStringToken(&newstr, kCommaReplacement, ",");
	ReplaceFormatStringToken(&newstr, kSingleQuoteReplacement, "'");
	ReplaceFormatStringToken(&newstr, kDollarSignReplacement, "$");
	ReplaceFormatStringToken(&newstr, kBlankSpaceReplacement, "");
*/
	RestoreSpecialChars(&newstr);
	
	strcpy(filename, newstr);
	if (extensions)
		for (i=0;i<strlen(ext);i++)
			ext[i] = tolower(ext[i]);
	strcat(filename, ext);		// restore saved file extension
	free(newstr);
	if (ext)
		free(ext);
	return;
}

/********************************/
void RemoveUnderscores()
{
	int i, val = 0;
	char *loc = NULL;

	GetCtrlVal (panelHandle, PANEL_UNDERSCORECHECKBOX, &val);
	if (!fileList || !val)
		return;
	for (i=0;i<numFiles;i++)
		if (IsItemChecked(i)) {
			loc = memchr (fileStruct[i].newName, '_', strlen(fileStruct[i].newName));
			while (loc) {
				memset (loc, 32, 1);
				loc = memchr (fileStruct[i].newName, '_', strlen(fileStruct[i].newName));
				}
			ReplaceListItem (panelHandle, PANEL_TREE, i, fileStruct[i].newName, i);
		}
}

/********************************/
void RemoveSpecifiedString(char *filename, char *string)
{
	char *loc = NULL, *string2;
	
	if (strlen(string) > 0) {
		loc = stristr (filename, string);
		if (loc) {
			string2 = loc + strlen(string);
			strcpy(loc, string2);
			}
		}
}

	

/* case insensitive strstr */
char *stristr(const char *String, const char *Pattern)
{
      char *pptr, *sptr, *start;

      for (start = (char *)String; *start != NULL; start++)
      {
            /* find start of pattern in string */
            for ( ; ((*start!=NULL) && (toupper(*start) != toupper(*Pattern))); start++)
                  ;
            if (NULL == *start)
                  return NULL;
            pptr = (char *)Pattern;
            sptr = (char *)start;
            while (toupper(*sptr) == toupper(*pptr))
            {
                  sptr++;
                  pptr++;

				  /* if end of pattern then pattern was found */
                  if (NULL == *pptr)
                        return (start);
            }
      }
      return NULL;
}

/********************************/
void RemoveYear(char *filename)
{
	double currDate;
	int i, year, endYear;
	char yearStr[5], *found;
	
	GetCurrentDateTime(&currDate);
	GetDateTimeElements(currDate,NULL,NULL,NULL,NULL,NULL,&endYear);
	yearStr[4] = '\0';
	for (i=1900;i<=endYear+1;i++) {
		year = i;
		yearStr[0] = year / 1000 + 48;
		year-=(yearStr[0]-48) * 1000;
		yearStr[1] = year / 100 + 48;
		year-=(yearStr[1]-48) * 100;
		yearStr[2] = year / 10 + 48;
		year-=(yearStr[2]-48) * 10;
		yearStr[3] = year + 48;
		found = strstr(filename, yearStr);
		if (found) {
			strcpy(found,found+4);
			break;
			}
		}
}

/********************************/
int	IsItemChecked(int itemNum)
{
	int val;
	
	GetTreeItemAttribute (panelHandle, PANEL_TREE, itemNum, ATTR_MARK_STATE, &val);
	
	return (val == VAL_MARK_ON);
}

int GetAudioFileType(char *filename)
{
	int isAudio=0;
	char *ext;
	
	ext = strrchr(filename, '.');
	if (ext) {
		ext++;
		if (strlen(ext)) {	/* not last char in filename */
			if (!stricmp(ext, "mp3"))
				isAudio=kFileMP3;
			else if (!stricmp(ext, "flac"))
				isAudio=kFileFLAC;
			else if (!stricmp(ext, "ac3"))
				isAudio=kFileAC3;
			else if (!stricmp(ext, "dts"))
				isAudio=kFileDTS;
			else if (!stricmp(ext, "ape"))
				isAudio=kFileAPE;
			else if (!stricmp(ext, "aac"))
				isAudio=kFileAAC;
			else if (!stricmp(ext, "wav"))
				isAudio=kFileWAV;
			else if (!stricmp(ext, "wma"))
				isAudio=kFileWMA;
			else if (!stricmp(ext, "wv"))
				isAudio=kFileWV;
		}
	}
	
	return isAudio;
}

/************************************************************/

diacriticVal diacriticVals[kNumDiacritics] = {{'á','a'},{'ä','a'},{'â','a'},{'å','a'},{'é','e'},
											  {'è','e'},{'ê','e'},{'ñ','n'},{'ö','o'},{'ó','o'},
											  {'ò','o'},{'ô','o'},{'í','i'},{'ì','i'},{'ï','i'},
											  {'ú','u'},{'ü','u'},{'û','u'}};



void ReplaceDiacritics(char *str)
{
	int i;
	char *ptr = NULL;
	
	for (i=0;i<kNumDiacritics;i++) {
		while (ptr = strchr(str, diacriticVals[i].diacritic))
			ptr[0] = diacriticVals[i].replacement;
		}
}

void ReplaceCommonWords(char *str)
{
	int i;
	char *ptr = NULL;
	
	for (i=1;i<=kNumCommonWords;i++) {
		ptr = str;
		while (ptr = strstr(ptr, wordList[i]))
			if ((ptr == str || ptr[-1]==' ') && ((ptr+strlen(wordList[i]) == (str+strlen(str))) ||
												 (ptr+strlen(wordList[i]) <  (str+strlen(str))) && ptr[strlen(wordList[i])]==' '))
				memset(ptr, ' ', strlen(wordList[i]));
			else
				ptr += strlen(wordList[i]);
	}
	
}

void ReplaceAmpersands(char *str)
{
	char *ptr = NULL;
	char temp[512];
	
	while (ptr = strstr(str, "&")) {
		strcpy(temp, ptr+1);
		strcpy(ptr, "%26\0");
		strcat(ptr, temp);
	}
}

void ReplaceUnicodeApostrophe(char *str)
{
	char *ptr = NULL;
	char tempStr[strlen(str)*3 + 1];	// make enough space for ... to replace the unicode ellipses if all the characters are unicode ellipses
	int  i, j;
	
	while (ptr = strstr(str, "’")) {
		ptr[0] = '\'';
	}
	for (i=0, j=0; i<strlen(str); i++) {
		if (str[i] == '\205') {		// ... unicode char
			tempStr[j++]='.';
			tempStr[j++]='.';
			tempStr[j++]='.';
		} else {
			tempStr[j++]=str[i];
		}
	}
	tempStr[j] = '\0';
	strcpy(str, tempStr);
}

/*** This function streamlines the MusicBrainz search by removing those "[Deluxe Edition]" strings from the album title ***/
#define kNumBrackets	3
void RemoveAlbumEditions(char *str)
{
	char *tmpStr = NULL, *ptr;
	int i, j, editionStrLen;
	char openBracket[kNumBrackets]  = {'[','(','{'};
	char closeBracket[kNumBrackets] = {']',')','}'};
	
	tmpStr = calloc(strlen(str) + 1, sizeof(char));
	for (i=0; i<kNumEditions; i++) {
		if (ptr = stristr(str, albumEditions[i])) {
			editionStrLen = strlen(albumEditions[i]);
			if (ptr != tmpStr && (ptr-str) + editionStrLen < strlen(str))	{ // check to see if braces could be around string
				for (j=0;j<kNumBrackets;j++)
					if (ptr[-1] == openBracket[j] && ptr[editionStrLen] == closeBracket[j]) {
						editionStrLen += 2;
						ptr--;
						break;
					}
			}
			strncpy(tmpStr, str, ptr-str);
			if (ptr-str+editionStrLen < strlen(str))
				strcpy(tmpStr+(ptr-str), ptr+editionStrLen);
			strcpy(str, tmpStr);
		}
	}
	free(tmpStr);
}

/************************************************************/
/*** Finds Track Num and returns first char AFTER the num ***/
/************************************************************/
char* FindTrackNum(char *filename, int index, int *numLen, char *trackNum)
{
	int i, found=0, fixNums, idx;
	size_t len;
	char *string = NULL, *origFilename = NULL, trackStr[5], temp[3] = "0 \0";
	char indexStr[10];
	
	if (GetAudioFileType(filename)) {
		GetTreeCellAttribute (panelHandle, PANEL_TREE, index, kTreeColID, ATTR_LABEL_TEXT, indexStr);
		idx = atoi(indexStr);
		if (dataHandle.trackNumPtr && dataHandle.trackNumPtr[idx] && strlen(dataHandle.trackNumPtr[idx])>0 && 
				isalnum(dataHandle.trackNumPtr[idx][0]) && !isalpha(dataHandle.trackNumPtr[idx][0])) {
			string = strchr(dataHandle.trackNumPtr[idx], '/');
			if (string) { /* track format is 01/12 */
				len = strlen(dataHandle.trackNumPtr[idx]) - strlen(string);
				}
			else {
				len = strlen(dataHandle.trackNumPtr[idx]);
				}
			strncpy(trackStr, dataHandle.trackNumPtr[idx], len);
			trackStr[len] = '\0';
			GetCtrlVal(panelHandle, PANEL_FIXNUMCHECKBOX, &fixNums);
			if (trackNum) {
				if (len == 1 && fixNums)
					snprintf(trackNum, len+2, "0%s", trackStr);
				else
					snprintf(trackNum, len+1, "%s", trackStr);
				}
		
			string = NULL;
			if (len == 1) {				// search two numbers wide first
				temp[1] = trackStr[0];	// format 01 from 1 
				string = strstr(filename, temp);
				if (string) {
					if (numLen) *numLen=2;	// length of the string in the name is 2
					return string + 2;
					}
				}
			if(dataHandle.albumPtr && dataHandle.albumPtr[idx] && 
				(string = strstr(filename, trackStr)) &&
				(origFilename = strstr(filename, dataHandle.albumPtr[idx])) &&
				string > origFilename)	// find start of album name, skip album if album starts before track
										// number is found (so album called 2001 does not mess up track 01)
				origFilename += strlen(dataHandle.albumPtr[idx]);			// skip album name
			else
				origFilename = filename;
			string = strstr(origFilename, trackStr);
			if (string) {
				if (numLen) *numLen=(int)strlen(trackStr);
				return string + strlen(trackStr);
			}
	
		} else {	// ID3 tag didn't have trackNum, so attempt to find one
			origFilename = filename + strlen(filename)-1;

			for (i=48;i<=57;i++) {	// search for numbers 0-9
				string = strchr (filename, i);
				if (string && strcmp(string, origFilename)) {
					found = 1;
					if (string[1] >= 48 && string[1] <= 57) {	// check next character i.e. we found 0, is the next character a 1?
						if (numLen)
							*numLen = 2;
						if (trackNum)
							snprintf(trackNum, 3, "%s", string);
						return string + 2;
					} else {
						string = NULL;
					}
				}
			}
	
			if (found) {
				if (numLen)
					*numLen = 1;
				/* assume that we found numbers but not a ## style track number */
				for (i=48;i<=57;i++) {
					string = strchr (filename, i);
					if (string && strcmp(string, origFilename)) {
						if (!isalpha (string[1])) {
							if (trackNum) {
								snprintf(trackNum, 3, "0%s", string);
							}
							return string + 1;
						}
					}
				}
			}
		}
	}

	if (numLen)
		*numLen = 0;
	return NULL;
}

/*  I make some assumptions here. i.e. Song Title is ALWAYS going to be the last part of the song name 
	and will always follow trackNum */
char* FindSongTitle(char *filename, int index, int *numLen)
{
	int length;
	char *ptr = NULL;
	char trackNum[5];

	ptr = FindTrackNum(filename, index, &length, trackNum);
	if (ptr) {
		ptr = SkipToTrackNameStart(ptr);
		if (numLen)
			*numLen = (int)strlen(ptr)-4;	/* subtract ".mp3" from name */
		}
	else if (numLen)
		*numLen = 0;
	
	return ptr;
}

int CVICALLBACK RemoveUnderscoresCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			RemoveUnderscores();
			break;
		}
	return 0;
}

int IsWordInList(char *word, int len)
{
	int i, found=0;
	char *lowerWord = NULL;
	
	lowerWord = calloc(len+1, sizeof(char));
	strncpy(lowerWord, word, len);
	lowerWord[len] = '\0';
	StringLowerCase(lowerWord);
	
	for (i=0;i<kNumWords;i++) {
		if (strlen(wordList[i]) == len)
			if (!memcmp(lowerWord, wordList[i], len)) {
				found = 1;	// word exact match
				break;
				}
		}
	free(lowerWord);
	return found;	// not found
}

void DoSmartCaps(char *string)
{
	/* Smart Capitalization Rules:
		Capitalize first word and last word of a title - The is the The 
		Do not capitalize little words within titles such as:
			a, an, the, but, as, if, and, or, nor, or prepositions, regardless of their length.
		Prepositions: "about," "above," "across," "after," "against," "along," "among," "around," 
			"at," "before," "behind," "below," "beneath," "beside," "between," "beyond," "but," 
			"by," "despite," "down," "during," "except," "for," "from," "in," "inside," "into," 
			"like," "near," "of," "off," "on," "onto," "out," "outside," "over," "past," "since," 
			"through," "throughout," "till," "to," "toward," "under," "underneath," "until," "up," 
			"upon," "with," "within," and "without."*/
	int i=0;
	int spaces[200] = {0}; // maximum of 200 words
	int numSpaces=0;
	char *pos;
	
	
	pos = string;
	for (i=0;i<strlen(string);i++)
		if (isspace(string[i]) || string[i]=='.') {
			while (!isalpha(string[i+1]))	// manually skip non alpha numerics
				i++;	
			if (i >= strlen(string))	// just to make sure
				break;
			string[i+1] = toupper(string[i+1]);
			spaces[numSpaces] = i;	// save location of spaces
			numSpaces++;
			}
	if (numSpaces && string[spaces[numSpaces-1]] == '.') // last period is in front of .mp3 so don't count it!
		numSpaces--;
		
	// Band Name Here [Album Title Begins 01] - Song Title Here.mp3
	//	   5   10   15	   21	 27		34  3840   45	 51
	string[0] = toupper(string[0]);		// always cap first word
	//pos = string;
	for (i=0; i < numSpaces-1; i++) {
		pos = string + spaces[i]+1;
		
		if (!IsWordInList(pos, spaces[i+1]-spaces[i]-1))
			string[spaces[i]+1] = toupper(string[spaces[i]+1]);
		else
			string[spaces[i]+1] = tolower(string[spaces[i]+1]);
		}
	if (numSpaces > 0)
		string[spaces[i]+1] = toupper(string[spaces[i]+1]);	// should be last word in string
}

int CVICALLBACK RevertNames (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i;
	
	switch (event)
		{
		case EVENT_COMMIT:
			for (i=0;i<numFiles;i++) {
				if (IsItemChecked(i))
					strcpy(fileStruct[i].newName, fileStruct[i].origName + strlen(pathName) + 1);
					ReplaceListItem (panelHandle, PANEL_TREE, i, fileStruct[i].newName, i);
				}
			SortFileNames();
			SortTreeItems (panel, PANEL_TREE, 0, 0, 0, 0, TreeSortCI, 0);
			break;
		}						   
	return 0;
}


int CVICALLBACK CancelCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			RemovePopup (0);
			DiscardPanel(panel);
			break;
		}
	return 0;
}

/**********************************************/

int CVICALLBACK GetID3Tag (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i;
	char buf[100];
	char indexStr[3];
	
	switch (event) {
		case EVENT_COMMIT:
			ClearLEDs();
			initID3DataStruct(numFiles);
			ClearID3Fields();
			SetCtrlAttribute(tab2Handle, TAB2_IMAGECORRUPTEDMSG, ATTR_VISIBLE, FALSE);
			SetCtrlAttribute(tab2Handle, TAB2_CLEARARTWORK, ATTR_VISIBLE, FALSE);
			SetCtrlAttribute(tab2Handle, TAB2_ARTWORK, ATTR_DIMMED, FALSE);
			SetCtrlVal(tab2Handle, TAB2_CLEARARTWORK, FALSE);
			sprintf(buf, kPictureSizeStr, 0, 0);
			SetCtrlVal(tab2Handle, TAB2_IMAGESIZEMSG, buf);
			SetCtrlAttribute(panel, PANEL_RENAMEFOLDER, ATTR_DIMMED, FALSE);
			DeleteListItem (tab3Handle, TAB3_EXTENDEDTAGS, 0, -1);	// clear extended tags tree
			firstFile = TRUE;
			for (i=0;i<numFiles;i++) {
				if (IsItemChecked(i)) {
					switch (GetAudioFileType(fileStruct[i].origName)) {
						case kFileMP3:
							GetID3v2Tag(panel, fileStruct[i].origName, i);
							GetID3v1Tag(panel, fileStruct[i].origName);
							firstFile = FALSE;	// clear firstFile flag after the first checked file
							break;
						case kFileAC3:
						case kFileDTS:
							LoadAPEv2Tag(panel, fileStruct[i].origName, i);
							firstFile = FALSE;	// clear firstFile flag after the first checked file
							break;
					}
				}
				sprintf(indexStr, "%d\0", i);
				SetTreeCellAttribute (panelHandle, PANEL_TREE, i, kTreeColID, ATTR_LABEL_TEXT, indexStr);
			}
			PopulateTrackData();
			SetMetaDataButtonDimming(panel, 0);
			SetConflictTooltips(panel);
			ClearRedundantFieldsIfNeeded(panel);
			break;
	}
	return 0;
}

int GetID3v1Tag (int panel, char *filename)
{
	int		fHandle, bytesRead, len, error = 0, genreID;
	char	buf[128];
	char	artist[31], album[31], year[5], comment[30];
	

	fHandle = OpenFile(filename, VAL_READ_ONLY, VAL_OPEN_AS_IS, VAL_BINARY);
	DisableBreakOnLibraryErrors();
	errChk((int)SetFilePtr (fHandle, -128, 2));
	EnableBreakOnLibraryErrors();
	bytesRead = ReadFile (fHandle, buf, 128);
	if (!memcmp(buf,"TAG",3)) {
		GetCtrlAttribute(panel, PANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &len); 
		if (!len) {
			strncpy(artist, buf+33, 30);
			artist[30] = '\0';
			RemoveSurroundingWhiteSpace (artist);
			SetCtrlVal(panel, PANEL_ARTIST, artist);
			}
		GetCtrlAttribute(panel, PANEL_ALBUM, ATTR_STRING_TEXT_LENGTH, &len); 
		if (!len) {
			strncpy(album, buf+63, 30);
			album[30] = '\0';
			RemoveSurroundingWhiteSpace (album);
			SetCtrlVal(panel, PANEL_ALBUM, album);
		}
		GetCtrlAttribute(tab1Handle, TAB1_COMMENT, ATTR_STRING_TEXT_LENGTH, &len); 
		if (!len) {
			strncpy(comment, buf+97, 29);
			comment[29] = '\0';
			RemoveSurroundingWhiteSpace (comment);
			SetCtrlVal(tab1Handle, TAB1_COMMENT, comment);
		}
		GetCtrlAttribute(tab1Handle, TAB1_YEAR, ATTR_STRING_TEXT_LENGTH, &len); 
		if (!len) {
			strncpy(year, buf+93, 4);
			year[4] = '\0';
			SetCtrlVal(tab1Handle, TAB1_YEAR, year);
		}
		GetTableCellValLength(tab1Handle, TAB1_GENRE, tagCell, &len); 
		if (!len) {
			if (buf[127] < 0 || buf[127] > (kNumWinampGenres - 1)) {
				genreID = 0;
				SetTableCellVal(tab1Handle, TAB1_GENRE, tagCell, "");
				}
			else {
				genreID = buf[127];
				SetTableCellVal(tab1Handle, TAB1_GENRE, tagCell, genreList[genreID]);
				}
			}
		}

Error:
	CloseFile(fHandle);
	return 0;
}

/**********************************************/

int CVICALLBACK SetID3Tag (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i, readOnly, archive, hidden, system, error=0, populateAlbumOrder;
	int len, yearLen, doID3v1, val;
	char *yearStr = NULL, artist[512];
	
	switch (event)
		{
		case EVENT_COMMIT:
			ProgressBar_ClearMilestones(progressHandle, PROGRESS_PROGRESSBAR);
			for (i=1;i<=numFiles;i++)
				ProgressBar_AddMilestone(progressHandle, PROGRESS_PROGRESSBAR, 100.0*i/(double)numFiles);
			ProgressBar_SetAttribute(progressHandle, PROGRESS_PROGRESSBAR, ATTR_PROGRESSBAR_UPDATE_MODE, VAL_PROGRESSBAR_AUTO_MODE);
			CenterPopupPanel(panel, progressHandle);
			ProgressBar_Start(progressHandle, PROGRESS_PROGRESSBAR, "Writing ID3 Tags...");
			InstallPopup(progressHandle);
			GetCtrlVal(configHandle, OPTIONS_POPULATEALBUMORDER, &populateAlbumOrder);
			GetCtrlVal(panel, PANEL_DOID3V1, &doID3v1);
			GetCtrlVal(tab1Handle, TAB1_UPDATEPERFSORT, &val);
			GetCtrlVal(panel, PANEL_ARTIST, artist);
			if (val && !strncmp("The ", artist, 4))		// detect if artist starts with "The " so we can move it to the end for artistsortorder
				GuessSortOrderCB(tab1Handle, TAB1_GUESSBUTTON, EVENT_COMMIT, NULL, 0, 0);
			GetCtrlAttribute(tab1Handle, TAB1_ALBUMSORTORDER, ATTR_STRING_TEXT_LENGTH, &len);
			GetCtrlAttribute(tab1Handle, TAB1_YEAR, ATTR_STRING_TEXT_LENGTH, &yearLen);
			if (populateAlbumOrder && len<5 && yearLen) {
				yearStr = GetCtrlStrVal(tab1Handle, TAB1_YEAR);
				yearStr[4] = '\0';	// only use the first four #s of the year string incase it's something dumb like "1976-08-21"
				SetCtrlVal(tab1Handle, TAB1_ALBUMSORTORDER, yearStr);
				}
			for (i=0;i<numFiles;i++) {
				GetFileAttrs(fileStruct[i].origName, &readOnly, &system, &hidden, &archive);
				if (IsItemChecked(i) && !readOnly) {
					switch (GetAudioFileType(fileStruct[i].origName)) {
						case kFileMP3:
							SetID3v2Tag(panel, fileStruct[i].origName, fileStruct[i].newName, i);
							if (doID3v1) 
								SetID3v1Tag(panel, fileStruct[i].origName, fileStruct[i].newName, i);
							break;
						case kFileAC3:
						case kFileDTS:
							SetAPEv2Tag(panel, fileStruct[i].origName, fileStruct[i].newName, i);
							break;
					}
				} else if (readOnly) {
					error = readOnly;
				}
				ProgressBar_AdvanceMilestone(progressHandle, PROGRESS_PROGRESSBAR, 0);
				}
			ProgressBar_End(progressHandle, PROGRESS_PROGRESSBAR, NULL, 0);
			Delay(0.15);
			HidePanel(progressHandle);
			if (error)
				MessagePopup ("Error Occured", "Some/all of the files were read-only\nand their tags could not be written.");
			break;
		}
	
	if (yearStr)
		free(yearStr);
	return 0;
}

int SetID3v1Tag (int panel, char *filename, char *newname, int index)
{
	int		fHandle=0, error, len, commentV1, val;
	char	*buf=NULL, *ptr, *data=NULL, *songTitle=NULL, tag[3];
	char	title[30], track, genreString[kMaxGenreEntryLength];

	
	nullChk(buf = calloc(128, sizeof(char)));
	
	strncpy(buf,"TAG",3);

	GetCtrlVal(panel, PANEL_UPDATETITLE, &val);
	if (val) {
		GetTreeCellAttribute (panelHandle, PANEL_TREE, index, kTreeColTrackName, ATTR_LABEL_TEXT_LENGTH, &len);
		if (len) {
			songTitle = malloc(sizeof(char) * (len + 1));
			GetTreeCellAttribute (panelHandle, PANEL_TREE, index, kTreeColTrackName, ATTR_LABEL_TEXT, songTitle);
			ptr = songTitle;
			}
		else
			ptr = FindSongTitle(newname, index, &len);
		if (ptr) {	// didn't find song title so don't write a title
			strncpy(title, ptr, 30);
			if (len < 30)
				title[len]='\0';
			strncpy(buf+3,title,30);
			}
		}

	GetCtrlVal(panel, PANEL_UPDATEARTIST, &val);
	if (val) {
		data = GetCtrlStrVal(panel, PANEL_ARTIST);
		if (data) {
			strncpy(buf+33,data,30);
			free(data);
			}
		}
	
	GetCtrlVal(panel, PANEL_UPDATEALBUM, &val);
	if (val) {
		data = GetCtrlStrVal(panel, PANEL_ALBUM);
		if (data) {
			strncpy(buf+63,data,30);
			free(data);
			}
		}
	
	GetCtrlVal(tab1Handle, TAB1_UPDATEYEAR, &val);
	if (val) {
		data = GetCtrlStrVal(tab1Handle, TAB1_YEAR);
		if (data) {
			strncpy(buf+93,data,4);
			free(data);
			}
		}

	GetCtrlVal(tab1Handle, TAB1_UPDATECOMMENT, &val);
	if (val) {
		GetCtrlVal(panel, PANEL_ID3V1COMMENTS, &commentV1);
		if (commentV1) {
			data = GetCtrlStrVal(tab1Handle, TAB1_COMMENT);
			if (data) {
				strncpy(buf+97,data,28);
				free(data);
				}
			}
		}
	
	track = GetNumericTrackNum(newname, index);
	buf[126]=track;
	GetTableCellVal(tab1Handle, TAB1_GENRE, tagCell, genreString);
	buf[127] = GetGenreIndex(genreString);	// 255 if no valid value
	
	errChk(fHandle = OpenFile(filename, VAL_READ_WRITE, VAL_OPEN_AS_IS, VAL_BINARY));
	errChk((int)SetFilePtr (fHandle, -128, 2));
	errChk(ReadFile (fHandle, tag, 3));
	if (!memcmp(tag,"TAG",3)) {
		errChk((int)SetFilePtr (fHandle, -128, 2));
		}
	else
		errChk((int)SetFilePtr (fHandle, 0, 2));	// tack on to the end
	errChk(WriteFile(fHandle, buf, 128));
	fHandle = CloseFile(fHandle);

Error:
	if (songTitle)
		free(songTitle);
	if (buf)
		free(buf);
	if (fHandle)
		CloseFile(fHandle);
	return 0;
}

/**********************************************/
void PopulateTrackData(void)
{
	int i, j, k, fixNums;
	char track[10];
	char buf[10];
	
	GetCtrlVal(panelHandle, PANEL_FIXNUMCHECKBOX, &fixNums);
	strcpy(track,"");

	for (i=0;i<numFiles;i++) {
		SetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackNum, ATTR_LABEL_TEXT, "");
	}
	for (i=0;i<numFiles;i++) {
		if (dataHandle.trackNumPtr[i]) {
			strcpy(buf, dataHandle.trackNumPtr[i]);
			RemoveSurroundingWhiteSpace(buf);
			k=0;
			for (j=0;j<strlen(buf);j++)
				if (isdigit(buf[j])) {
					track[k]=buf[j];
					k++;
				} else {
					break;
				}
			track[k] = '\0';
			if (fixNums && strlen(track) == 1) {
				track[2] = track[1];
				track[1] = track[0];
				track[0] = '0';
			}
			SetTreeCellAttribute (panelHandle, PANEL_TREE, i, kTreeColTrackNum, ATTR_LABEL_TEXT, track);
		}
	}
	
}

/**********************************************/

/* this function is used to get the numeric number for the ID3v1 tags */
int GetNumericTrackNum(char *filename, int index)
{
	int len=0, i=0, found=0, track=0;
	char *str = NULL, *string = NULL, *origFilename = NULL;
	char noTagFilename[MAX_FILENAME_LEN];
	
	GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColTrackNum, ATTR_LABEL_TEXT_LENGTH, &len);
	if (len) {
		str = malloc((len+1) * sizeof(char));
		GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColTrackNum, ATTR_LABEL_TEXT, str);
		RemoveSurroundingWhiteSpace(str);
		while (isdigit(str[i])) {
			track *= 10;
			track += str[i]-48;
			i++;
			}
		if (track > 255) track = 0;
		}
	else {
		strncpy(noTagFilename, filename, strlen(filename)-4);
		noTagFilename[strlen(filename)-4] = '\0';
		origFilename = filename + strlen(filename)-1;
	
		for (i=48;i<=57;i++) {
			string = strchr (filename, i);
			if (string && strcmp(string, origFilename)) {
				found = 1;
				track = 10*(i-48);
				if (string[1] >= 48 && string[1] <= 57) {
					return track+=string[1]-48;
					}
				else
					track = 0;
				}
			}
	
		if (found) {
			/* assume that we found numbers but not a ## style track number */
			for (i=48;i<=57;i++) {
				string = strchr (filename, i);
				if (string && strcmp(string, origFilename)) {
					track = i-48;
					if (!isalpha (string[1]))
						return track;
					}
				}
			}
		}

	if (str)
		free(str);
	return track;
}

/**********************************************/

void SetMetaDataButtonDimming(int panel, int undim)
{
	int len;

	if (undim)
		SetCtrlAttribute(panel, PANEL_GetMusicBrainzData, ATTR_DIMMED, 0);
	else {
		GetCtrlAttribute(panel, PANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &len);
		if (len)
			SetCtrlAttribute(panel, PANEL_GetMusicBrainzData, ATTR_DIMMED, 0);
		else {
			GetCtrlAttribute(panel, PANEL_ALBUM, ATTR_STRING_TEXT_LENGTH, &len);
			if (len)
				SetCtrlAttribute(panel, PANEL_GetMusicBrainzData, ATTR_DIMMED, 0);
			else
				SetCtrlAttribute(panel, PANEL_GetMusicBrainzData, ATTR_DIMMED, 1);
			}
		}
}

/* this function will clear fields such as Album Artist (if it equals album) and disc num (if it equals "1/1")
   assuming there are no conflicts and the options are enabled in the Options panel. */
void ClearRedundantFieldsIfNeeded(int panel)
{
	int clearAlbArtist, clearDiscNum;
	int led1, led2;
	char artist[128], albumArtist[128], discNum[32];
	
	GetCtrlVal(configHandle, OPTIONS_SKIPALBUMARTIST, &clearAlbArtist); 
	GetCtrlVal(configHandle, OPTIONS_IGNOREDISC1OF1, &clearDiscNum);
	
	if (clearAlbArtist) {
		GetCtrlVal(tab1Handle, TAB1_ALBUMARTISTLED, &led1);
		GetCtrlVal(panelHandle, PANEL_ARTISTLED, &led2);
		GetCtrlVal(tab1Handle, TAB1_ALBUMARTIST, albumArtist);
		GetCtrlVal(panelHandle, PANEL_ARTIST, artist);
		if (!led1 && !led2 && !StrICmp(artist, albumArtist)) {
			SetCtrlVal(tab1Handle, TAB1_ALBUMARTIST, "");
		}
	}
	if (clearDiscNum) {
		GetCtrlVal(tab1Handle, TAB1_DISCNUMLED, &led1);
		GetCtrlVal(tab1Handle, TAB1_DISCNUM, discNum);
		if (!led1 && !StrICmp(discNum, "1/1"))
			SetCtrlVal(tab1Handle, TAB1_DISCNUM, "");
	}
}


/**********************************************/

int GetGenreIndex(char *genre)
{
	int i;
	
	for (i=0;i<kNumWinampGenres;i++)
		if (!strcmp(genreList[i], genre))
			return i;

	return kNoGenre;
}

/**********************************************/

int CVICALLBACK ArtistCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static checkOnValChanged = 0;
	
	switch (event)
		{
		case EVENT_KEYPRESS:
			SetCtrlVal(panel, PANEL_ARTISTLED, 0);
			SetCtrlAttribute(panel, PANEL_RENAMEFOLDER, ATTR_DIMMED, 0);
			if ((eventData1 != VAL_BACKSPACE_VKEY && eventData1 != VAL_FWD_DELETE_VKEY))
				SetMetaDataButtonDimming(panel, 1);
			else 
				checkOnValChanged = 1;
			break;
		case EVENT_VAL_CHANGED:
			if (checkOnValChanged) {
				SetMetaDataButtonDimming(panel, 0);
				checkOnValChanged = 0;
				}
			break;
		}
	return 0;
}

int CVICALLBACK AlbumCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static checkOnValChanged = 0;
	
	switch (event)
		{
		case EVENT_KEYPRESS:
			SetCtrlVal(panel, PANEL_ALBUMLED, 0);
			SetCtrlAttribute(panel, PANEL_RENAMEFOLDER, ATTR_DIMMED, 0);
			if ((eventData1 != VAL_BACKSPACE_VKEY && eventData1 != VAL_FWD_DELETE_VKEY))
				SetMetaDataButtonDimming(panel, 1);
			else 
				checkOnValChanged = 1;
			break;
		case EVENT_VAL_CHANGED:
			if (checkOnValChanged) {
				SetMetaDataButtonDimming(panel, 0);
				checkOnValChanged = 0;
				}
			break;
		}
	return 0;
}

int CVICALLBACK TabCB (int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int pos;
	switch (event)
		{
		case EVENT_ACTIVE_TAB_CHANGE:
			GetCtrlAttribute(panel, PANEL_TABVALS, ATTR_TOP, &pos);
			if (eventData2==1) /* 2nd tab was clicked */
				SetCtrlAttribute(panelHandle, PANEL_TABVALS, ATTR_TOP, pos-1);
			else if (eventData1==1) // previously selected was tab 2
				SetCtrlAttribute(panelHandle, PANEL_TABVALS, ATTR_TOP, pos+1);
			GetCtrlAttribute(panel, PANEL_TABVALS2, ATTR_TOP, &pos);
			if (eventData2==2) /* 3rd tab was clicked */
				SetCtrlAttribute(panelHandle, PANEL_TABVALS2, ATTR_TOP, pos-1);
			else if (eventData1==2) // previously selected was tab 3
				SetCtrlAttribute(panelHandle, PANEL_TABVALS2, ATTR_TOP, pos+1);
			break;
		}
	return 0;
}

int CVICALLBACK tabLEDCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int old, new=0;
	
	switch (event)
		{
		case EVENT_LEFT_CLICK:
			if (control == PANEL_TABVALS) 
				new = 1;
			else if (control == PANEL_TABVALS2)
				new = 2;
			GetActiveTabPage(panel, PANEL_TAB, &old);
			SetActiveTabPage(panel, PANEL_TAB, new);
			TabCB(panel, PANEL_TAB, EVENT_ACTIVE_TAB_CHANGE, NULL, old, new);
			break;
		}
	return 0;
}


int CVICALLBACK CapitalizeCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &val);
			SetCtrlAttribute(panel, PANEL_SMARTCAPS, ATTR_DIMMED, !val);
			SetCtrlAttribute(panel, PANEL_SMARTCAPID3, ATTR_DIMMED, !val);
			if (val)
				SmartCapCB(panel, PANEL_SMARTCAPS, event, NULL, 0, 0);
			break;
		}
	return 0;
}

int CVICALLBACK SmartCapCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &val);
			SetCtrlAttribute(panel, PANEL_SMARTCAPID3, ATTR_DIMMED, !val);
			break;
		}
	return 0;
}

int CVICALLBACK CharReplaceCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char key;
	char string[4];
	int swallow = 0, vKey;
	
	switch (event)
		{
		case EVENT_KEYPRESS:
			swallow=1;
			key = (char)eventData1;
			if (isprint(key)) {
				sprintf(string, "'%c'",key);
				SetCtrlVal(panel, control, string);
				}
			else {
				vKey = GetKeyPressEventVirtualKey(eventData2);
				if (vKey == VAL_BACKSPACE_VKEY || vKey == VAL_FWD_DELETE_VKEY)
					SetCtrlVal(panel, control, "");
				if (vKey == VAL_TAB_VKEY || vKey == VAL_TAB_VKEY | VAL_SHIFT_MODIFIER)
					swallow=0;
				}
			break;
		case EVENT_VAL_CHANGED:
			GetCtrlVal(panel, control, string);
			key = string[0];
			if (isprint(key)) {
				sprintf(string, "'%c'",key);
				SetCtrlVal(panel, control, string);
				}
			else
				SetCtrlVal(panel, control, "");
			break;
		}
	return swallow;
}

void ReplaceUnusableChars(char *string)
{
	int i, index,advance;
	char *newstring = NULL;
	
	newstring = calloc(strlen(string)+1, sizeof(char));
	//strcpy(newstring, string);
	for (i=0,index=0;i<strlen(string);i++,index++) {
		advance=1;
		if (string[i] == '\\') {
			advance=0;
			if (replaceChar[0])
				newstring[index] = replaceChar[0];
			else
				index--;
			}
		if (string[i] == '/') {
			advance=0;
			if (replaceChar[1])
				newstring[index] = replaceChar[1];
			else
				index--;
			}
		if (string[i] == ':') {
			advance=0;
			if (replaceChar[2])
				newstring[index] = replaceChar[2];
			else
				index--;
			}
		if (string[i] == '*') {
			advance=0;
			if (replaceChar[3])
				newstring[index] = replaceChar[3];
			else
				index--;
			}
		if (string[i] == '?') {
			advance=0;
			if (replaceChar[4])
				newstring[index] = replaceChar[4];
			else
				index--;
			}
		if (string[i] == '"') {
			advance=0;
			if (replaceChar[5])
				newstring[index] = replaceChar[5];
			else
				index--;
			}
		if (string[i] == '<') {
			advance=0;
			if (replaceChar[6])
				newstring[index] = replaceChar[6];
			else
				index--;
			}
		if (string[i] == '>') {
			advance=0;
			if (replaceChar[7])
				newstring[index] = replaceChar[7];
			else
				index--;
			}
		if (string[i] == '|') {
			advance=0;
			if (replaceChar[8])
				newstring[index] = replaceChar[8];
			else
				index--;
			}
		if (advance)
			newstring[index] = string[i]; // copy the char
		}
		
	strcpy(string, newstring);
	free (newstring);
}

int CVICALLBACK LoadArtworkCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char imgpath[MAX_FILENAME_LEN * 2], sizeStr[100];
	int status, bitmap, width, height;
	
	switch (event)
		{
		case EVENT_COMMIT:
#if _CVI_ <= 1001 
			status = FileSelectPopup("", "*.jpg","*.jpg; *.png; *.tif; *.bmp", 
				"Select an image file", VAL_LOAD_BUTTON, 0, 0, 1, 1, imgpath);
#else
			status = FileSelectPopupEx("", "*.jpg","*.jpg; *.png; *.tif; *.bmp", 
				"Select an image file", VAL_LOAD_BUTTON, 0, 0, imgpath);
#endif
			if (status == VAL_EXISTING_FILE_SELECTED) {
				// image selected
				DisplayImageFile(tab2Handle, TAB2_ARTWORK, imgpath);
				GetCtrlBitmap(tab2Handle, TAB2_ARTWORK, 0, &bitmap);
				GetBitmapData(bitmap, NULL, NULL, &width, &height, NULL, NULL, NULL);
				sprintf(sizeStr, kPictureSizeStr, width, height);
				SetCtrlVal(tab2Handle, TAB2_IMAGESIZEMSG, sizeStr);
				DiscardBitmap(bitmap);
				}
			else {
				sprintf(sizeStr, kPictureSizeStr, 0, 0);
				SetCtrlVal(tab2Handle, TAB2_IMAGESIZEMSG, sizeStr);
				}
			break;
		}
	return 0;
}

int CVICALLBACK ClearArtworkCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &val);
			SetCtrlAttribute(panel, TAB2_ARTWORK, ATTR_DIMMED, val);
			break;
		}
	return 0;
}

int CVICALLBACK DownloadArtworkCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double currTime;
	static double lastTime = 0;
	int len, val;
	char albumArtStr[1000];
	char *str=NULL;
	char fileName[20];
	ssize_t fileSize;
	
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal(panel, PANEL_IMAGERING, &val);
			switch (val) {
				case VAL_FRONT:
					sprintf(fileName, "folder.%%extension%%\0");
					break;
				case VAL_BACK:
					sprintf(fileName, "back.%%extension%%\0");
					break;
				case VAL_CD:
					sprintf(fileName, "CD.%%extension%%\0");
					break;
				case VAL_CD2:
					sprintf(fileName, "CD2.%%extension%%\0");
					break;
				case VAL_INSIDE:
					sprintf(fileName, "inside.%%extension%%\0");
					break;
				case VAL_TRAY:
					sprintf(fileName, "tray.%%extension%%\0");
					break;
				case VAL_THUMB:
					sprintf(fileName, "thumb.%%extension%%\0");
					break;
				}
			// "..\AlbumArtDownloader\AlbumArt.exe" /f folder.%extension% /path "$directory_path(%path%)" /autoclose /ar "%artist%" /al "%album%"
			sprintf(albumArtStr, "%s /path \"%s\\%s\" /autoclose \0", albumArtDLPath, pathName, fileName);
			GetCtrlAttribute(panel, PANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &len);
			if (len > 0) {
				str = malloc(len+6 * sizeof(char));
				strcpy(str, "/ar \"");
				GetCtrlVal(panel, PANEL_ARTIST, str+5);
				ReplaceDiacritics(str);
				strcat(albumArtStr, str);
				strcat(albumArtStr, "\" ");
				free(str);
				}
			GetCtrlAttribute(panel, PANEL_ALBUM, ATTR_STRING_TEXT_LENGTH, &len);
			if (len > 0) {
				str = malloc(len+6 * sizeof(char));
				strcpy(str, "/al \"");
				GetCtrlVal(panel, PANEL_ALBUM, str+5);
				ReplaceDiacritics(str);
				strcat(albumArtStr, str);
				strcat(albumArtStr, "\" ");
				free(str);
				}
			LaunchExecutable(albumArtStr);
			gRetrievedFolderJpgData = 0;
			break;
		case EVENT_MOUSE_POINTER_MOVE:
			GetCurrentDateTime(&currTime);
			if (currTime - lastTime > 8.0 || !gRetrievedFolderJpgData) {	// check once every 8 seconds to see if file has changed
				sprintf(albumArtStr, "%s\\folder.jpg", pathName);	// only check the file size of folder.jpg
				DisableBreakOnLibraryErrors();
				if (pathName[0] != '\0' && GetFileSize(albumArtStr, &fileSize) != -1 && fileSize != folderImageFileSize) {
					int bitmap, width, height;
					
					EnableBreakOnLibraryErrors();
					folderImageFileSize = fileSize;
					GetBitmapFromFile(albumArtStr, &bitmap);
					GetBitmapData(bitmap, NULL, NULL, &width, &height, NULL, NULL, NULL);
					DiscardBitmap(bitmap);
					sprintf(albumArtStr, "folder.jpg (%d KB)\n%d x %d", fileSize/1024, width, height);
					SetCtrlAttribute(panel, control, ATTR_TOOLTIP_TEXT, albumArtStr);
					SetCtrlAttribute(panel, PANEL_IMAGERING, ATTR_TOOLTIP_TEXT, albumArtStr);
					SetCtrlAttribute(panel, control, ATTR_DISABLE_CTRL_TOOLTIP, 0);
					SetCtrlAttribute(panel, PANEL_IMAGERING, ATTR_DISABLE_CTRL_TOOLTIP, 0);
					gRetrievedFolderJpgData = 1;
					GetCurrentDateTime(&lastTime);
				}
				EnableBreakOnLibraryErrors();	// in case the file wasn't found
			}
			break;
		}
	return 0;
}

int CVICALLBACK RenameFolderCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char folder[MAX_PATHNAME_LEN*2], new[MAX_PATHNAME_LEN*2], titleName[MAX_PATHNAME_LEN];
	char *artist = NULL, *album = NULL, *year = NULL, *oldFolder = NULL, *folderStart;
	int artistLen=0, albumLen, yearLen, i, usePerfSortOrder, error;
	
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlAttribute(panel, PANEL_ALBUM, ATTR_STRING_TEXT_LENGTH, &albumLen);
			GetCtrlAttribute(panel, PANEL_ALBUM, ATTR_STRING_TEXT_LENGTH, &yearLen);
			GetCtrlVal(configHandle, OPTIONS_RENAMEFOLDER, &usePerfSortOrder);
			if (usePerfSortOrder)
				GetCtrlAttribute(tab1Handle, TAB1_PERFORMERSORTORDER, ATTR_STRING_TEXT_LENGTH, &artistLen);
			if (artistLen==0) { 		// no text entered here 
				usePerfSortOrder = 0;	// so disable this option
				GetCtrlAttribute(panel, PANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &artistLen);
				}
			artist = calloc (artistLen + 1, sizeof(char));
			album = calloc (albumLen + 1, sizeof(char));
			year = calloc (yearLen + 1, sizeof(char));
			if (usePerfSortOrder)
				GetCtrlVal(tab1Handle, TAB1_PERFORMERSORTORDER, artist);
			else
				GetCtrlVal(panel, PANEL_ARTIST, artist);
			GetCtrlVal(panel, PANEL_ALBUM, album);
			GetCtrlVal(tab1Handle, TAB1_YEAR, year);
			year[4]='\0';	// only 4 digit year
			sprintf(folder, "\\%s - %s - %s\0", artist, year, album);
			while (folder[strlen(folder)-1] == '.')
				folder[strlen(folder)-1] = '\0';
			ReplaceUnusableChars(folder+1);
			oldFolder = calloc(strlen(pathName)+1, sizeof(char));
			strcpy(oldFolder, pathName);
			folderStart = strrchr(pathName, '\\');
			strcpy(folderStart, folder);	// substring of pathName
			DisableBreakOnLibraryErrors();
			error = rename(oldFolder, pathName);	// renames folder
			for (i=0;i<numFiles;i++) {
				sprintf(new, "%s\\%s", pathName, fileStruct[i].origFileName);
				if (error)
					error += rename(fileStruct[i].origName, new);	// move files if the folder didn't get renamed
				strcpy(fileStruct[i].origName, new);
				}
			if (error)
				remove(oldFolder);
			EnableBreakOnLibraryErrors();
			snprintf(titleName, MAX_PATHNAME_LEN, "MP3Renamer - %s", pathName);
			SetPanelAttribute (panel, ATTR_TITLE, titleName);
			SetCtrlAttribute(panel, PANEL_RENAMEFOLDER, ATTR_DIMMED, 1);
			break;
		}
	
	if (artist)
		free(artist);
	if (album)
		free(album);
	if (year)
		free(year);
	if (oldFolder)
		free(oldFolder);
	return 0;
}

char* GetCtrlStrVal(int panel, int control)
{
	char *str = NULL;
	int len;

	GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
	if (len != 0) {
		str = calloc(len+1, sizeof(char));
		GetCtrlVal(panel, control, str);
		}
	return str;
}

int CVICALLBACK RenumberTracks (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i;
	char str[5];
	
	switch (event)
		{
		case EVENT_COMMIT:
			for (i=0;i<numFiles;i++) {
				sprintf(str, i+1<10 ? "0%d\0" : "%d\0", i+1);
				SetTreeCellAttribute(panel, PANEL_TREE, i, kTreeColTrackNum, ATTR_LABEL_TEXT, str);
				}
			break;
		}
	return 0;
}

void SetupSplitters(int panel)
{
	int splitterTop, splitterLeft;
	
	gPanelBorderHeight = 0;
	gPanelBorderWidth = 0;
	GetPanelAttribute(panel, ATTR_HEIGHT, &gPanelHeight);
	GetPanelAttribute(panel, ATTR_WIDTH, &gPanelWidth);
	GetCtrlAttribute(panel, PANEL_HSPLITTER, ATTR_TOP, &splitterTop);
	GetCtrlAttribute(panel, PANEL_VSPLITTER, ATTR_LEFT, &splitterLeft);
	GetPanelAttribute(panel, ATTR_WIDTH, &gPanelInitialWidth);
	gSplitterGap = gPanelHeight - splitterTop;
	gVSplitterGap = gPanelWidth - splitterLeft;
}

