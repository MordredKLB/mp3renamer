#ifndef ID3V2_HEADER
#define ID3V2_HEADER

#include "config.h"

/*** Defines ***/

#define kExperimentalBit		1 << 5
#define kExtendedHeaderBit		1 << 6
#define kUnsyncronizationBit	1 << 7
#define kFooterBit				1 << 8

#define DEFAULT_PAD_SIZE		2048
#define MAJOR_REVISION			3
#define MINOR_REVISION			0

#define kMaxGenreEntryLength	128
#define kNumWinampGenres		148
#define kNoGenre				255

#define kBlankStr				"<blank line>"
#define kPictureSizeStr			"Image Size %d x %d"

enum {
	kFileMP3 = 1,
	kFileFLAC,
	kFileAC3,
	kFileDTS,
	kFileWAV,
	kFileAAC,
	kFileAPE,
	kFileWV,	/* WavPack */
	kFileWMA
};

struct {
/* When new data fields are added to this struct, changes must be made in the following functions: 
   GetTextData (or equivalent), initID3DataStruct, ClearID3DataStruct and SetConflictTooltips.
   Also add a new enum in the proper order to the enum below. */
	char **artistPtr;
	char **albumPtr;
	char **trackNumPtr;
	char **genrePtr;
	char **commentPtr;
	char **yearPtr;
	char **discPtr;
	char **discSubtitlePtr;
	char **composerPtr;
	char **copyrightPtr;
	char **artistFilterPtr;
	char **urlPtr;
	char **encodedPtr;
	char **countryPtr;
	char **relTypePtr;
	char **origArtistPtr;
	char **albumArtistPtr;
	char **publisherPtr;
	char **editionPtr;
	char **albumGainPtr;
	char **albSortOrderPtr;
	char **perfSortOrderPtr;
} dataHandle;

enum {
	kArtist = 1,
	kAlbum,
	kGenre,
	kComment,
	kYear,
	kDisc,
	kDiscSubtitle,
	kPublisher,
	kEdition,
	kArtistCountry,
	kRelType,
	kAlbumArtist,
	kArtistFilter,
	kPerformerSortOrder,
	kAlbumGain,
	kAlbumSortOrder,
	kOrigArtist,
	kURL,
	kCopyright,
	kEncoded,
	kComposer,
	kEndOfList = kComposer
};

typedef struct defFileStruct {
	char origName[MAX_PATHNAME_LEN + MAX_FILENAME_LEN];
	char newName[MAX_PATHNAME_LEN + MAX_FILENAME_LEN];
	char origFileName[MAX_FILENAME_LEN];
} mainFileStruct;

mainFileStruct fileStruct[kMaxFiles];

typedef struct {
	char	diacritic;
	char	replacement;
} diacriticVal, *diacriticValPtr;

typedef struct { 
	char code[3];
	char name[50];
} countryEntry;

/*** Prototypes ***/

int  GetID3v2Tag(int panel, char *filename, int index, int filetype);
int  SetID3v2Tag(int panel, char *filename, char *newname, int index, int filetype);
void SetConflictTooltips(int panel);
void PopulateGenreComboBox(int panel, int control);
void DoSmartCaps(char *fileName);
void RemoveSpecifiedString(char *filename, char *string);
int	 IsItemChecked(int itemNum);
void StoreDataVals(int panel, int control, char *string, int index);

char* stristr(const char *String, const char *Pattern);

/* mp3renamer.c */
char* FindTrackNum(char *filename, int index, int *numLen, char *trackNum); 
char* FindSongTitle(char *filename, int index, int *numLen);
char* GetCtrlStrVal(int panel, int control);
char* SkipToTrackNameStart(char *name);
int   GetAudioFileType(char *filename);

#endif /* ID3V2_HEADER */
