/* General Configuration and Ini File functions for MP3Renamer */

#ifndef MP3CONFIG_H
#define MP3CONFIG_H

/***************************/
/*** Function Prototypes ***/
/***************************/
void ClearID3Fields(void);
void GetReplacementChars(void);
void SetReplacementChars(void);
void SetToolTipInfo(void);
void InitUIAttrs(void);
void ReplaceAmpersands(char *str);
void ReplaceDiacritics(char *str);
void ReplaceCommonWords(char *str);
void ReplaceUnicodeApostrophe(char *str);
void RemoveAlbumEditions(char *str);
void ReadConfigOptions(int restrictedRead);
void CenterPopupPanel(int parent, int child);
void SetCountryName(char countryCode[3]);

/* from fanart.c */
void DeleteOldXMLandJSONFiles (int removeAll);
void DownloadFileIfNotExists(char *url, char *filename);

char *stristr(const char *String, const char *Pattern);

/***************************/
/*** Globals             ***/
/***************************/
char 	startFolder[MAX_PATHNAME_LEN*2];
char 	albumArtDLPath[MAX_PATHNAME_LEN*2];
char	artistLogoPath[MAX_PATHNAME_LEN*2];
char    fanartPersonalAPIKey[33];
int 	configHandle;
int 	panelHandle;
int		tab1Handle;
int		tab2Handle;
int		tab3Handle;
int		albumPanHandle;
int		fanartPanHandle;
int		cdartPanHandle;
int		hdlogoPanHandle;
int		hdPreviewHandle;
int		pMenuHandle;
int 	progressHandle;
int 	numFiles;

int gPanelHeight;
int gPanelWidth;
int gSplitterGap;
int gVSplitterGap;
int gPanelInitialWidth;
int gPanelBorderHeight;
int gPanelBorderWidth;

ssize_t	folderImageFileSize;

char pathName[MAX_PATHNAME_LEN];

char replaceChar[10];	// list of unuseable characters replacement values

enum {
	kTreeColFilename = 0,
	kTreeColArtistName,
	kTreeColTrackName,
	kTreeColTrackNum,
	kTreeColID,
	kTreeNumColumns				// always insert columns before this val
};

enum {
	kTrackTreeColTrackDisc = 0,
	kTrackTreeColTrackNum,
	kTrackTreeColTrackName,
	kTrackTreeColTrackLength,
	kTrackTreeColTrackArtist,
	kTrackTreeColTrackArtistFilter
};

enum {
	kUnhandledTreeColFieldName = 0,
	kUnhandledTreeColValue,
	kUnhandledTreeColOrigValue,
	kUnhandledTreeColOrigField,
};

enum {
	kAlbTreeColArtist = 0,
	kAlbTreeColAlbum,
	kAlbTreeColNumTracks,
	kAlbTreeColDate,
	kAlbTreeColCountry,
	kAlbTreeColREID,
	kAlbTreeColNumDiscs,
	kAlbTreeColASIN,
	kAlbTreeColBarcode,
	kAlbTreeColCatalog,
	kAlbTreeColLabel,
	kAlbTreeColType,
	kAlbTreeColArtistSortOrder,
	kAlbTreeColArtistID,
	kAlbTreeColRelGroupID,
	kAlbTreeNumColumns	// insert above this
};

/***************************/
/*** Defines             ***/
/***************************/
#define kCfgFileName	"MP3Renamer.ini"

#define kIniGeneralOptions		"General Options"
#define	kIniReplaceCharSection	"Replacement Chars"
#define kIniExternalPrograms	"External Programs"

#define kMaxFiles		99

#define kNumDiacritics			18

#define kTooltipLineLength		80

/* used for image ring */
#define VAL_FRONT	0	// VAL_FOLDER is used for something so VAL_FRONT == folder.jpg
#define VAL_BACK	1
#define VAL_CD		2
#define VAL_CD2		3
#define VAL_INSIDE	4
#define VAL_TRAY	5
#define VAL_THUMB	6

#endif /* MP3CONFIG_H */
