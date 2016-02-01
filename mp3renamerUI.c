#include "toolbox.h"
#include <userint.h>
#include <ansi_c.h>
#include "config.h"
#include "ID3v2.h"
#include "mp3renamer.h"

#define kAlbumArtistMsg 	"According to foobar standards, Album Artist should\n" \
							"only be set for albums/compilations which have various\n" \
							"artists but need to be grouped together. Winamp\n" \
							"uses the TPE2 field instead of a text information\n" \
							"field which is the more accepted standard.\n\n" \
							"You can toggle between the two modes using the\n" \
							"\"Use Winamp style Album Artist\" checkbox.\0"

extern Point tagCell;

void InitUIAttrs(void)
{
	int color, height;
	
	PopulateGenreComboBox(tab1Handle, TAB1_GENRE);
	SetTableCellAttribute(tab1Handle, TAB1_GENRE, tagCell, ATTR_MAX_ENTRY_LENGTH, kMaxGenreEntryLength);
	GetCtrlAttribute(tab1Handle, TAB1_ENCODED, ATTR_TEXT_BGCOLOR, &color);
	SetCtrlAttribute(tab1Handle, TAB1_ALBUMGAIN, ATTR_TEXT_BGCOLOR, color);
	SetCtrlAttribute(tab2Handle, TAB2_ARTWORK, ATTR_FRAME_COLOR, VAL_WHITE);
	SetCtrlAttribute(tab1Handle, TAB1_VABUTTON, ATTR_HEIGHT, 13);
	SetCtrlAttribute(tab1Handle, TAB1_SOUNDTRACKBUTTON, ATTR_HEIGHT, 13);
	SetCtrlAttribute(tab1Handle, TAB1_SOUNDTRACKBUTTON, ATTR_WIDTH, 40);
	SetCtrlAttribute(tab1Handle, TAB1_UKBUTTON, ATTR_HEIGHT, 13);
	SetCtrlAttribute(tab1Handle, TAB1_USBUTTON, ATTR_HEIGHT, 13);
	SetCtrlAttribute(tab1Handle, TAB1_GUESSBUTTON, ATTR_HEIGHT, 21);
	SetCtrlAttribute(tab1Handle, TAB1_GUESSBUTTON, ATTR_WIDTH, 40);
	SetTreeColumnAttribute(tab3Handle, TAB3_EXTENDEDTAGS, 2, ATTR_COLUMN_VISIBLE, 0);
	SetCtrlAttribute(panelHandle, PANEL_IMAGERING, ATTR_FRAME_COLOR, VAL_TRANSPARENT);
	SetCtrlAttribute(panelHandle, PANEL_DLARTWORKBUTTON, ATTR_TOOLTIP_DELAY, 500);
	SetCtrlAttribute(panelHandle, PANEL_IMAGERING, ATTR_TOOLTIP_DELAY, 500);
	GetPanelAttribute(panelHandle, ATTR_TITLEBAR_ACTUAL_THICKNESS, &height);
	SetPanelAttribute(panelHandle, ATTR_TOP, 10+height);
	SetPanelAttribute(panelHandle, ATTR_LEFT, 10);
	SetCtrlAttribute(tab1Handle, TAB1_GENRE, ATTR_FIRST_VISIBLE_ROW, 1);
	SetCtrlAttribute(tab1Handle, TAB1_RELTYPE, ATTR_FIRST_VISIBLE_ROW, 1);
}

void SetToolTipInfo(void)
{
	SetCtrlAttribute(panelHandle, PANEL_RENUMBERBUTTON, ATTR_TOOLTIP_TEXT, 
		"Renumbers tracks starting from 01 in the\nsame order they are currently listed.\0");
	SetCtrlAttribute(tab1Handle, TAB1_ALBUMARTIST, ATTR_TOOLTIP_TEXT, kAlbumArtistMsg);
	SetCtrlAttribute(panelHandle, PANEL_USEWINAMPALBUMARTIST, ATTR_TOOLTIP_TEXT, kAlbumArtistMsg);
	SetCtrlAttribute(tab1Handle, TAB1_ALBUMARTIST, ATTR_TOOLTIP_DELAY, 250);
	SetCtrlAttribute(tab1Handle, PANEL_USEWINAMPALBUMARTIST, ATTR_TOOLTIP_DELAY, 250);
	SetCtrlAttribute(configHandle, OPTIONS_SHOWREPLAYGAIN, ATTR_TOOLTIP_TEXT, 
		"When an MP3 has ReplayGain info it is\nfiltered out of the \"Extended Tags\"\ntab by default.\0");
}

void ClearID3Fields(void)
{
	ResetTextBox(tab1Handle, TAB1_COMMENT, "");
	SetTableCellVal(tab1Handle, TAB1_GENRE, tagCell, "");
	SetTableCellVal(tab1Handle, TAB1_RELTYPE, tagCell, "");
	SetCtrlVal (tab1Handle, TAB1_YEAR, "");
	SetCtrlVal (tab1Handle, TAB1_DISCNUM, "");
	SetCtrlVal (tab1Handle, TAB1_COMPOSER, "");
	SetCtrlVal (tab1Handle, TAB1_PUBLISHER, "");
	SetCtrlVal (tab1Handle, TAB1_ENCODED, "");
	SetCtrlVal (tab1Handle, TAB1_COUNTRY, "");
	//SetCtrlVal (tab1Handle, TAB1_RELTYPE, "");
	SetCtrlVal (tab1Handle, TAB1_ALBUMARTIST, "");
	SetCtrlVal (tab1Handle, TAB1_ARTISTFILTER, "");
	SetCtrlVal (tab1Handle, TAB1_PERFORMERSORTORDER, "");
	SetCtrlVal (tab1Handle, TAB1_ALBUMGAIN, "");
	SetCtrlVal (tab1Handle, TAB1_ALBUMSORTORDER, "");
	SetCtrlVal (tab2Handle, TAB2_ORIGARTIST, "");
	SetCtrlVal (tab2Handle, TAB2_URL, "");
	DeleteImage(tab2Handle, TAB2_ARTWORK);
	SetCtrlVal (tab2Handle, TAB2_COPYRIGHT, "");
	SetCtrlVal (tab3Handle, TAB3_ARTISTMBID, "");
	SetCtrlVal (tab3Handle, TAB3_REID, "");
}

int CVICALLBACK GuessTitlesCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i=0, len, capitalize, smartcaps;
	char album[255], artist[255], *songTitle = NULL, *removeStr = NULL, *title, *cap, *ext;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle, PANEL_CAPITALIZE, &capitalize);
			GetCtrlVal (panelHandle, PANEL_SMARTCAPS, &smartcaps);
			//GetCtrlVal (panelHandle, PANEL_SMARTCAPID3, &smartID3);
			if (/*smartID3 &&*/ capitalize && smartcaps) {
				GetCtrlVal (panelHandle, PANEL_ALBUM, album);
				GetCtrlVal (panelHandle, PANEL_ARTIST, artist);
				DoSmartCaps(album);
				SetCtrlVal (panelHandle, PANEL_ALBUM, album);
		
				DoSmartCaps(artist);
				SetCtrlVal (panelHandle, PANEL_ARTIST, artist);
			}
			GetCtrlAttribute (panelHandle, PANEL_REMOVESTRING, ATTR_STRING_TEXT_LENGTH, &len);
			if (len) {
				removeStr = malloc(sizeof(char) * len + 1);
				GetCtrlVal (panelHandle, PANEL_REMOVESTRING, removeStr);
			}
			for (i=0;i<numFiles;i++) {
				GetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackName, ATTR_LABEL_TEXT_LENGTH, &len);
				if (len) {
					songTitle = malloc(sizeof(char) * len + 1);
					GetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackName, ATTR_LABEL_TEXT, songTitle);
					if (removeStr)
						RemoveSpecifiedString(songTitle, removeStr);
					if (capitalize && smartcaps)
						DoSmartCaps(songTitle);
					SetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackName, ATTR_LABEL_TEXT, songTitle);
					if (songTitle)
						free(songTitle);
					}
				else {
					songTitle = malloc(sizeof(char) * strlen(fileStruct[i].newName) + 1);
					strcpy(songTitle, fileStruct[i].newName);
					title = FindTrackNum(songTitle, i, &len, NULL);
					if (!title)
						title = songTitle;
					title = SkipToTrackNameStart(title);
					if (removeStr)
						RemoveSpecifiedString(title, removeStr);
					if (capitalize) {	/* capitalize every first letter */
						cap = title;
						while (cap[0] != title[strlen(title)]) {
							while (isspace(cap[0]) || cap[0] == '-' || cap[0] == '(')
								cap++;
							if (islower(cap[0]))
								cap[0] -= 32;
							cap++;
							while (cap[0] != ' ' && cap[0] != '-' && cap[0]!=title[strlen(title)])
								cap++;
							}
						}
					if (capitalize && smartcaps)
						DoSmartCaps(title);
					ext = strrchr(title, '.');
					title[ext-title] = '\0';
					SetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackName, ATTR_LABEL_TEXT, title);
					free(songTitle);
					}
				}
			break;
		}
	if (removeStr)
		free(removeStr);
	return 0;
}

int CVICALLBACK GuessSortOrderCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char *str = NULL, *newStr = NULL, *space;
	switch (event)
		{
		case EVENT_COMMIT:
			str = GetCtrlStrVal(panelHandle, PANEL_ARTIST);
			RemoveSurroundingWhiteSpace(str);
			if (!strncmp("The ", str, 4) || !strncmp("the ", str, 4))
				space = str+3;
			else
				space = strrchr(str, ' ');
			if (space) {
				newStr = calloc(strlen(str) + 2, sizeof(char));
				sprintf(newStr, "%s, ", space+1);
				strncat(newStr, str, strlen(str)-strlen(space));
				SetCtrlVal(panel, TAB1_PERFORMERSORTORDER, newStr);
				}
			else {
				SetCtrlVal(panel, TAB1_PERFORMERSORTORDER, str);
				}
			SetCtrlAttribute(panelHandle, PANEL_RENAMEFOLDER, ATTR_DIMMED, 0);
			break;
		}
	if (str)
		free(str);
	if (newStr)
		free(newStr);
	return 0;
}

int CVICALLBACK ImageRingCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	char buf[50];
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &val);
			switch (val) {
				case VAL_FRONT:
					sprintf(buf, "__DL %s Cover   \0", "Front");
					SetCtrlAttribute(panel, PANEL_DLARTWORKBUTTON, ATTR_LABEL_TEXT, buf);
					break;
				case VAL_BACK:
					sprintf(buf, "__DL %s Cover   \0", "Back");
					SetCtrlAttribute(panel, PANEL_DLARTWORKBUTTON, ATTR_LABEL_TEXT, buf);
					break;
				case VAL_CD:
					sprintf(buf, "__DL %s Art   \0", "CD");
					SetCtrlAttribute(panel, PANEL_DLARTWORKBUTTON, ATTR_LABEL_TEXT, buf);
					break;
				case VAL_CD2:
					sprintf(buf, "__DL %s Art   \0", "CD2");
					SetCtrlAttribute(panel, PANEL_DLARTWORKBUTTON, ATTR_LABEL_TEXT, buf);
					break;
				case VAL_INSIDE:
					sprintf(buf, "__DL %s Art   \0", "Inside");
					SetCtrlAttribute(panel, PANEL_DLARTWORKBUTTON, ATTR_LABEL_TEXT, buf);
					break;
				case VAL_TRAY:
					sprintf(buf, "__DL %s Art   \0", "Tray");
					SetCtrlAttribute(panel, PANEL_DLARTWORKBUTTON, ATTR_LABEL_TEXT, buf);
					break;
				case VAL_THUMB:
					sprintf(buf, "__DL %s Art   \0", "Thumb");
					SetCtrlAttribute(panel, PANEL_DLARTWORKBUTTON, ATTR_LABEL_TEXT, buf);
					break;
				}
			if (eventData1==0) {	// we pass in a value for eventData1 when manually setting this
				SetActiveCtrl(panel, PANEL_DLARTWORKBUTTON);
				DownloadArtworkCB(panel, PANEL_DLARTWORKBUTTON, EVENT_COMMIT, NULL, 0, 0);
				}
			break;
		case EVENT_LEFT_CLICK_UP:
		case EVENT_GOT_FOCUS:
			SetActiveCtrl(panel, PANEL_DLARTWORKBUTTON);
			break;
		}
	return 0;
}

int CVICALLBACK PanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	int 	splitterTop, splitterLeft;
	Rect	rect;
	
	switch (event)
		{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitCallback(panel, PANEL_QUITBUTTON, EVENT_COMMIT, NULL, 0, 0);
			break;
		case EVENT_PANEL_SIZING:
			if (gPanelBorderHeight == 0 && gPanelBorderWidth == 0) {
				GetPanelEventRect(eventData2, &rect);
				gPanelBorderHeight = rect.height - gPanelHeight;
				gPanelBorderWidth = rect.width - gPanelWidth;
				}
			if (eventData1 != PANEL_SIZING_RIGHT && eventData1 != PANEL_SIZING_LEFT) {	// do for all other sizes
				GetPanelEventRect(eventData2, &rect);
				GetCtrlAttribute(panel, PANEL_HSPLITTER, ATTR_TOP, &splitterTop);
				if (rect.height < 600 + gPanelBorderHeight) {
					rect.height = 600 + gPanelBorderHeight;
					SetPanelEventRect(eventData2, rect);
					}
				if (rect.height - gPanelBorderHeight - splitterTop != gSplitterGap)
	            	OperateSplitter(panel, PANEL_HSPLITTER, rect.height - gPanelBorderHeight - splitterTop - gSplitterGap);
				gPanelHeight = rect.height - gPanelBorderHeight;
				}
			if (eventData1 != PANEL_SIZING_TOP && eventData1 != PANEL_SIZING_BOTTOM) {	// do for all other sizes
				GetPanelEventRect(eventData2, &rect);
				GetCtrlAttribute(panel, PANEL_VSPLITTER, ATTR_LEFT, &splitterLeft);
				if (rect.width < gPanelInitialWidth + gPanelBorderWidth) {
					rect.width = gPanelInitialWidth + gPanelBorderWidth;
					SetPanelEventRect(eventData2, rect);
					}
				if (rect.width - gPanelBorderWidth - splitterLeft != gVSplitterGap)
	            	OperateSplitter(panel, PANEL_VSPLITTER, rect.width - gPanelBorderWidth - splitterLeft - gVSplitterGap);
				gPanelWidth = rect.width - gPanelBorderWidth;
				ShowLengthCB(panel, PANEL_TREE, EVENT_COMMIT, NULL, 0, 0);
				}
			break;
		}
	return 0;
}

/**********************************************************/

int CVICALLBACK ShowLengthCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int treeWidth, thickness, sbSize, newWidth;
	int width, colTitleWidth, col1Width, col2Width, col3Width;
	switch (event)
		{
		case EVENT_COMMIT:
		case EVENT_COLUMN_SIZE_CHANGE:
			GetCtrlAttribute(panelHandle, PANEL_TREE, ATTR_WIDTH, &treeWidth);
			GetCtrlAttribute(panelHandle, PANEL_TREE, ATTR_FRAME_THICKNESS, &thickness);
			GetCtrlAttribute(panelHandle, PANEL_TREE, ATTR_SCROLL_BAR_SIZE, &sbSize);
			treeWidth = treeWidth-thickness-sbSize-3;	// not sure what the discrepency is
			GetTreeColumnAttribute(panelHandle, PANEL_TREE, kTreeColFilename, ATTR_COLUMN_WIDTH, &width);
			GetTreeColumnAttribute(panelHandle, PANEL_TREE, kTreeColTrackName, ATTR_COLUMN_WIDTH, &colTitleWidth);
			GetTreeColumnAttribute(panelHandle, PANEL_TREE, kTreeColTrackNum, ATTR_COLUMN_WIDTH, &col1Width);
			if (event == EVENT_COLUMN_SIZE_CHANGE && eventData2 == 0)
				newWidth = treeWidth-width-col1Width;
			else
				newWidth = treeWidth-colTitleWidth-col1Width;
			if (newWidth < 100)
				newWidth = 100;
			if (event == EVENT_COLUMN_SIZE_CHANGE && eventData2 == 0)
				SetTreeColumnAttribute(panelHandle, PANEL_TREE, kTreeColTrackName, ATTR_COLUMN_WIDTH, newWidth);
			else
				SetTreeColumnAttribute(panelHandle, PANEL_TREE, kTreeColFilename, ATTR_COLUMN_WIDTH, newWidth);
			
			/* Size the Album Panel Tree Columns here too cause there's not a great place for it */
			GetCtrlAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, ATTR_WIDTH, &treeWidth);
			GetCtrlAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, ATTR_FRAME_THICKNESS, &thickness);
			GetCtrlAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, ATTR_SCROLL_BAR_SIZE, &sbSize);
			treeWidth = treeWidth-thickness-sbSize-3;	// not sure what the discrepency is
			GetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColArtist, ATTR_COLUMN_WIDTH, &width);
			GetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColAlbum, ATTR_COLUMN_WIDTH, &colTitleWidth);
			GetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColNumTracks, ATTR_COLUMN_WIDTH, &col1Width);
			GetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColDate, ATTR_COLUMN_WIDTH, &col2Width);
			GetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColCountry, ATTR_COLUMN_WIDTH, &col3Width);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColAlbum, ATTR_COLUMN_WIDTH, treeWidth-width-col1Width-col2Width-col3Width);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColREID, 		ATTR_COLUMN_VISIBLE, 0);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColNumDiscs,	ATTR_COLUMN_VISIBLE, 0);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColASIN, 		ATTR_COLUMN_VISIBLE, 0);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColBarcode, 	ATTR_COLUMN_VISIBLE, 0);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColCatalog, 	ATTR_COLUMN_VISIBLE, 0);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColLabel, 		ATTR_COLUMN_VISIBLE, 0);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColArtistID,	ATTR_COLUMN_VISIBLE, 0);
			SetTreeColumnAttribute(albumPanHandle, ALBUMPANEL_ALBUMTREE, kAlbTreeColRelGroupID,	ATTR_COLUMN_VISIBLE, 0);
			break;
		}
	return 0;
}

int CVICALLBACK ID3v1CB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int val;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, control, &val);
			SetCtrlAttribute(panel, PANEL_ID3V1COMMENTS, ATTR_DIMMED, !val);
			break;
	}
	return 0;
}

int CVICALLBACK TreeCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int swallowed = 0, column, rows;
	
	switch (event)
		{
		case EVENT_COMMIT:
			if (eventData1 == LABEL_CHANGE)
				GetLabelFromIndex (panel, PANEL_TREE, eventData2, fileStruct[eventData2].newName);
			break;
		case EVENT_BEGIN_EDIT_TREE_CELL:
			column = eventData2;	// don't allow the user to edit the filename without hitting F2
			if (column == kTreeColFilename)
				swallowed = 1;
			break;
		case EVENT_TREE_CELL_COMMIT:
			column = eventData2;
			if (column == kTreeColFilename)
				GetLabelFromIndex (panel, PANEL_TREE, eventData1, fileStruct[eventData1].newName);
			else {
				GetNumTreeItems(panel, PANEL_TREE, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, &rows);
				if (eventData1+1 < rows)
					SetTreeCellAttribute(panel, PANEL_TREE, eventData1+1, eventData2, ATTR_TREE_RUN_STATE, VAL_EDIT_STATE);
			}
			break;
		case EVENT_COLUMN_SIZE_CHANGE:
			ShowLengthCB(panel, control, event, NULL, eventData1, eventData2);
			break;
		case EVENT_VSCROLL:
			break;
		}
	return swallowed;
}


void CenterPopupPanel(int parent, int child)
{
	int x, y, height, width, childHeight, childWidth;
	
	GetPanelAttribute (parent, ATTR_TOP, &y);
	GetPanelAttribute (parent, ATTR_LEFT, &x);
	GetPanelAttribute (parent, ATTR_HEIGHT, &height);
	GetPanelAttribute (parent, ATTR_WIDTH, &width);
	GetPanelAttribute (child, ATTR_HEIGHT, &childHeight);
	GetPanelAttribute (child, ATTR_WIDTH, &childWidth);
	SetPanelAttribute (child, ATTR_LEFT, (x + width-childWidth) / 2);
	SetPanelAttribute (child, ATTR_TOP, (y + height-childHeight) / 2);
}

int CVICALLBACK ExploreCurrentCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned long major, minor;
	char command[MAX_PATHNAME_LEN*2];
	
	switch (event)
	{
		case EVENT_COMMIT:
			GetWinOSVersion(&major, &minor, NULL, NULL);
			if (major >= 6)	// Vista or later
				sprintf(command, "cmd /C explorer \"%s\"", pathName[0] ? pathName : ".");
			else
				sprintf(command, "cmd /C explorer /e, \"%s\"", pathName[0] ? pathName : ".");
			LaunchExecutableEx(command, LE_HIDE, NULL);
			break;
	}
	return 0;
}

int CVICALLBACK CapitalizeTitlesCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int capitalize, smartcaps, smartID3, len, i;
	char album[255], artist[255], *songTitle, *cap;
	
	switch (event) {
		case EVENT_COMMIT:
			GetCtrlVal (panelHandle, PANEL_CAPITALIZE, &capitalize);
			GetCtrlVal (panelHandle, PANEL_SMARTCAPS, &smartcaps);
			GetCtrlVal (panelHandle, PANEL_SMARTCAPID3, &smartID3);
			if (smartID3 && capitalize && smartcaps) {
				GetCtrlVal (panelHandle, PANEL_ALBUM, album);
				GetCtrlVal (panelHandle, PANEL_ARTIST, artist);
				DoSmartCaps(album);
				SetCtrlVal (panelHandle, PANEL_ALBUM, album);
		
				DoSmartCaps(artist);
				SetCtrlVal (panelHandle, PANEL_ARTIST, artist);
			}
			if (capitalize) {
				for (i=0; i<numFiles;i++) {
					if (IsItemChecked(i)) {
						GetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackName, ATTR_LABEL_TEXT_LENGTH, &len);
						if (len) {
							songTitle = malloc(sizeof(char) * len + 1);
							GetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackName, ATTR_LABEL_TEXT, songTitle);
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
							if (capitalize && smartcaps) {
								DoSmartCaps(songTitle);
							}
							SetTreeCellAttribute(panelHandle, PANEL_TREE, i, kTreeColTrackName, ATTR_LABEL_TEXT, songTitle);
							if (songTitle) {
								free(songTitle);
							}
						}
					}
				}
			}
			break;
		}
	return 0;
}

/*********************************************************************/

int CVICALLBACK ErrorPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	int	key;
	switch (event)
	{
		case EVENT_KEYPRESS:
			key = GetKeyPressEventVirtualKey(eventData2);
			if (key == VAL_ESC_VKEY)
				RemovePopup(0);
			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:

			break;
	}
	return 0;
}

int CVICALLBACK ErrorCtrlCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			RemovePopup(0);
			break;
	}
	return 0;
}

/*********************************************************************/

int CVICALLBACK ValChangeCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int vk;
	switch (event)
	{
		case EVENT_VAL_CHANGED:
			switch (control) {
				case ALBUMPANEL_ALBUM:
					SetCtrlVal(panel, ALBUMPANEL_USEALBUM, 1);
					break;
				case ALBUMPANEL_ARTIST:
					SetCtrlVal(panel, ALBUMPANEL_USEARTIST, 1);
					break;
				case ALBUMPANEL_YEAR:
					SetCtrlVal(panel, ALBUMPANEL_USEYEAR, 1);
					break;
				case ALBUMPANEL_TRACKS:
					SetCtrlVal(panel, ALBUMPANEL_USETRACKS, 1);
					break;
				case ALBUMPANEL_DISCS:
					SetCtrlVal(panel, ALBUMPANEL_USEDISCS, 1);
					break;
				case ALBUMPANEL_TYPE:
					SetCtrlVal(panel, ALBUMPANEL_USETYPE, 1);
					break;
				case ALBUMPANEL_FORMAT:
					SetCtrlVal(panel, ALBUMPANEL_USEFORMAT, 1);
					break;
				case ALBUMPANEL_STATUS:
					SetCtrlVal(panel, ALBUMPANEL_USESTATUS, 1);
					break;
				case ALBUMPANEL_COUNTRY:
					SetCtrlVal(panel, ALBUMPANEL_USECOUNTRY, 1);
					break;
				}
			break;
		case EVENT_KEYPRESS:
			vk = GetKeyPressEventVirtualKey(eventData2);
			if (vk == VAL_ENTER_VKEY)
				RefreshReleasesCB(panel, control, EVENT_COMMIT, NULL, 0, 0);
			break;
	}
	return 0;
}

void SelectAllText (int panel, int control)
{
	int len;
	
	GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
	SetCtrlAttribute(panel, control, ATTR_TEXT_SELECTION_START, 0);
	SetCtrlAttribute(panel, control, ATTR_TEXT_SELECTION_LENGTH, len);
}

int CVICALLBACK TagCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int led = 0;
	
	switch (event)
		{
		case EVENT_RIGHT_CLICK:
			break;
		case EVENT_GOT_FOCUS:
			if (panel == tab1Handle && control == TAB1_COMMENT) {
				SelectAllText(panel, control);
			}
			if (panel == tab3Handle) {
				switch (control) {
					case TAB3_ARTISTMBID:
					case TAB3_REID:
						SelectAllText(panel, control);
						break;
				}
			}
			break;
		case EVENT_KEYPRESS:
			if (panel == tab1Handle) 
				switch (control) {
					case TAB1_COMMENT:
						led = TAB1_COMMENTLED;
						break;
					case TAB1_YEAR:
						led = TAB1_YEARLED;
						SetCtrlAttribute(panelHandle, PANEL_RENAMEFOLDER, ATTR_DIMMED, 0);
						break;
					case TAB1_COMPOSER:
						led = TAB1_COMPOSERLED;
						break;
					case TAB1_ENCODED:
						led = TAB1_ENCODEDLED;
						break;
					case TAB1_ALBUMARTIST:
						led = TAB1_ALBUMARTISTLED;
						break;
					case TAB1_ARTISTFILTER:
						led = TAB1_ARTISTFILTERLED;
						break;
					case TAB1_GENRE:
						led = TAB1_GENRELED;
						break;
					case TAB1_DISCNUM:
						led = TAB1_DISCNUMLED;
						gUseMetaDataDiscVal = FALSE;
						break;
					case TAB1_PUBLISHER:
						led = TAB1_PUBLISHERLED;
						break;
					case TAB1_ALBUMSORTORDER:
						led = TAB1_ALBUMSORTLED;
						break;
					}
			else if (panel == tab2Handle)
				switch (control) {
					case TAB2_ORIGARTIST:
						led = TAB2_ORIGARTISTLED;
						break;
					case TAB2_URL:
						led = TAB2_URLLED;
						break;
					case TAB2_COPYRIGHT:
						led = TAB2_COPYRIGHTLED;
						break;
					}
			if (led)
				SetCtrlVal(panel, led, 0);
			if (eventData1 == (VAL_MENUKEY_MODIFIER | 'a') || eventData1 == (VAL_MENUKEY_MODIFIER | 'A')) {
				SelectAllText(panel, control);
			}
			break;
		}
	return 0;
}

int CVICALLBACK AutoAlbumArtistCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			switch (control) {
				case TAB1_VABUTTON:
					SetCtrlVal(panel, TAB1_ALBUMARTIST, "Various Artists");
					break;
				case TAB1_SOUNDTRACKBUTTON:
					SetCtrlVal(panel, TAB1_ALBUMARTIST, "Soundtrack");
					break;
			}
			SetActiveCtrl(panel, TAB1_ALBUMARTIST);
			TagCB(panel, TAB1_ALBUMARTIST, EVENT_KEYPRESS, NULL, 0, 0);
			break;
	}
	return 0;
}


#define kNumCountryCodes	242

countryEntry countryCodeTable[kNumCountryCodes] = {
	{"US", "United States"},
	{"GB", "United Kingdom"},
	{"AU", "Australia"},
	{"DE", "Germany"},
	{"FR", "France"},
	{"SE", "Sweden"},
	{"NO", "Norway"},
	{"IT", "Italy"},
	{"JP", "Japan"},
	{"CN", "China"},
	{"FI", "Finland"},
	{"KR", "South Korea"},
	{"RU", "Russia"},
	{"IE", "Ireland"},
	{"GR", "Greece"},
	{"IS", "Iceland"},
	{"IN", "India"},
	{"--", ""},
	{"AD", "Andorra"},
	{"AE", "United Arab Emirates"},
	{"AF", "Afghanistan"},
	{"AG", "Antigua and Barbuda"},
	{"AI", "Anguilla"},
	{"AL", "Albania"},
	{"AM", "Armenia"},
	{"AO", "Angola"},
	{"AQ", "Antarctica"},
	{"AR", "Argentina"},
	{"AS", "American Samoa"},
	{"AT", "Austria"},
	{"AW", "Aruba"},
	{"AX", "Åland Islands"},
	{"AZ", "Azerbaijan"},
	{"BA", "Bosnia and Herzegovina"},
	{"BB", "Barbados"},
	{"BD", "Bangladesh"},
	{"BE", "Belgium"},
	{"BF", "Burkina Faso"},
	{"BG", "Bulgaria"},
	{"BH", "Bahrain"},
	{"BI", "Burundi"},
	{"BJ", "Benin"},
	{"BL", "Saint Barthélemy"},
	{"BM", "Bermuda"},
	{"BN", "Brunei Darussalam"},
	{"BO", "Bolivia"},
	{"BQ", "Bonaire, Sint Eustatius and Saba"},
	{"BR", "Brazil"},
	{"BS", "Bahamas"},
	{"BT", "Bhutan"},
	{"BV", "Bouvet Island"},
	{"BW", "Botswana"},
	{"BY", "Belarus"},
	{"BZ", "Belize"},
	{"CA", "Canada"},
	{"CC", "Cocos Keeling Islands"},
	{"CD", "Democratic Republic of the Congo"},
	{"CF", "Central African Republic"},
	{"CH", "Switzerland"},
	{"CI", "Côte d'Ivoire"},
	{"CK", "Cook Islands"},
	{"CL", "Chile"},
	{"CM", "Cameroon"},
	{"CO", "Colombia"},
	{"CR", "Costa Rica"},
	{"CU", "Cuba"},
	{"CV", "Cape Verde"},
	{"CX", "Christmas Island"},
	{"CY", "Cyprus"},
	{"CZ", "Czech Republic"},
	{"DJ", "Djibouti"},
	{"DK", "Denmark"},
	{"DM", "Dominica"},
	{"DO", "Dominican Republic"},
	{"DZ", "Algeria"},
	{"EC", "Ecuador"},
	{"EE", "Estonia"},
	{"EG", "Egypt"},
	{"EH", "Western Sahara"},
	{"ER", "Eritrea"},
	{"ES", "Spain"},
	{"ET", "Ethiopia"},
	{"FJ", "Fiji"},
	{"FK", "Falkland Islands"},
	{"FM", "Micronesia"},
	{"FO", "Faroess"},
	{"GA", "Gabon"},
	{"GD", "Grenada"},
	{"GE", "Georgia"},
	{"GG", "Guernsey"},
	{"GH", "Ghana"},
	{"GI", "Gibraltar"},
	{"GL", "Greenland"},
	{"GM", "Gambia"},
	{"GN", "Guinea"},
	{"GQ", "Equatorial Guinea"},
	{"GS", "South Georgia and the South Sandwich Islands"},
	{"GT", "Guatemala"},
	{"GU", "Guam"},
	{"GW", "Guinea-Bissau"},
	{"GY", "Guyana"},
	{"HK", "Hong Kong"},
	{"HN", "Honduras"},
	{"HR", "Croatia"},
	{"HT", "Haiti"},
	{"HU", "Hungary"},
	{"ID", "Indonesia"},
	{"IL", "Israel"},
	{"IM", "Isle of Man"},
	{"IQ", "Iraq"},
	{"IR", "Iran"},
	{"JE", "Jersey"},
	{"JM", "Jamaica"},
	{"JO", "Jordan"},
	{"KE", "Kenya"},
	{"KG", "Kyrgyzstan"},
	{"KH", "Cambodia"},
	{"KI", "Kiribati"},
	{"KM", "Comoros"},
	{"KN", "Saint Kitts and Nevis"},
	{"KP", "North Korea"},
	{"KW", "Kuwait"},
	{"KY", "Cayman Islands"},
	{"KZ", "Kazakhstan"},
	{"LA", "Laos"},
	{"LB", "Lebanon"},
	{"LC", "Saint Lucia"},
	{"LI", "Liechtenstein"},
	{"LK", "Sri Lanka"},
	{"LR", "Liberia"},
	{"LS", "Lesotho"},
	{"LT", "Lithuania"},
	{"LU", "Luxembourg"},
	{"LV", "Latvia"},
	{"LY", "Libya"},
	{"MA", "Morocco"},
	{"MC", "Monaco"},
	{"MD", "Moldova"},
	{"ME", "Montenegro"},
	{"MF", "Saint Martin"},
	{"MG", "Madagascar"},
	{"MH", "Marshall Islands"},
	{"MK", "Macedonia"},
	{"ML", "Mali"},
	{"MM", "Myanmar"},
	{"MN", "Mongolia"},
	{"MO", "Macao"},
	{"MP", "Northern Mariana Islands"},
	{"MQ", "Martinique"},
	{"MR", "Mauritania"},
	{"MS", "Montserrat"},
	{"MT", "Malta"},
	{"MU", "Mauritius"},
	{"MV", "Maldives"},
	{"MW", "Malawi"},
	{"MX", "Mexico"},
	{"MY", "Malaysia"},
	{"MZ", "Mozambique"},
	{"NA", "Namibia"},
	{"NC", "New Caledonia"},
	{"NE", "Niger"},
	{"NF", "Norfolk Island"},
	{"NG", "Nigeria"},
	{"NI", "Nicaragua"},
	{"NL", "Netherlands"},
	{"NP", "Nepal"},
	{"NR", "Nauru"},
	{"NU", "Niue"},
	{"NZ", "New Zealand"},
	{"OM", "Oman"},
	{"PA", "Panama"},
	{"PE", "Peru"},
	{"PF", "French Polynesia"},
	{"PG", "Papua New Guinea"},
	{"PH", "Philippines"},
	{"PK", "Pakistan"},
	{"PL", "Poland"},
	{"PM", "Saint Pierre and Miquelon"},
	{"PN", "Pitcairn"},
	{"PR", "Puerto Rico"},
	{"PS", "Palestine"},
	{"PT", "Portugal"},
	{"PW", "Palau"},
	{"PY", "Paraguay"},
	{"QA", "Qatar"},
	{"RE", "Réunion"},
	{"RO", "Romania"},
	{"RS", "Serbia"},
	{"RW", "Rwanda"},
	{"SA", "Saudi Arabia"},
	{"SB", "Solomon Islands"},
	{"SC", "Seychelles"},
	{"SD", "Sudan"},
	{"SG", "Singapore"},
	{"SH", "Saint Helena"},
	{"SI", "Slovenia"},
	{"SJ", "Svalbard and Jan Mayen"},
	{"SK", "Slovakia"},
	{"SL", "Sierra Leone"},
	{"SM", "San Marino"},
	{"SN", "Senegal"},
	{"SO", "Somalia"},
	{"SR", "Suriname"},
	{"SS", "South Sudan"},
	{"ST", "Sao Tome and Principe"},
	{"SV", "El Salvador"},
	{"SX", "Sint Maarten"},
	{"SY", "Syrian Arab Republic"},
	{"SZ", "Swaziland"},
	{"TC", "Turks and Caicos Islands"},
	{"TD", "Chad"},
	{"TF", "French Southern Territories"},
	{"TG", "Togo"},
	{"TH", "Thailand"},
	{"TJ", "Tajikistan"},
	{"TK", "Tokelau"},
	{"TL", "Timor-Leste"},
	{"TM", "Turkmenistan"},
	{"TN", "Tunisia"},
	{"TO", "Tonga"},
	{"TR", "Turkey"},
	{"TT", "Trinidad and Tobago"},
	{"TV", "Tuvalu"},
	{"TW", "Taiwan"},
	{"TZ", "Tanzania"},
	{"UA", "Ukraine"},
	{"UG", "Uganda"},
	{"UY", "Uruguay"},
	{"UZ", "Uzbekistan"},
	{"VA", "Vatican City"},
	{"VC", "Saint Vincent and the Grenadines"},
	{"VE", "Venezuela"},
	{"VI", "US Virgin Islands"},
	{"VN", "Vietnam"},
	{"VU", "Vanuatu"},
	{"WF", "Wallis and Futuna"},
	{"WS", "Samoa"},
	{"YE", "Yemen"},
	{"YT", "Mayotte"},
	{"ZA", "South Africa"},
	{"ZM", "Zambia"},
	{"ZW", "Zimbabwe"}
};

void SetCountryName(char countryCode[3])
{
	int i;
	for (i=0; i<kNumCountryCodes;i++) {
		if (!strncmp(countryCode, countryCodeTable[i].code, 2)) {
			SetCtrlVal(tab1Handle, TAB1_COUNTRY, countryCodeTable[i].name);
			break;
		}
	}
	return;
}

int CVICALLBACK AutoArtistCountryCB (int panel, int control, int event,
									 void *callbackData, int eventData1, int eventData2) {
	switch (event) {
		case EVENT_COMMIT:
			switch (control) {
				case TAB1_UKBUTTON:
					SetCtrlVal(panel, TAB1_COUNTRY, "United Kingdom");
					break;
				case TAB1_USBUTTON:
					SetCtrlVal(panel, TAB1_COUNTRY, "United States");
					break;
			}
			SetActiveCtrl(panel, TAB1_COUNTRY);
			TagCB(panel, TAB1_ALBUMARTIST, EVENT_KEYPRESS, NULL, 0, 0);
			break;
	}
	return 0;
}

int CVICALLBACK TableComboCB (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	Point cell;
	int swallow=0;
	switch (event) {
		case EVENT_GOT_FOCUS:
			GetActiveTableCell(panel, control, &cell);
			if (cell.y != 1) {
				cell.y = 1;
				SetActiveTableCell(panel, control, cell);
			}
			break;
		case EVENT_ACTIVE_CELL_CHANGE:
			SetCtrlAttribute(panel, control, ATTR_FIRST_VISIBLE_ROW, 1);
			GetActiveTableCell(panel, control, &cell);
			if (cell.y != 1) {
				switch (control) {
					case TAB1_GENRE:	
						SetActiveCtrl(panel, TAB1_COMMENT);
						break;
					case TAB1_RELTYPE:
						SetActiveCtrl(panel, TAB1_ALBUMARTIST);
						break;
				}
			}
			break;
		case EVENT_LOST_FOCUS: 
			GetActiveTableCell(panel, control, &cell);
			if (cell.y == 1) {
				cell.y = 2;
				SetActiveTableCell(panel, control, cell);
				SetCtrlAttribute(panel, control, ATTR_FIRST_VISIBLE_ROW, 1);
			}
			break;
	}
	return swallow;
}
