#include <userint.h>
#include "pathctrl.h"
#include "inifile.h"
#include "config.h"
#include "mp3renamer.h"

void ReadConfigOptions(int restrictedRead)
{
	int	status;
	unsigned long autoRevert=0, autoGetTag=1, extensions=1, showLength=1, ignoreDisc=0, skipAlbArtist=0, 
				  populateAlbumOrder=0, autoCapitalize=1, smartCaps=1, renameUsePerf=1, showReplayGain=0, 
				  saveID3v1=1, saveV1Comments=0, replaceUnicodeApostrophe=1;
	IniText cfg;
	
	cfg = Ini_New(0);
	status = Ini_ReadFromFile(cfg, kCfgFileName); 
	if (status < 0) {
		if (status == -5001) { // file not found
			Ini_PutRawString(cfg, kIniGeneralOptions, "Initial Search Directory", "C:\\");

			Ini_PutInt(cfg, kIniGeneralOptions, "AutoCapitalize", autoCapitalize);
			Ini_PutInt(cfg, kIniGeneralOptions, "UseSmartCaps", smartCaps);
			Ini_PutInt(cfg, kIniGeneralOptions, "SaveID3v1Tags", saveID3v1);
			Ini_PutInt(cfg, kIniGeneralOptions, "SaveCommentsInID3v1", saveV1Comments);
			
			Ini_PutInt(cfg, kIniGeneralOptions, "AutoGetTag", autoGetTag);
			Ini_PutInt(cfg, kIniGeneralOptions, "AutoRevert", autoRevert);
			Ini_PutInt(cfg, kIniGeneralOptions, "LowerCaseExtensions", extensions);
			Ini_PutInt(cfg, kIniGeneralOptions, "ShowFileNameLength", showLength);
			Ini_PutInt(cfg, kIniGeneralOptions, "IgnoreDiscNumIfEquals1of1", ignoreDisc);
			Ini_PutInt(cfg, kIniGeneralOptions, "SkipAlbumArtistIfEqualArtist", skipAlbArtist);
			Ini_PutInt(cfg, kIniGeneralOptions, "PopulateAlbumOrderWithYear", populateAlbumOrder);
			Ini_PutInt(cfg, kIniGeneralOptions, "RenameUsesPerfSortOrder", renameUsePerf);
			Ini_PutInt(cfg, kIniGeneralOptions, "ShowReplayGain", showReplayGain);
			Ini_PutInt(cfg, kIniGeneralOptions, "ReplaceUnicodeApostrophe", replaceUnicodeApostrophe);
			
			Ini_PutRawString(cfg, kIniExternalPrograms, "Album Art Downloader", "");
			
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceBackslash", " "); 
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceForewardslash", " ");
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceColon", " ");
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceAsterix", " ");
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceQuestion", " ");
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceQuote", " ");
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceLessThan", " ");
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceGreaterThan", " ");
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplacePipe", " ");
			
			Ini_WriteToFile(cfg, kCfgFileName);
			}
		}
	Ini_GetRawStringIntoBuffer(cfg, kIniGeneralOptions, "Initial Search Directory", startFolder, MAX_PATHNAME_LEN*2);
	Ini_GetRawStringIntoBuffer(cfg, kIniGeneralOptions, "Fanart Personal API Key", fanartPersonalAPIKey, 33);
	Ini_GetInt(cfg, kIniGeneralOptions, "AutoCapitalize", &autoCapitalize);
	Ini_GetInt(cfg, kIniGeneralOptions, "UseSmartCaps", &smartCaps);
	Ini_GetInt(cfg, kIniGeneralOptions, "SaveID3v1Tags", &saveID3v1);
	Ini_GetInt(cfg, kIniGeneralOptions, "SaveCommentsInID3v1", &saveV1Comments);
	
	Ini_GetInt(cfg, kIniGeneralOptions, "AutoGetTag", &autoGetTag);
	Ini_GetInt(cfg, kIniGeneralOptions, "AutoRevert", &autoRevert);
	Ini_GetInt(cfg, kIniGeneralOptions, "LowerCaseExtensions", &extensions);
	Ini_GetInt(cfg, kIniGeneralOptions, "ShowFileNameLength", &showLength);
	Ini_GetInt(cfg, kIniGeneralOptions, "IgnoreDiscNumIfEquals1of1", &ignoreDisc);
	Ini_GetInt(cfg, kIniGeneralOptions, "SkipAlbumArtistIfEqualArtist", &skipAlbArtist);
	Ini_GetInt(cfg, kIniGeneralOptions, "PopulateAlbumOrderWithYear", &populateAlbumOrder);
	Ini_GetInt(cfg, kIniGeneralOptions, "RenameUsesPerfSortOrder", &renameUsePerf);
	Ini_GetInt(cfg, kIniGeneralOptions, "ShowReplayGain", &showReplayGain);
	Ini_GetInt(cfg, kIniGeneralOptions, "ReplaceUnicodeApostrophe", &replaceUnicodeApostrophe);
			
	Ini_GetRawStringIntoBuffer(cfg, kIniExternalPrograms, "Album Art Downloader", albumArtDLPath, MAX_PATHNAME_LEN*2);
	Ini_GetRawStringIntoBuffer(cfg, kIniExternalPrograms, "Artist Logo Directory", artistLogoPath, MAX_PATHNAME_LEN*2);
	
	if (!restrictedRead) {
		SetCtrlVal(panelHandle, PANEL_CAPITALIZE, autoCapitalize);
		SetCtrlVal(panelHandle, PANEL_SMARTCAPS, smartCaps);
		SetCtrlVal(panelHandle, PANEL_DOID3V1, saveID3v1);
		SetCtrlVal(panelHandle, PANEL_ID3V1COMMENTS, saveV1Comments);
	}
	SetCtrlVal(configHandle, OPTIONS_PATH, startFolder);
	SetCtrlVal(configHandle, OPTIONS_FANART_APIKEY, fanartPersonalAPIKey);
	SetCtrlVal(configHandle, OPTIONS_AUTOGETTAG, autoGetTag);
	SetCtrlVal(configHandle, OPTIONS_AUTOREVERT, autoRevert);
	SetCtrlVal(configHandle, OPTIONS_EXTENSIONS, extensions);
	SetCtrlVal(configHandle, OPTIONS_SHOWLENGTH, showLength);
	SetCtrlVal(configHandle, OPTIONS_IGNOREDISC1OF1, ignoreDisc);
	SetCtrlVal(configHandle, OPTIONS_SKIPALBUMARTIST, skipAlbArtist);
	SetCtrlVal(configHandle, OPTIONS_ALBUMARTPATH, albumArtDLPath);
	SetCtrlVal(configHandle, OPTIONS_HDLOGOPATH, artistLogoPath);
	SetCtrlVal(configHandle, OPTIONS_POPULATEALBUMORDER, populateAlbumOrder);
	SetCtrlVal(configHandle, OPTIONS_RENAMEFOLDER, renameUsePerf);
	SetCtrlVal(configHandle, OPTIONS_SHOWREPLAYGAIN, showReplayGain);
	SetCtrlVal(configHandle, OPTIONS_REPLACEAPOSTROPHE, replaceUnicodeApostrophe);
	if (!strcmp(albumArtDLPath, ""))
		SetCtrlAttribute(panelHandle, PANEL_DLARTWORKBUTTON, ATTR_DIMMED, 1);
	if (!strcmp(artistLogoPath, ""))
		SetCtrlAttribute(panelHandle, PANEL_FANART, ATTR_DIMMED, 1);

	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceBackslash", &replaceChar[0], 2); 
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceForewardslash", &replaceChar[1], 2);
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceColon", &replaceChar[2], 2);
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceAsterix", &replaceChar[3], 2);
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceQuestion", &replaceChar[4], 2);
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceQuote", &replaceChar[5], 2);
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceLessThan", &replaceChar[6], 2);
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplaceGreaterThan", &replaceChar[7], 2);
	Ini_GetStringIntoBuffer(cfg, kIniReplaceCharSection, "ReplacePipe", &replaceChar[8], 2);
	SetReplacementChars();

Error:
	Ini_Dispose(cfg);
}

void CVICALLBACK OptionsCB (int menuBar, int menuItem, void *callbackData, int panel)
{
	int x, y, height, width, childHeight, childWidth;
	
	GetPanelAttribute (panel, ATTR_TOP, &y);
	GetPanelAttribute (panel, ATTR_LEFT, &x);
	GetPanelAttribute (panel, ATTR_HEIGHT, &height);
	GetPanelAttribute (panel, ATTR_WIDTH, &width);
	GetPanelAttribute (configHandle, ATTR_HEIGHT, &childHeight);
	GetPanelAttribute (configHandle, ATTR_WIDTH, &childWidth);
	SetPanelAttribute (configHandle, ATTR_LEFT, (x + width-childWidth) / 2);
	SetPanelAttribute (configHandle, ATTR_TOP, (y + height-childHeight) / 2);
	GetCtrlAttribute (configHandle, OPTIONS_PATH, ATTR_HEIGHT, &height);
	SetCtrlAttribute (configHandle, OPTIONS_DIRSEARCH, ATTR_WIDTH, height);
	SetCtrlAttribute (configHandle, OPTIONS_DIRSEARCH, ATTR_HEIGHT, height);
	SetCtrlAttribute (configHandle, OPTIONS_ARTDOWNLOADERPATH, ATTR_WIDTH, height);
	SetCtrlAttribute (configHandle, OPTIONS_ARTDOWNLOADERPATH, ATTR_HEIGHT, height);
	NewPathCtrl(configHandle, OPTIONS_PATH, 8, 1);
	SetPathCtrlAttribute(configHandle, OPTIONS_PATH, ATTR_PATH_CTRL_FILE_SPECIFICATION, kPathCtrlDirectoryFile);
	InstallPopup (configHandle);
}

int CVICALLBACK OptionsOKCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	unsigned long autoRevert, autoGetTag, extensions, showLength, ignoreDisc, skipAlbArtist, populateAlbumOrder,
				  autoCap, smartCaps, renameUsePerf, showReplayGain, saveID3v1, saveV1Comments, replaceUnicodeApostrophe;
	char 	saveChar[2] = {0,0};
	IniText cfg=0;
	
	switch (event)
		{
		case EVENT_COMMIT:
			cfg = Ini_New(0);
	
			GetCtrlVal(panel, OPTIONS_PATH, startFolder);
			GetCtrlVal(panel, OPTIONS_AUTOGETTAG, &autoGetTag);
			GetCtrlVal(panel, OPTIONS_AUTOREVERT, &autoRevert);
			GetCtrlVal(panel, OPTIONS_EXTENSIONS, &extensions);
			GetCtrlVal(panel, OPTIONS_SHOWLENGTH, &showLength);
			GetCtrlVal(panel, OPTIONS_SKIPALBUMARTIST, &skipAlbArtist);
			GetCtrlVal(panel, OPTIONS_IGNOREDISC1OF1, &ignoreDisc);
			GetCtrlVal(panel, OPTIONS_POPULATEALBUMORDER, &populateAlbumOrder);
			GetCtrlVal(panel, OPTIONS_RENAMEFOLDER, &renameUsePerf);
			GetCtrlVal(panel, OPTIONS_SHOWREPLAYGAIN, &showReplayGain);
			GetCtrlVal(panel, OPTIONS_REPLACEAPOSTROPHE, &replaceUnicodeApostrophe);
			GetCtrlVal(panel, OPTIONS_FANART_APIKEY, fanartPersonalAPIKey);
			GetCtrlVal(panelHandle, PANEL_CAPITALIZE, &autoCap);
			GetCtrlVal(panelHandle, PANEL_SMARTCAPS, &smartCaps);
			GetCtrlVal(panelHandle, PANEL_DOID3V1, &saveID3v1);
			GetCtrlVal(panelHandle, PANEL_ID3V1COMMENTS, &saveV1Comments);
			GetReplacementChars();
			Ini_PutRawString(cfg, kIniGeneralOptions, "Initial Search Directory", startFolder);
			Ini_PutRawString(cfg, kIniExternalPrograms, "Album Art Downloader", albumArtDLPath);
			Ini_PutRawString(cfg, kIniExternalPrograms, "Artist Logo Directory", artistLogoPath);
			Ini_PutRawString(cfg, kIniGeneralOptions, "Fanart Personal API Key", fanartPersonalAPIKey);

			Ini_PutInt(cfg, kIniGeneralOptions, "AutoCapitalize", autoCap);			
			Ini_PutInt(cfg, kIniGeneralOptions, "UseSmartCaps", smartCaps);
			Ini_PutInt(cfg, kIniGeneralOptions, "SaveID3v1Tags", saveID3v1);
			Ini_PutInt(cfg, kIniGeneralOptions, "SaveCommentsInID3v1", saveV1Comments);
			Ini_PutInt(cfg, kIniGeneralOptions, "AutoGetTag", autoGetTag);			
			Ini_PutInt(cfg, kIniGeneralOptions, "AutoRevert", autoRevert);
			Ini_PutInt(cfg, kIniGeneralOptions, "LowerCaseExtensions", extensions);
			Ini_PutInt(cfg, kIniGeneralOptions, "ShowFileNameLength", showLength);
			Ini_PutInt(cfg, kIniGeneralOptions, "IgnoreDiscNumIfEquals1of1", ignoreDisc);
			Ini_PutInt(cfg, kIniGeneralOptions, "SkipAlbumArtistIfEqualArtist", skipAlbArtist);
			Ini_PutInt(cfg, kIniGeneralOptions, "PopulateAlbumOrderWithYear", populateAlbumOrder);
			Ini_PutInt(cfg, kIniGeneralOptions, "RenameUsesPerfSortOrder", renameUsePerf);
			Ini_PutInt(cfg, kIniGeneralOptions, "ShowReplayGain", showReplayGain);
			Ini_PutInt(cfg, kIniGeneralOptions, "ReplaceUnicodeApostrophe", replaceUnicodeApostrophe);

			saveChar[0] = replaceChar[0];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceBackslash", saveChar); 
			saveChar[0] = replaceChar[1];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceForewardslash", saveChar);
			saveChar[0] = replaceChar[2];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceColon", saveChar);
			saveChar[0] = replaceChar[3];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceAsterix", saveChar);
			saveChar[0] = replaceChar[4];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceQuestion", saveChar);
			saveChar[0] = replaceChar[5];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceQuote", saveChar);
			saveChar[0] = replaceChar[6];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceLessThan", saveChar);
			saveChar[0] = replaceChar[7];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplaceGreaterThan", saveChar);
			saveChar[0] = replaceChar[8];
			Ini_PutString(cfg, kIniReplaceCharSection, "ReplacePipe", saveChar);
			
			Ini_WriteToFile(cfg, kCfgFileName);

			HidePanel(panel);
			break;
		}
	
	if (cfg)
		Ini_Dispose(cfg);
	return 0;
}

/*********************************************/

void GetReplacementChars(void)
{
	char buf[5];
	int  i, count, ctrlArray;
	
	ctrlArray = GetCtrlArrayFromResourceID(configHandle, CHARACTER_ARRAY);
	GetNumCtrlArrayItems(ctrlArray, &count);
	for (i=0;i<count;i++) {
		GetCtrlVal(configHandle, GetCtrlArrayItem(ctrlArray, i), buf);
		if (strlen(buf)>2) 
			replaceChar[i] = buf[1];
		else
			replaceChar[i] = 0;
		}
}

void SetReplacementChars(void)
{
	int i, count, ctrlArray;
	char buf[4] = "' '";
	
	ctrlArray = GetCtrlArrayFromResourceID(configHandle, CHARACTER_ARRAY);
	GetNumCtrlArrayItems(ctrlArray, &count);
	for (i=0; i<count; i++)
		if (replaceChar[i]) {
			sprintf(buf, "'%c'",(char)replaceChar[i]);
			SetCtrlVal(configHandle, GetCtrlArrayItem(ctrlArray, i), buf);
			}
		
}

/*********************************************/

int CVICALLBACK InitialDirCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char path[MAX_PATHNAME_LEN];
	
	switch (event)
	{
		case EVENT_COMMIT:
			DirSelectPopupEx(startFolder, "Initial Directory", path);
			SetCtrlVal(panel, OPTIONS_PATH, path);
			break;
	}
	return 0;
}

int CVICALLBACK StartDirCancelCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			ReadConfigOptions(TRUE);
			HidePanel(panel);
			break;
		}
	return 0;
}

int CVICALLBACK HDLogoDirCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			DirSelectPopupEx(artistLogoPath, "Artist Logo Directory", artistLogoPath);
			SetCtrlVal(panel, OPTIONS_HDLOGOPATH, artistLogoPath);
			if (!strcmp(artistLogoPath, "")) {
				SetCtrlVal(panel, OPTIONS_HDLOGOPATH, "");
			} else {
				SetCtrlVal(panel, OPTIONS_HDLOGOPATH, artistLogoPath);
				SetCtrlAttribute(panelHandle, PANEL_FANART, ATTR_DIMMED, 0);
			}
			break;
	}
	return 0;
}

int CVICALLBACK AlbumArtDownloaderSearchCB (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int status;
	char path[MAX_PATHNAME_LEN * 2];
	switch (event) {
		case EVENT_COMMIT:
			status = FileSelectPopupEx(albumArtDLPath, "AlbumArt.exe", "*.exe", "Find Album Art Downloader.exe", 
				VAL_LOAD_BUTTON, 0, 1, albumArtDLPath);
			if (status == 1) {
				SetCtrlVal(panel, OPTIONS_ALBUMARTPATH, albumArtDLPath);
				SetCtrlAttribute(panelHandle, PANEL_DLARTWORKBUTTON, ATTR_DIMMED, 0);
				}
			else {
				GetCtrlVal(panel, OPTIONS_ALBUMARTPATH, path);
				if (!strcmp(path, "")) {
					SetCtrlVal(panel, OPTIONS_ALBUMARTPATH, "");
					SetCtrlAttribute(panelHandle, PANEL_DLARTWORKBUTTON, ATTR_DIMMED, 1);
					}
				}
			break;
		}
	return 0;
}

int CVICALLBACK OptionsPanelCB (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_KEYPRESS:
			if (eventData1 != VAL_ESC_VKEY)
				break;
			/* fall through on escape */
		case EVENT_CLOSE:
			ReadConfigOptions(TRUE);
			HidePanel(panel);
			break;
	}
	return 0;
}
