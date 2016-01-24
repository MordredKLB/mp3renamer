#include "combobox.h"
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>
#include "mp3renamer.h"
#include "ID3v2.h"
#include "toolbox.h"

#include "id3tag.h"
#include "file.h"

/*** Globals ***/

char PADDING[3] = {0,0,0};
Point tagCell = {1,1};	// combobox cell of the genre table
char gFilename[MAX_FILENAME_LEN];	// needed to get filename from file.c


/*** Prototypes ***/

int GetTitleData(id3_Tag *id3tag, char *frameType, int panel, int control, int index);
int GetTextData(id3_Tag *id3tag, char *frameType, int panel, int control, int conflict, int index);
int GetCommentsData(id3_Tag *id3tag, char *frameType, int panel, int control, int conflict, int index);
int GetTextInformation(id3_Tag *id3tag, char *descString, int panel, int control, int conflict, int index);
int GetGenreData(id3_Tag *id3tag, int panel, int control, int conflict, int index);
int GetPictureData(id3_Tag *id3tag, char *frameType, int panel, int control, int conflict, int index);
int GetExtendedFields(id3_Tag *id3tag, int index);

int SetTextData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int index);
int SetCommentsData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int index);
int SetGenreData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int index);
int SetTitleData(id3_Tag *id3tag, char *frameType, int panel, int updateCtrl, char *filename, int index);
int SetTrackData(id3_Tag *id3tag, char *frameType, int panel, char *filename, int index);
int SetPictureData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int clearCtrl);
int SetExtendedFields(id3_Tag *id3tag, int index);
int SetTextInformation(id3_Tag *id3tag, char *descString, int panel, int control, int updatePanel, int updateCtrl, int index);
int RemoveTextInformation(id3_Tag *id3tag, char *textDescription);

int RemoveFrame(id3_Tag *id3tag, char *frameType);
id3frame* NewFrame(id3_Tag *id3tag, char *frameType);
void GetFileName(char *buf);

int isHandledFrameType(char *id);

int CVICALLBACK StringCompare(void *item1, void *item2);
int CheckForDuplicates(char **stringVals, int nStrings);
void AllocAndCopyStr(char **string, char *val);

/*** Code ***/

extern int panelHandle;
extern int tab1Handle;
extern int tab2Handle;
extern int tab3Handle;
extern int numFiles;
extern int firstFile;

int GetID3v2Tag(int panel, char *filename, int index)
{
	int useTPE2;
	int version, foundYear;

	void *id3file = NULL;
	id3_Tag *id3tag = NULL;
	
	id3file = id3_file_open(filename, ID3_FILE_MODE_READONLY);
	id3tag = id3_file_tag(id3file);
	version = id3_tag_version(id3tag);
	GetTitleData(id3tag,"TIT2", panel, PANEL_TREE, index);
	GetTextData(id3tag, "TALB", panel, PANEL_ALBUM, PANEL_ALBUMLED, index);
	GetTextData(id3tag, "TPE1", panel, PANEL_ARTIST, PANEL_ARTISTLED, index);
	GetTextData(id3tag, "TRCK", panel, PANEL_TRACKNUM, PANEL_TRACKNUMLED, index);
	GetTextData(id3tag, "TENC", tab1Handle, TAB1_ENCODED, TAB1_ENCODEDLED, index);
	GetTextData(id3tag, "TCOP", tab2Handle, TAB2_COPYRIGHT, TAB2_COPYRIGHTLED, index);
	GetTextData(id3tag, "TCOM", tab1Handle, TAB1_COMPOSER, TAB1_COMPOSERLED, index);
	GetTextData(id3tag, "TPOS", tab1Handle, TAB1_DISCNUM, TAB1_DISCNUMLED, index);
	GetTextData(id3tag, "TPUB", tab1Handle, TAB1_PUBLISHER, TAB1_PUBLISHERLED, index);
	GetTextData(id3tag, "TOPE", tab2Handle, TAB2_ORIGARTIST, TAB2_ORIGARTISTLED, index);
	GetTextData(id3tag, "WXXX", tab2Handle, TAB2_URL, TAB2_URLLED, index);
	GetTextData(id3tag, "TSOP", tab1Handle, TAB1_PERFORMERSORTORDER, TAB1_PERFSORTLED, index);
	GetTextData(id3tag, "TSOA", tab1Handle, TAB1_ALBUMSORTORDER, TAB1_ALBUMSORTLED, index);
	GetTextData(id3tag, "TPE3", tab1Handle, TAB1_ARTISTFILTER, TAB1_ARTISTFILTERLED, index); // overloading the TPE3 (Conductor field) to handle my ArtistFilter tag
	foundYear = GetTextData(id3tag, "TYER", tab1Handle, TAB1_YEAR, TAB1_YEARLED, index);
	if (!foundYear)		// only need to get TDRC if TYER not found I think
		GetTextData(id3tag, "TDRC", tab1Handle, TAB1_YEAR, TAB1_YEARLED, index);
	
	GetCommentsData(id3tag, "COMM", tab1Handle, TAB1_COMMENT, TAB1_COMMENTLED, index); 
	GetGenreData(id3tag, tab1Handle, TAB1_GENRE, TAB1_GENRELED, index);
	GetTextInformation(id3tag, "replaygain_album_gain", tab1Handle, TAB1_ALBUMGAIN, TAB1_ALBUMGAINLED, index);
	GetTextInformation(id3tag, "RELEASETYPE", tab1Handle, TAB1_RELTYPE, TAB1_RELTYPELED, index);
	GetTextInformation(id3tag, "MUSICBRAINZ_RELEASEGROUPID", tab3Handle, TAB3_REID, 0, index);
	GetTextInformation(id3tag, "MUSICBRAINZ_ARTISTID", tab3Handle, TAB3_ARTISTMBID, 0, index);
	GetTextInformation(id3tag, "ARTISTCOUNTRY", tab1Handle, TAB1_COUNTRY, TAB1_COUNTRYLED, index);
	GetCtrlVal(panel, PANEL_USEWINAMPALBUMARTIST, &useTPE2);
	if (useTPE2)
		GetTextData(id3tag, "TPE2", tab1Handle, TAB1_ALBUMARTIST, TAB1_ALBUMARTISTLED, index);				  	// winamp style
	else
		GetTextInformation(id3tag, "ALBUM ARTIST", tab1Handle, TAB1_ALBUMARTIST, TAB1_ALBUMARTISTLED, index);// foobar style
	//GetTextInformation(id3tag, index);
	GetPictureData(id3tag, "APIC", tab2Handle, TAB2_ARTWORK, TAB2_ARTWORKLED, index);
	GetExtendedFields(id3tag, index);

Error:
	if (id3file)
		id3_file_close(id3file);
	return 0;
}

int GetTitleData(id3_Tag *id3tag, char *frameType, int panel, int control, int index)
{
	int			nStrings, found = 1;
	char 		*data = NULL, *string = NULL;
	id3frame 	*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;
	
	frame = id3_tag_findframe(id3tag, frameType, 0);
	if (!frame)
		return 0;
	if (frame->nfields < 2)
		Breakpoint();
	field = id3_frame_field(frame, frame->nfields-1);	// I think we always want the last field in the frame
	if (!field)
		return 0;
	switch (field->type) {
		case ID3_FIELD_TYPE_STRINGLIST:
			nStrings = id3_field_getnstrings(field);
			if (!nStrings)
				return 0;
			ucs4Str = id3_field_getstrings(field, 0);
			string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
			break;
		case ID3_FIELD_TYPE_LATIN1:
			data = (char *)id3_field_getlatin1(field);
			string = malloc(sizeof(char) * strlen(data)+1);
			strcpy(string, data);
			data = NULL;
			break;
		}
	
	SetTreeCellAttribute(panel, control, index, kTreeColTrackName, ATTR_LABEL_TEXT, string);

Error:
	if (data)
		free(data);
	if (string)
		free(string);
	return found;
}

int GetTextData(id3_Tag *id3tag, char *frameType, int panel, int control, int conflict, int index)
{
	int			i, len, error, nStrings, found = 0;
	size_t		size;
	char 		*data = NULL, *origData = NULL, *string = NULL, *tmpStr = NULL;
	id3frame 	*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;
	
	frame = id3_tag_findframe(id3tag, frameType, 0);
	if (!frame)
		return 0;
	if (frame->nfields < 2)
		Breakpoint();
	field = id3_frame_field(frame, frame->nfields-1);	// I think we always want the last field in the frame
	if (!field)
		return 0;
	switch (field->type) {
		case ID3_FIELD_TYPE_STRINGLIST:
			nStrings = id3_field_getnstrings(field);
			if (!nStrings)
				return 0;
			for (i=0;i<nStrings;i++) {
				ucs4Str = id3_field_getstrings(field, i);
				tmpStr = (char *)id3_ucs4_latin1duplicate(ucs4Str);
				if (i==0) {
					string = malloc(sizeof(char) * (strlen(tmpStr) + 3));
					strcpy(string, tmpStr);
				}
				else {
					string = realloc(string, sizeof(char) * (strlen(tmpStr) + strlen(string) + 3));
					strcat(string, "; \0");
					strcat(string, tmpStr);
				}
				free(tmpStr);
				tmpStr = NULL;
			}
			break;
		case ID3_FIELD_TYPE_LATIN1:
			data = (char *)id3_field_getlatin1(field);
			string = malloc(sizeof(char) * strlen(data)+1);
			strcpy(string, data);
			data = NULL;
			break;
		case ID3_FIELD_TYPE_STRING: { /* should be description field */
				ucs4Str = id3_field_getstring(field);
				string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
				len = strlen(string);
				free(string);
				string = NULL;
				}
			break;
		}

	GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
	if (len) {
		nullChk(origData = malloc(len+1));
		GetCtrlVal(panel, control, origData); 
		}
	if (!firstFile && (len!=strlen(string) || memcmp(origData, string, len)))
		SetCtrlVal(panel, conflict, 1);
	else
		errChk(SetCtrlVal(panel, control, string));

	size = strlen(string)+1;
	if (panel == panelHandle) {
		switch (control) {
			case PANEL_ARTIST:
				dataHandle.artistPtr[index] = calloc(size + 2, sizeof(char));	// we need a fudge factor for TPE2 field
				strcpy(dataHandle.artistPtr[index], string);
				break;
			case PANEL_ALBUM:
				dataHandle.albumPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.albumPtr[index], string);
				break;
			case PANEL_TRACKNUM:	/* hidden behind the tree control */
				dataHandle.trackNumPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.trackNumPtr[index], string);
				break;
			}
		}
	else if (panel == tab1Handle) {
		switch (control) {
			case TAB1_COMPOSER:
				dataHandle.composerPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.composerPtr[index], string);
				break;
			case TAB1_YEAR:
				dataHandle.yearPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.yearPtr[index], string);
				break;
			case TAB1_DISCNUM:
				dataHandle.discPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.discPtr[index], string);
				gUseMetaDataDiscVal = 1;
				break;
			case TAB1_ARTISTFILTER:
				dataHandle.artistFilterPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.artistFilterPtr[index], string);
				break;
			case TAB1_PUBLISHER:
				dataHandle.publisherPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.publisherPtr[index], string);
				break;
			case TAB1_ENCODED:
				dataHandle.encodedPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.encodedPtr[index], string);
				break;
			case TAB1_COUNTRY:
				dataHandle.countryPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.countryPtr[index], string);
				break;
			case TAB1_ALBUMARTIST:
				dataHandle.albumArtistPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.albumArtistPtr[index], string);
				break;
			case TAB1_PERFORMERSORTORDER:
				dataHandle.perfSortOrderPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.perfSortOrderPtr[index], string);
				break;
			case TAB1_ALBUMSORTORDER:
				dataHandle.albSortOrderPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.albSortOrderPtr[index], string);
				break;
			}
		}
	else {
		switch (control) {
			case TAB2_COPYRIGHT:
				dataHandle.copyrightPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.copyrightPtr[index], string);
				break;
			case TAB2_ORIGARTIST:
				dataHandle.origArtistPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.origArtistPtr[index], string);
				break;
			case TAB2_URL:
				dataHandle.urlPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.urlPtr[index], string);
				break;
			}
		SetCtrlVal(panelHandle, PANEL_TABVALS, 1);	// always set this
		}
	found = 1;
	
Error:
	if (origData)
		free(origData);
	if (data)
		free(data);
	if (string)
		free(string);
	if (tmpStr)
		free(tmpStr);
	return found;
}

int GetCommentsData(id3_Tag *id3tag, char *frameType, int panel, int control, int conflict, int index)
{
	int				i=0, j, error, found=0, length;
	size_t			len=0;
	char 			*origData = NULL, *string = NULL;
 	id3frame 		*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;

	while((frame = id3_tag_findframe(id3tag, frameType, i++)) && !found) {
		if (!frame)
			return 0;
		if (frame->nfields < 2)
			Breakpoint();
		for (j=1;j<frame->nfields;j++) {
			field = id3_frame_field(frame, j);	// do we always want the 2nd field?
			if (!field)
				return 0;

			if (field->type == ID3_FIELD_TYPE_STRING) { /* should be description field */
				ucs4Str = id3_field_getstring(field);
				string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
				len = strlen(string);
				if (len > 0 && (!strncmp(string, "iTunPGAP", len) || 
					!strncmp(string, "iTunSMPB", len) || 
					!strncmp(string, "iTunNORM", len))) {
					j = frame->nfields;	// don't do anything with iTunes Comment Frames
					free(string);
					string = NULL;
					}
				}
			else if (field->type == ID3_FIELD_TYPE_STRINGFULL) {
				if (string)
					free(string);
				string = NULL;
				ucs4Str = id3_field_getfullstring(field);
				string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
				found = 1;
				break;
				}
			}

		}

	if (found) {
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &length);
		if (length) {
			origData = malloc(length+1);
			GetCtrlVal(panel, control, origData); 
			}

		if (string) {
			dataHandle.commentPtr[index] = calloc(strlen(string)+1, sizeof(char));
			strcpy(dataHandle.commentPtr[index], string);
			}
		if (len && (len!=strlen(string) || memcmp(origData, string, len)))
			SetCtrlVal(panel, conflict, 1);
		else
			errChk(ResetTextBox(panel, control, string));
		}
	
Error:
	if (origData)
		free(origData);
	if (string)
		free(string);
	return found;
}

/* Retrieve information from a TXXX field with a description == descString */
int GetTextInformation(id3_Tag *id3tag, char *descString, int panel, int control, int conflict, int index)
{
	int				len, i=0, j, error, found=0, style;
	char 			*data = NULL, *origData = NULL, *string = NULL;
 	id3frame 		*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;

	while((frame = id3_tag_findframe(id3tag, "TXXX", i++)) && !found) {
		if (!frame)
			return 0;
		if (frame->nfields < 2)
			Breakpoint();
		for (j=1;j<frame->nfields;j++) {
			field = id3_frame_field(frame, j);	// do we always want the 2nd field?
			if (!field)
				return 0;

			if (field->type == ID3_FIELD_TYPE_STRING) { /* should be description field */
				ucs4Str = id3_field_getstring(field);
				string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
				if (!stricmp(string, descString)) {
					free(string);
					string = NULL;
					field = id3_frame_field(frame, j+1);
					if (field) {
						ucs4Str = id3_field_getstring(field);
						string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
						found = 1;
						j=frame->nfields;
						}
					}
				else {
					free(string);
					string = NULL;
					j=frame->nfields;	// skip this frame
					}
				}
			}
		}

	if (found) {
		GetCtrlAttribute(panel, control, ATTR_CTRL_STYLE, &style);
		if (style == CTRL_TABLE_LS)
			GetTableCellValLength(panel, control, tagCell, &len);
		else
			GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
		if (len) {
			nullChk(origData = malloc(len+1));
			if (style == CTRL_TABLE_LS)
				GetTableCellVal(panel, control, tagCell, origData);
			else
				GetCtrlVal(panel, control, origData); 
			}
		if (!firstFile && (len!=strlen(string) || memcmp(origData, string, len)) && conflict)
			SetCtrlVal(panel, conflict, 1);
		else {
			if (style == CTRL_TABLE_LS)
				errChk(SetTableCellVal(panel, control, tagCell, string))
			else
				errChk(SetCtrlVal(panel, control, string));
		}

		switch (control) {
			case TAB1_ALBUMGAIN:
				dataHandle.albumGainPtr[index] = calloc(strlen(string)+1, sizeof(char));
				strcpy(dataHandle.albumGainPtr[index], string);
				break;
			case TAB1_ALBUMARTIST:
				dataHandle.albumArtistPtr[index] = calloc(strlen(string)+1, sizeof(char));
				strcpy(dataHandle.albumArtistPtr[index], string);
				break;
			case TAB1_RELTYPE:
				dataHandle.relTypePtr[index] = calloc(strlen(string)+1, sizeof(char));
				strcpy(dataHandle.relTypePtr[index], string);
				break;
			case TAB1_ARTISTFILTER:
				dataHandle.artistFilterPtr[index] = calloc(strlen(string)+1, sizeof(char));
				strcpy(dataHandle.artistFilterPtr[index], string);
				break;
			case TAB1_COUNTRY:
				dataHandle.countryPtr[index] = calloc(strlen(string)+1, sizeof(char));
				strcpy(dataHandle.countryPtr[index], string);
				break;
			}
		}
	
Error:
	if (data)
		free(data);
	if (origData)
		free(origData);
	if (string)
		free(string);
	return found;
}

int isHandledFrameType(char *id) {
	/* do not include TXXX here, because we only handle some cases */
	if (!strcmp("TALB", id) || !strcmp("TPE1", id) ||
		!strcmp("TRCK", id) || !strcmp("TENC", id) || 
		!strcmp("TCOP", id) || !strcmp("TCOM", id) || 
		!strcmp("TPOS", id) || !strcmp("TPUB", id) || 
		!strcmp("TOPE", id) || !strcmp("TPE2", id) || 
		!strcmp("WXXX", id) || !strcmp("TSOP", id) ||
		!strcmp("TSOA", id) || !strcmp("TYER", id) || 
		!strcmp("TDRC", id) || !strcmp("COMM", id) || 
		!strcmp("APIC", id) || !strcmp("TIT2", id) ||
		!strcmp("TCON", id))
		return 1;
	else
		return 0;
	
}

int GetExtendedFields(id3_Tag *id3tag, int index)
{
	int				i=0, numItems, found=-1, showRG, useTPE2;
	char 			*string = NULL;
 	id3frame 		*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;

	GetCtrlVal(configHandle, OPTIONS_SHOWREPLAYGAIN, &showRG);
	GetCtrlVal(panelHandle, PANEL_USEWINAMPALBUMARTIST, &useTPE2);
	while(frame = id3_tag_findframe(id3tag, NULL, i++)) {
		if (!isHandledFrameType(frame->id)) {
			if (!strcmp(frame->id, "TXXX")) {
				field = id3_frame_field(frame, 1);	// we always want to start with the 2nd field
				if (!field)
					continue;

				if (field->type == ID3_FIELD_TYPE_STRING) { // should be description field 
					ucs4Str = id3_field_getstring(field);
					string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
					
					if ((useTPE2 || stricmp(string, "ALBUM ARTIST")) &&
						(stricmp(string, "RELEASETYPE")) &&
						(stricmp(string, "MUSICBRAINZ_RELEASEGROUPID")) &&
						(stricmp(string, "MUSICBRAINZ_ARTISTID")) &&
						(stricmp(string, "ARTISTCOUNTRY")) &&
						(showRG || (stricmp(string, "replaygain_album_gain") && stricmp(string, "replaygain_track_gain") &&
								   stricmp(string, "replaygain_album_peak") && stricmp(string, "replaygain_track_peak"))) ) {
						GetNumListItems(tab3Handle, TAB3_EXTENDEDTAGS, &numItems);
						if (numItems)
							GetTreeItemFromLabel(tab3Handle, TAB3_EXTENDEDTAGS, VAL_ALL, 0, 0, VAL_NEXT_PLUS_SELF, 0, string, &found);
						if (found==-1) {
							InsertTreeItem(tab3Handle, TAB3_EXTENDEDTAGS, VAL_SIBLING, 0, VAL_LAST, string, NULL, NULL, numItems);
							SetTreeItemAttribute (tab3Handle, TAB3_EXTENDEDTAGS, numItems, ATTR_MARK_STATE, 1);
							SetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, numItems, 2, ATTR_LABEL_TEXT, "TXXX");
							free(string);
							string = NULL;
							field = id3_frame_field(frame, 2);
							if (field) {
								ucs4Str = id3_field_getstring(field);
								string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
								SetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, numItems, 1, ATTR_LABEL_TEXT, string);
								}
							SetCtrlVal(panelHandle, PANEL_TABVALS2, 1);
							}
						}
					// string should always be allocated at this point.
					free(string);
					string = NULL;
					}
				}
			}
		}

	
Error:
	if (string)
		free(string);
	return found;
}

int GetGenreData(id3_Tag *id3tag, int panel, int control, int conflict, int index)
{
	int			error, nStrings;
	char 		*genreVal = NULL, oldGenre[kMaxGenreEntryLength];
	char		*genreString = NULL;
	id3frame 	*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;
	id3_ucs4_t const *ucs4genre;
	
	frame = id3_tag_findframe(id3tag, "TCON", 0);
	if (!frame)
		return 0;
	if (frame->nfields < 2)
		Breakpoint();
	field = id3_frame_field(frame, frame->nfields-1);	// I think we always want the last field in the frame
	if (!field)
		return 0;
	if (field->type == ID3_FIELD_TYPE_STRINGLIST) {
		nStrings = id3_field_getnstrings(field);
		switch (nStrings) {
			case 0:
				return 0;
			case 1:
				ucs4Str = id3_field_getstrings(field, 0);
				ucs4genre = id3_genre_name(ucs4Str);
				genreString = (char *)id3_ucs4_latin1duplicate(ucs4genre);
				break;
			case 2:
				ucs4Str = id3_field_getstrings(field, 0);
				ucs4genre = id3_genre_name(ucs4Str);			 
				genreVal = (char *)id3_ucs4_latin1duplicate(ucs4genre);  
				ucs4Str = id3_field_getstrings(field, 1);
				genreString = (char *)id3_ucs4_latin1duplicate(ucs4Str);
				break;
			default:
				Breakpoint();
				errChk(-1)
				break;
			}
		}
	else {
		Breakpoint();
		errChk(-1);
		}

	GetTableCellVal(panel, control, tagCell, oldGenre);
	if (!firstFile && index > 0 && strcmp(oldGenre, genreString))
		SetCtrlVal(panel, conflict, 1);
	else
		SetTableCellVal(panel, control, tagCell, genreString);

	dataHandle.genrePtr[index] = calloc(strlen(genreString)+1, sizeof(char));
	strcpy(dataHandle.genrePtr[index], genreString);

Error:
	if (genreVal)
		free(genreVal);
	if (genreString)
		free(genreString);
	return 1;
}

int GetImageTypeExtension(char *type, char ext[4])
{
	int found=0;
	char errMsg[255];
	
	if (stristr(type, "png")) {
		strcpy(ext, "png\0");
		}
	else if (stristr(type, "jpeg") || stristr(type, "jpg")) {
		strcpy(ext, "jpg\0");
		}
	else if (stristr(type, "gif")) {
		// We can't handle .gif files
		//strcpy(ext, "gif\0");
		found = -1;
		}
	else {
		sprintf(errMsg, "Undefined mimeType = %s", type);
		MessagePopup("ERROR",errMsg);
		found = -1;
		}
	
	return found;
}

int GetPictureData(id3_Tag *id3tag, char *frameType, int panel, int control, int conflict, int index)
{
	int			error, pictureType;
	int			width, height, bitmap = 0;
	size_t		len;
	char 		*mimeType, imageNameStr[MAX_FILENAME_LEN], ext[4], sizeStr[100];
	char		*desc = NULL;
	char		*data = NULL;
	FILE		*fileIn = NULL;//, *fileOut = NULL;
	id3frame 	*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;
	
	frame = id3_tag_findframe(id3tag, "APIC", 0);
	if (!frame) {
		DisableBreakOnLibraryErrors();
		DeleteImage(panel, control);
		EnableBreakOnLibraryErrors();
		return 0;
		}
	
	/* APIC STRUCTURE
	FIELDS(APIC) = {
	  ID3_FIELD_TYPE_TEXTENCODING,		    // Text encoding      $xx
	  ID3_FIELD_TYPE_LATIN1,				// MIME type          <text string> $00
	  ID3_FIELD_TYPE_INT8,					// Picture type       $xx
	  ID3_FIELD_TYPE_STRING,				// Description        <text string according to encoding> $00 (00)
	  ID3_FIELD_TYPE_BINARYDATA				// Picture data       <binary data>
	}; */
	
	field =    id3_frame_field(frame, 1);	// MIME Type
	mimeType = (char *)id3_field_getlatin1(field);
	field =    id3_frame_field(frame, 2);	// Picture Type
	pictureType = field->number.value;
	field =    id3_frame_field(frame, 3);	// Description
	ucs4Str =  id3_field_getstring(field);
	desc =     (char *)id3_ucs4_latin1duplicate(ucs4Str);
	field =    id3_frame_field(frame, 4);	// Picture data
	len =      field->binary.length;
	errChk(GetImageTypeExtension(mimeType, ext));
	sprintf(imageNameStr, "imageFile%d.%s", index, ext);

	fileIn = fopen(imageNameStr,"wb+");
	fwrite(field->binary.data, 1, field->binary.length, fileIn);
	fclose(fileIn);
	DisableBreakOnLibraryErrors();
	if (DisplayImageFile(panel, control, imageNameStr)) {
		SetCtrlAttribute(panel, TAB2_IMAGECORRUPTEDMSG, ATTR_VISIBLE, 1);
		sprintf(sizeStr, kPictureSizeStr, 0, 0);
		SetCtrlVal(panel, TAB2_IMAGESIZEMSG, sizeStr);
		}
	else {
		SetCtrlAttribute(panel, TAB2_IMAGECORRUPTEDMSG, ATTR_VISIBLE, 0);
		GetCtrlBitmap(panel, control, 0, &bitmap);
		GetBitmapData(bitmap, NULL, NULL, &width, &height, NULL, NULL, NULL);
		sprintf(sizeStr, kPictureSizeStr, width, height);
		SetCtrlVal(panel, TAB2_IMAGESIZEMSG, sizeStr);
		}
	EnableBreakOnLibraryErrors();
	fileIn = fopen(imageNameStr, "rb");
	SetCtrlVal(panelHandle, PANEL_TABVALS, 1);
	SetCtrlAttribute(panel, TAB2_CLEARARTWORK, ATTR_VISIBLE, 1);
	
	//errChk(GetImageTypeExtension(mimeType, ext));

	//fileOut = fopen(imageNameStr, "wb");
	//decodeMIME(fileIn, fileOut);
	
	//data = calloc(len * 3 / 4, sizeof(char));

	
Error:
	if (desc)
		free(desc);
	if (data)
		free(data);
	if (fileIn && imageNameStr) {
		fclose(fileIn);
		DeleteFile(imageNameStr);
		}
	if (bitmap)
		DiscardBitmap(bitmap);
	return 1;
}

/**************************************************/
/*** Functions below here are Set Tag Functions ***/
/**************************************************/

int SetID3v2Tag(int panel, char *filename, char *newname, int index)
{
	int 	i, error, useTPE2, update;
	size_t	v2size;
	void 	*id3file = NULL;
	id3_Tag *id3tag = NULL;
	
	strcpy(gFilename, filename);	// needed for file.c
		
	id3file = id3_file_open(filename, ID3_FILE_MODE_READWRITE);
	id3tag = id3_file_tag(id3file);
	SetTitleData(id3tag, "TIT2", panel, PANEL_UPDATETITLE, newname, index);
	SetTrackData(id3tag, "TRCK", panel, newname, index);
	SetTextData(id3tag, "TALB", panel, PANEL_ALBUM, PANEL_UPDATEALBUM, index);
	SetTextData(id3tag, "TPE1", panel, PANEL_ARTIST, PANEL_UPDATEARTIST, index);
	SetTextData(id3tag, "TENC", tab1Handle, TAB1_ENCODED, TAB1_UPDATEENCODED, index);
	//SetTextData(id3tag, "TCOP", tab1Handle, TAB1_COPYRIGHT, TAB1_UPDATECOPYRIGHT, index);
	SetTextData(id3tag, "TCOP", tab2Handle, TAB2_COPYRIGHT, TAB2_UPDATECOPYRIGHT, index);
	SetTextData(id3tag, "TCOM", tab1Handle, TAB1_COMPOSER, TAB1_UPDATECOMPOSER, index);
	GetCtrlVal(tab1Handle, TAB1_UPDATEDISCNUM, &update);
	if (update)
		RemoveFrame(id3tag, "TPOS");
	SetTextData(id3tag, "TPOS", tab1Handle, TAB1_DISCNUM, TAB1_UPDATEDISCNUM, index);
	SetTextData(id3tag, "TPUB", tab1Handle, TAB1_PUBLISHER, TAB1_UPDATEPUBLISHER, index);
	SetTextData(id3tag, "TOPE", tab2Handle, TAB2_ORIGARTIST, TAB2_UPDATEORIGARTIST, index);
	SetTextData(id3tag, "WXXX", tab2Handle, TAB2_URL, TAB2_UPDATEURL, index);
	//SetTextData(id3tag, "TYER", tab1Handle, TAB1_YEAR, TAB1_UPDATEYEAR, index);
	RemoveFrame(id3tag, "TYER");	// supposedly this frame is not used in v2.4
	RemoveFrame(id3tag, "TDRC");	// force it to only show what is in Year
	SetTextData(id3tag, "TDRC", tab1Handle, TAB1_YEAR, TAB1_UPDATEYEAR, index);
	SetTextData(id3tag, "TSOP", tab1Handle, TAB1_PERFORMERSORTORDER, TAB1_UPDATEPERFSORT, index);
	
	SetCommentsData(id3tag, "COMM", tab1Handle, TAB1_COMMENT, TAB1_UPDATECOMMENT, index);
	SetGenreData(id3tag, "TCON", tab1Handle, TAB1_GENRE, TAB1_UPDATEGENRE, index);
	SetPictureData(id3tag,"APIC",tab2Handle, TAB2_ARTWORK, TAB2_UPDATEARTWORK, TAB2_CLEARARTWORK);
	SetTextData(id3tag, "TSOA", tab1Handle, TAB1_ALBUMSORTORDER, TAB1_UPDATEALBUMSORT, index);
	SetTextData(id3tag, "TPE3", tab1Handle, TAB1_ARTISTFILTER, TAB1_UPDATEARTISTFILTER, index); // overloading the TPE3 (Conductor field) to handle my ArtistFilter tag


	SetExtendedFields(id3tag, index);
	
	GetCtrlVal(panel, PANEL_USEWINAMPALBUMARTIST, &useTPE2);
	if (useTPE2)
		SetTextData(id3tag, "TPE2", tab1Handle, TAB1_ALBUMARTIST, TAB1_UPDATEALBUMARTIST, index);
	else
		SetTextInformation(id3tag, "ALBUM ARTIST", tab1Handle, TAB1_ALBUMARTIST, tab1Handle, TAB1_UPDATEALBUMARTIST, index);
	SetTextInformation(id3tag, "RELEASETYPE", tab1Handle, TAB1_RELTYPE, tab1Handle, TAB1_UPDATERELTYPE, index);
	SetTextInformation(id3tag, "MUSICBRAINZ_ARTISTID", tab3Handle, TAB3_ARTISTMBID, tab3Handle, TAB3_UPDATEMBID, index);
	SetTextInformation(id3tag, "MUSICBRAINZ_RELEASEGROUPID", tab3Handle, TAB3_REID, tab3Handle, TAB3_UPDATEREID, index);
	SetTextInformation(id3tag, "ARTISTCOUNTRY", tab1Handle, TAB1_COUNTRY, tab1Handle, TAB1_UPDATECOUNTRY, index);
	
	
	i = id3tag->options;
	id3tag->options &= ~ID3_TAG_OPTION_ID3V1;		// clear flag to get size of id3v2
	id3tag->options |= ID3_TAG_OPTION_NOPADDING;	// calculate size without padding
	v2size = id3_tag_render(id3tag, 0);
	id3tag->options = i;
	if (v2size > id3tag->paddedsize || (v2size + (DEFAULT_PAD_SIZE*2) < id3tag->paddedsize))
		id3_tag_setlength(id3tag, v2size + DEFAULT_PAD_SIZE);
	error = id3_file_update(id3file);
	
Error:
	if (id3file)
		id3_file_close(id3file);

	return error;
}

void GetFileName(char *buf)
{
	strcpy(buf, gFilename);
}

#define kMaxMultipleValues	10

int SetTextData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int index)
{
	int 			error, len, i, update, nStrings, type=0;
	char 			*data=NULL, *strVal=NULL, *tmpStr = NULL, *ptr, *start;
	id3frame 		*frame;
	union id3_field	*field;
	id3_ucs4_t 		*ucs4Str[kMaxMultipleValues] = {0};	// max of 10 values?
	unsigned char	*stringVals[kMaxMultipleValues] = {0};

	for (i=0;i<kMaxMultipleValues;i++) {
		ucs4Str[i] = NULL;
		stringVals[i] = NULL;
	}
	errChk(GetCtrlVal(panel, updateCtrl, &update));
	if (!update) 
		goto Error;		
	
	if (!strcmp(frameType, "TPOS") && gUseMetaDataDiscVal && dataHandle.discPtr[index]) {	// disc num
		if (!strcmp(dataHandle.discPtr[index], "1/1"))
			goto Error;	// don't save "1/1" strings
		len = strlen(dataHandle.discPtr[index]);
		nullChk(data = calloc(len+3, sizeof(char)));
		sprintf(data, "%s\0", dataHandle.discPtr[index]);
	} else if (!stricmp(frameType, "TPE3")) {			// If we're setting the custom "Artist Filter" check to see if Album Artist is VA/Soundtrack and add that, along with Artist
		GetCtrlAttribute(tab1Handle, TAB1_ALBUMARTIST, ATTR_STRING_TEXT_LENGTH, &len);
		nullChk(data = calloc(len+1, sizeof(char)));
		GetCtrlVal(tab1Handle, TAB1_ALBUMARTIST, data);
		if (stristr(data, "Various Artists"))	// album artist
			type = 1;
		else if (stristr(data, "Soundtrack"))
			type = 2;
		free(data);
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
		nullChk(data = calloc(len+3, sizeof(char)));
		nullChk(tmpStr = calloc(len+3, sizeof(char)));
		errChk(GetCtrlVal(panel, control, data));
		if (type > 0)
			len++;	// we have to do something here whether anything was entered in "Artist Filter" field or not
	} else {		// handle anything but disc num or Artist Filter
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
		nullChk(data = calloc(len+3, sizeof(char)));
		nullChk(tmpStr = calloc(len+3, sizeof(char)));
		errChk(GetCtrlVal(panel, control, data));
	}
	
	frame = id3_tag_findframe(id3tag, frameType, 0);
	if (len) {	// add/update tag
		if (!frame) {
			frame = NewFrame(id3tag, frameType);
			if (stristr(frameType, "WXXX")) {
				//id3_field_set
				}
			}
		/* update existing frame */
		if (frame->nfields < 2)
			Breakpoint();
		field = id3_frame_field(frame, frame->nfields-1);	// I think we always want the last field in the frame
		if (!field)
			return 0;
		switch (field->type) {
			case ID3_FIELD_TYPE_STRINGLIST:
				if (type > 0) {
					nStrings = 0;
					GetCtrlVal(panelHandle, PANEL_UPDATEARTIST, &update);
					if (update) {
						GetCtrlAttribute(tab1Handle, TAB1_PERFORMERSORTORDER, ATTR_STRING_TEXT_LENGTH, &len);
						if (len > 0) {
							nullChk(strVal = calloc(len+2, sizeof(char)));
							errChk(GetCtrlVal(tab1Handle, TAB1_PERFORMERSORTORDER, strVal));
						} else {		
							GetCtrlAttribute(panelHandle, PANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &len);
							nullChk(strVal = calloc(len+2, sizeof(char)));
							errChk(GetCtrlVal(panelHandle, PANEL_ARTIST, strVal));
						}
						ptr = strVal;
					}
					else {
						ptr = dataHandle.artistPtr[index];
					}
					if (!strncmp("The ", ptr, 4) || !strncmp("the ", ptr, 4)){
						memmove(ptr, ptr+4, strlen(ptr)-3);	// include NULL
						strcat(ptr, ", The");
					}
					AllocAndCopyStr(&stringVals[nStrings++], ptr);
					/* get values already in Artist Filter */
					if (strlen(data) > 0) {	// if Artist Filter was empty at start don't attempt to add names
						start = ptr = data;
						while (ptr = strchr(start, ';')) {
							strncpy(tmpStr, start, ptr-start);
							tmpStr[ptr-start] = '\0';
							AllocAndCopyStr(&stringVals[nStrings++], tmpStr);
							start = ptr+2;	// skip the "; "
							}
						AllocAndCopyStr(&stringVals[nStrings++], start);
					}

					/* add in VA/Soundtrack string to the very end */
					if (type == 1)
						AllocAndCopyStr(&stringVals[nStrings++], "Various Artists");
					else
						AllocAndCopyStr(&stringVals[nStrings++], "Soundtrack");
				}
				else {
					nStrings = 0;
					start = ptr = data;
					while (ptr = strchr(start, ';')) {
						strncpy(tmpStr, start, ptr-start);
						tmpStr[ptr-start] = '\0';
						AllocAndCopyStr(&stringVals[nStrings++], tmpStr);
						start = ptr+2;	// skip the "; "
						}
					AllocAndCopyStr(&stringVals[nStrings++], start);
				}
				nStrings = CheckForDuplicates(stringVals, nStrings);
				for (i=0;i<nStrings;i++)
					ucs4Str[i] = id3_latin1_ucs4duplicate(stringVals[i]);
				id3_field_setstrings(field, nStrings, ucs4Str);
				break;
			case ID3_FIELD_TYPE_LATIN1:
				id3_field_setlatin1(field, (id3_latin1_t *)data);
				break;
			}
		}
	else if (frame) {
		id3_tag_detachframe(id3tag, frame);
		id3_frame_delete(frame);
		}
	
	
Error:
	for (i=0;i<kMaxMultipleValues;i++) {
		if (ucs4Str[i])
			free(ucs4Str[i]);
		if (stringVals[i])
			free(stringVals[i]);
	}
	if (data)
		free(data);
	if (tmpStr)
		free(tmpStr);
	if (strVal)
		free(strVal);
	
	return error;
}

int SetTitleData(id3_Tag *id3tag, char *frameType, int panel, int updateCtrl, char *filename, int index)
{
	int 			error, len, update, nStrings;
	char 			*data=NULL, *ptr = NULL, *title = NULL;
	id3frame 		*frame = NULL;
	union id3_field	*field;
	id3_ucs4_t 		*ucs4Str = NULL;

	errChk(GetCtrlVal(panel, updateCtrl, &update));
	if (!update) 
		return 0;		

	GetTreeCellAttribute (panelHandle, PANEL_TREE, index, kTreeColTrackName, ATTR_LABEL_TEXT_LENGTH, &len);
	if (len) {
		title = malloc(sizeof(char) * (len + 1));
		GetTreeCellAttribute (panelHandle, PANEL_TREE, index, kTreeColTrackName, ATTR_LABEL_TEXT, title);
		ptr = title;
		}
	else
		ptr = FindSongTitle(filename, index, &len);
	if (ptr) {
		nullChk(data = calloc(len+1, sizeof(char)));
		strncpy(data, ptr, len);

		frame = id3_tag_findframe(id3tag, frameType, 0);
		if (!frame) {
			frame = NewFrame(id3tag, frameType);
			}
		/* update existing frame */
		field = id3_frame_field(frame, frame->nfields-1);	// I think we always want the last field in the frame
		if (field->type) {
			nStrings = id3_field_getnstrings(field);
			ucs4Str = id3_latin1_ucs4duplicate((id3_latin1_t *)data);
			id3_field_setstrings(field, 1, &ucs4Str);	// always add/store in first string
			}
		}
	
Error:
	if (title)
		free(title);
	if (data)
		free(data);
	if (ucs4Str)
		free(ucs4Str);
	return 1;
}

int SetTrackData(id3_Tag *id3tag, char *frameType, int panel, char *filename, int index)
{
	int 			error, len, nStrings;
	char 			*data=NULL, *ptr = NULL, trackStr[5];
	id3frame 		*frame = NULL;
	union id3_field	*field;
	id3_ucs4_t 		*ucs4Str = NULL;

	GetTreeCellAttribute (panelHandle, PANEL_TREE, index, kTreeColTrackNum, ATTR_LABEL_TEXT, trackStr);
	if (!strcmp(trackStr, ""))
		ptr = FindTrackNum(filename, index, &len, trackStr);
	else {
		ptr = trackStr;
		len = (int)strlen(trackStr);
		}
	
	if (ptr) {
		nullChk(data = calloc(len+1, sizeof(char)));
		strncpy(data, trackStr, len);

		frame = id3_tag_findframe(id3tag, frameType, 0);
		if (!frame)
			frame = NewFrame(id3tag, frameType);
		/* update existing frame */
		field = id3_frame_field(frame, frame->nfields-1);	// I think we always want the last field in the frame
		if (field->type) {
			nStrings = id3_field_getnstrings(field);
			ucs4Str = id3_latin1_ucs4duplicate((id3_latin1_t *)data);
			id3_field_setstrings(field, 1, &ucs4Str);	// always add/store in first string
			}
		}
	else
		return 0;
	
Error:
	if (data)
		free(data);
	if (ucs4Str)
		free(ucs4Str);
	return 1;
}
	
int SetPictureData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int clearCtrl)
{
	int 			error, update, clear, bitmap=0;
//	id3frame 		*frame = NULL;
//	union id3_field	*field;
	
	errChk(GetCtrlVal(panel, updateCtrl, &update));
	errChk(GetCtrlVal(panel, clearCtrl, &clear));
	
	if (update) {
		if (clear)
			RemoveFrame(id3tag, frameType);
#if 0
		RemoveFrame(id3tag, frameType);		// always remove artwork
		if (!clear) {
			GetCtrlBitmap(tab2Handle, TAB2_ARTWORK, 0, &bitmap);
			if (!bitmap)
				errChk(-1);
			//SaveBitmapToJPEGFile(bitmap, "temporaryJPGFile.jpg",  
			frame = NewFrame(id3tag, frameType);
	/* APIC STRUCTURE
		FIELDS(APIC) = {
		  ID3_FIELD_TYPE_TEXTENCODING,		    // Text encoding      $xx
		  ID3_FIELD_TYPE_LATIN1,				// MIME type          <text string> $00
		  ID3_FIELD_TYPE_INT8,					// Picture type       $xx
		  ID3_FIELD_TYPE_STRING,				// Description        <text string according to encoding> $00 (00)
		  ID3_FIELD_TYPE_BINARYDATA				// Picture data       <binary data>
		}; */
			
			field = id3_frame_field(frame, 1);	// MIME Type
			field = id3_frame_field(frame, 2);	// Picture Type
			field = id3_frame_field(frame, 3);	// Description
			field = id3_frame_field(frame, 4);	// Picture data
			}
#endif
		
		}
Error:	
	if (bitmap)
		DiscardBitmap(bitmap);
	return 0;
}

int SetCommentsData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int index)
{
	int				len, i=0, j, error, update, found=0;
	char 			*data = NULL, *string = NULL;
 	id3frame 		*frame = NULL;
	union id3_field	*field = {0};
	id3_ucs4_t const *ucs4temp;
	id3_ucs4_t 		*ucs4Str = NULL;

	errChk(GetCtrlVal(panel, updateCtrl, &update));
	if (!update) 
		goto Error;		
	else
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);

	nullChk(data = calloc(len+1, sizeof(char)));
	errChk(GetCtrlVal(panel, control, data));
					
Start:		
	while(!found && (frame = id3_tag_findframe(id3tag, frameType, i++))) {
		if (!frame) {
			frame = NewFrame(id3tag, frameType);
			id3_field_setlanguage(&frame->fields[1], "eng");
			}
			
		for (j=1;j<frame->nfields;j++) {
			field = id3_frame_field(frame, j);	// do we always want the 2nd field?
			if (!field)
				return 0;

			if (field->type == ID3_FIELD_TYPE_STRING) { /* should be description field */
				ucs4temp = id3_field_getstring(field);
				string = (char *)id3_ucs4_latin1duplicate(ucs4temp);
				if (stristr(string, "iTunPGAP") || 
					stristr(string, "iTunSMPB") || 
					stristr(string, "iTunNORM"))
					j = frame->nfields;	// don't do anything with iTunes Comment Frames
				free(string);
				string = NULL;
				}
			else if (field->type == ID3_FIELD_TYPE_STRINGFULL) {
				found = 1;
				break;
				}
			}

		}

	if (len) {
		if (found && field->type == ID3_FIELD_TYPE_STRINGFULL) {
			ucs4Str = id3_latin1_ucs4duplicate((id3_latin1_t *)data);
			free(string);
			string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
			id3_field_setfullstring(field, ucs4Str);
			}
		if (!found) {
			/* there were comment frames, but they weren't general comments */
			frame = id3_frame_new(frameType);
			id3_tag_attachframe(id3tag, frame);
			id3_field_settextencoding(&frame->fields[0], ID3_FIELD_TEXTENCODING_UTF_8);
			id3_field_setlanguage(&frame->fields[1], "eng");
			}
		}
	else if (found) {
		id3_tag_detachframe(id3tag, frame);
		id3_frame_delete(frame);
		}
	
Error:
	if (ucs4Str)
		free(ucs4Str);
	if (data)
		free(data);
	if (string)
		free(string);
	
	return error;
}

int SetGenreData(id3_Tag *id3tag, char *frameType, int panel, int control, int updateCtrl, int index)
{
	int			len, error, update;
	char 		*data = NULL;
	id3frame 	*frame;
	union id3_field	 *field;
	id3_ucs4_t *ucs4genre = NULL;
	
	errChk(GetCtrlVal(panel, updateCtrl, &update));
	if (!update) 
		goto Error;		
	else {
		GetTableCellValLength(panel, control, tagCell, &len);
		nullChk(data = calloc(len+1, sizeof(char)));
		GetTableCellVal(panel, control, tagCell, data);
		}

	if (len) {	// add/update tag
		RemoveFrame(id3tag, frameType);	// we need to remove this frame and start over

		frame = NewFrame(id3tag, frameType);
		field = id3_frame_field(frame, 1);
		if (!field)
			return 0;
		if (field->type == ID3_FIELD_TYPE_STRINGLIST) {
			ucs4genre = id3_latin1_ucs4duplicate((id3_latin1_t *)data);
			id3_field_addstring(field, ucs4genre);
			}
		}

Error:
	if (data)
		free(data);
	if (ucs4genre)
		free(ucs4genre);
	return 1;
}


/* currently this can only be used to set the "ALBUM ARTIST" TXXX field */
int SetTextInformation(id3_Tag *id3tag, char *descString, int panel, int control, int updatePanel, int updateCtrl, int index)
{
	int  			len, error, update, style;
	char			*data = NULL;
	id3frame 		*frame;
	union id3_field	*field;
	id3_ucs4_t 		*ucs4desc = NULL;
	id3_ucs4_t 		*ucs4data = NULL;
	
	errChk(GetCtrlVal(updatePanel, updateCtrl, &update));
	if (!update) 
		goto Error;

	GetCtrlAttribute(panel, control, ATTR_CTRL_STYLE, &style);
	if (style == CTRL_TABLE_LS)
		GetTableCellValLength(panel, control, tagCell, &len);
	else
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
	nullChk(data = calloc(len+1, sizeof(char)));
	if (style == CTRL_TABLE_LS)
		errChk(GetTableCellVal(panel, control, tagCell, data))
	else
		errChk(GetCtrlVal(panel, control, data));

	// we are setting this field so we need to remove it first
	RemoveTextInformation(id3tag, descString);
	frame = NewFrame(id3tag, "TXXX");
	field = id3_frame_field(frame, 1);
	if (!field)
		errChk(-1);
	if (field->type == ID3_FIELD_TYPE_STRING) {
		ucs4desc = id3_latin1_ucs4duplicate((id3_latin1_t *)descString);
		id3_field_setstring(field, ucs4desc);
		}
	field = id3_frame_field(frame, 2);
	if (!field)
		errChk(-1);
	if (field->type == ID3_FIELD_TYPE_STRING) {
		ucs4data = id3_latin1_ucs4duplicate((id3_latin1_t *)data);
		id3_field_setstring(field, ucs4data);
		}
	
Error:
	if (data)
		free(data);
	if (ucs4desc)
		free(ucs4desc);
	if (ucs4data)
		free(ucs4data);
	return error;
}

int SetExtendedFields(id3_Tag *id3tag, int index)
{
	int	i, numItems, checked, descLen;
	char *description=NULL, fieldType[255];
	
	GetNumListItems(tab3Handle, TAB3_EXTENDEDTAGS, &numItems);
	for (i=0;i<numItems;i++) {
		GetTreeItemAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, ATTR_MARK_STATE, &checked);
		if (!checked) {
			// remove this field!
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, 0, ATTR_LABEL_TEXT_LENGTH, &descLen);
			description = malloc(sizeof(char) * descLen + 1);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, 0, ATTR_LABEL_TEXT, description);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, 2, ATTR_LABEL_TEXT, fieldType);
			if (!strcmp(fieldType, "TXXX")) {
				RemoveTextInformation(id3tag, description);
				}
			if (description)
				free(description);
			description = NULL;
			}
		}
		
	return 1;
}

int RemoveTextInformation(id3_Tag *id3tag, char *textDescription)
{
	int				i=0, j, found=0;
	char 			*string = NULL;
 	id3frame 		*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;

	while(!found && (frame = id3_tag_findframe(id3tag, "TXXX", i++))) {
		if (!frame)
			return 0;
		if (frame->nfields < 2)
			Breakpoint();
		for (j=1;j<frame->nfields;j++) {
			field = id3_frame_field(frame, j);	// do we always want the 2nd field?
			if (!field)
				return 0;

			if (field->type == ID3_FIELD_TYPE_STRING) { /* should be description field */
				ucs4Str = id3_field_getstring(field);
				string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
				if (!stricmp(string, textDescription)) {
					free(string);
					string = NULL;
					
#if _CVI_DEBUG_					
					field = id3_frame_field(frame, j+1);	/* just so we can see what we're about to delete */
					if (field) {
						ucs4Str = id3_field_getstring(field);
						string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
						j=frame->nfields;
						}
#endif /* _CVI_DEBUG_ */
					id3_tag_detachframe(id3tag, frame);
					id3_frame_delete(frame);
					goto Error;
					}
				else {
					free(string);
					string = NULL;
					j=frame->nfields;	// skip this frame
					}
				}
			}

		}

	
Error:
	if (string)
		free(string);
	return found;
}

/****************************************/

int RemoveFrame(id3_Tag *id3tag, char *frameType)
{
	int			removed = 0;
	id3frame 	*frame;

	while (frame = id3_tag_findframe(id3tag, frameType, 0)) {
		if (frame) {
			id3_tag_detachframe(id3tag, frame);
			id3_frame_delete(frame);
			removed++;
			}
		}
	return removed;
}

/* Remove a TXXX field with description == descString */ /***** UNTESTED!!!! *****/
int RemoveTextInformationFrame(id3_Tag *id3tag, char *descString)
{
	int				i=0, j, removed = 0, found=0;
	char 			*string = NULL;
 	id3frame 		*frame;
	union id3_field	*field;
	id3_ucs4_t const *ucs4Str;

	while((frame = id3_tag_findframe(id3tag, "TXXX", i++)) && !found) {
		if (frame) {
			for (j=1;j<frame->nfields;j++) {
				field = id3_frame_field(frame, j);	// do we always want the 2nd field?
				if (field && field->type == ID3_FIELD_TYPE_STRING) { /* should be description field */
					ucs4Str = id3_field_getstring(field);
					string = (char *)id3_ucs4_latin1duplicate(ucs4Str);
					if (!stricmp(string, descString)) {
						id3_tag_detachframe(id3tag, frame);
						id3_frame_delete(frame);
						removed++;
						found = 1;
					}
				}
			}
		}
	}
	if (string)
		free(string);
	return removed;
}

id3frame* NewFrame(id3_Tag *id3tag, char *frameType)
{
	id3frame *frame = NULL;
	
	frame = id3_frame_new(frameType);
	id3_tag_attachframe(id3tag, frame);
	id3_field_settextencoding(&frame->fields[0], ID3_FIELD_TEXTENCODING_UTF_8);

	return frame;
}

int CheckForDuplicates(char **stringVals, int nStrings) {
	int i, j, k;
	char *comma;
	char *reorderedStr = NULL;

	if (nStrings > 1) {
		/* regular search */
		for (i=0;i<nStrings-1;i++) {
			for (j=i+1;j<nStrings;j++) {
				if (!stricmp(stringVals[i], stringVals[j])) {
					free(stringVals[j]);
					for (k=j;k<nStrings-1;k++) {
						stringVals[k] = stringVals[k+1];
					}
					stringVals[k] = NULL;
					nStrings--;
				}
			}
		}
	}
	if (nStrings > 1) {
		/* backwards search to handle comma parsed names: "Black, Jack" "Who, The" */
		for (i=0;i<nStrings-1;i++) {
			if (comma = strstr(stringVals[i], ", ")) {
				reorderedStr = malloc(sizeof(char) * strlen(stringVals[i]) + 1);
				strcpy(reorderedStr, comma + 2);
				strcat(reorderedStr, " ");
				strncat(reorderedStr, stringVals[i], comma-stringVals[i]);
				for (j=i+1;j<nStrings;j++) {
					if (!stricmp(reorderedStr, stringVals[j])) {
						free(stringVals[j]);
						for (k=j;k<nStrings-1;k++) {
							stringVals[k] = stringVals[k+1];
						}
						stringVals[k] = NULL;
						nStrings--;
					}
				}
				free(reorderedStr);
				reorderedStr = NULL;
			}
		}
	}
	
	return nStrings;
}

void AllocAndCopyStr(char **string, char *val)
{
	int len, error;
	
	len = strlen(val);
	nullChk(*string = malloc(sizeof(char) * len + 1));
	strcpy(*string, val);
Error:
	return;
}

/****************************************/

extern char panelLEDList[2];
extern char tab1LEDList[kNumTab1Controls];
extern char tab2LEDList[kNumTab2Controls];

int CVICALLBACK StringCompare(void *item1, void *item2)
{
	// the standard CStringCompare wasn't working for some reason so had to create this version
	return strcmp(item1, item2);	
}

/* Turns on the "conflict" LED and populates a tooltip with the conflict values */
void SetConflictTooltips(panel)
{
	char 		***dataPtr = NULL;
	char 		buf[(kTooltipLineLength+2)*numFiles];
	char		str[kTooltipLineLength+1];
	ListType	list;
	size_t		offset=0;
	int			i,j, val, blankLine;
	int 		panelList[kEndOfList] = {0}, ledList[kEndOfList] = {0};	
	
	for (i=0,j=0;i<sizeof(panelLEDList);i++,j++) {
		GetCtrlVal(panel, panelLEDList[i], &val);
		if (val) {
			ledList[j]=panelLEDList[i];
			panelList[j]=panel;
			}
		else
			ledList[j]=0;
		}
	for (i=0;i<sizeof(tab1LEDList);i++,j++) {
		GetCtrlVal(tab1Handle, tab1LEDList[i], &val);
		if (val) {
			ledList[j]=tab1LEDList[i];
			panelList[j]=tab1Handle;
			}
		else
			ledList[j]=0;
		}
	for (i=0;i<sizeof(tab2LEDList);i++,j++) {
		GetCtrlVal(tab2Handle, tab2LEDList[i], &val);
		if (val) {
			ledList[j]=tab2LEDList[i];
			panelList[j]=tab2Handle;
			}
		else
			ledList[j]=0;
		}
	
	dataPtr = calloc(kEndOfList, sizeof(char**));
	// main panel
	dataPtr[0] 											= dataHandle.artistPtr;
	dataPtr[1] 											= dataHandle.albumPtr;
	// tab1
	dataPtr[kNumPanelControls + 0] 						= dataHandle.genrePtr;
	dataPtr[kNumPanelControls + 1] 						= dataHandle.commentPtr;
	dataPtr[kNumPanelControls + 2] 						= dataHandle.yearPtr;
	dataPtr[kNumPanelControls + 3] 						= dataHandle.discPtr;
	dataPtr[kNumPanelControls + 4] 						= dataHandle.composerPtr;
	dataPtr[kNumPanelControls + 5] 						= dataHandle.publisherPtr;
	dataPtr[kNumPanelControls + 6] 						= dataHandle.encodedPtr;
	dataPtr[kNumPanelControls + 7]						= dataHandle.countryPtr;
	dataPtr[kNumPanelControls + 8] 						= dataHandle.relTypePtr;
	dataPtr[kNumPanelControls + 9] 						= dataHandle.albumArtistPtr;
	dataPtr[kNumPanelControls + 10]						= dataHandle.artistFilterPtr;
	dataPtr[kNumPanelControls + 11]						= dataHandle.perfSortOrderPtr;
	dataPtr[kNumPanelControls + 12] 					= dataHandle.albumGainPtr;
	dataPtr[kNumPanelControls + 13]						= dataHandle.albSortOrderPtr;
	// tab2
	dataPtr[kNumPanelControls + kNumTab1Controls + 0] 	= dataHandle.origArtistPtr;
	dataPtr[kNumPanelControls + kNumTab1Controls + 1] 	= dataHandle.urlPtr;
	dataPtr[kNumPanelControls + kNumTab1Controls + 2]	= dataHandle.copyrightPtr;

	for (i=0;i<kEndOfList;i++)
		if (ledList[i]) {
			list = ListCreate(sizeof(char)*kTooltipLineLength);
			offset=0;
			for (j=0;j<numFiles;j++) {
				blankLine = 0;
				if (dataPtr[i][j] != NULL && !ListFindItem(list, dataPtr[i][j], FRONT_OF_LIST, StringCompare)) {
					if (!strcmp(dataPtr[i][j],""))
							blankLine = 1;
					ListInsertItem(list, dataPtr[i][j], FRONT_OF_LIST);
					if (blankLine)
						strcpy(str, kBlankStr);
					else {
						if (strlen(dataPtr[i][j])<kTooltipLineLength)
							strcpy(str, dataPtr[i][j]);
						else
							snprintf(str, kTooltipLineLength-3, "%s", dataPtr[i][j]);
							strcat(str, "...");
						}
					sprintf(buf+offset, offset ? "\n%s" : "%s", str);
					if (offset)
						offset++; // linebreak
					offset += strlen(dataPtr[i][j]) < kTooltipLineLength ? 
						(blankLine ? strlen(kBlankStr) : strlen(dataPtr[i][j])) : kTooltipLineLength;
					}
				}
			buf[offset] = 0;
			ListDispose(list);
		
			SetCtrlAttribute(panelList[i], ledList[i], ATTR_TOOLTIP_TEXT, buf);
			SetCtrlAttribute(panelList[i], ledList[i], ATTR_DISABLE_CTRL_TOOLTIP, 0);
			//SetCtrlToolTipAttribute(panelList[i], ledList[i], CTRL_TOOLTIP_ATTR_TEXT, buf);
			//SetCtrlToolTipAttribute(panelList[i], ledList[i], CTRL_TOOLTIP_ATTR_ENABLE, 1);
		}
Error:
	free(dataPtr);
}

/****************************************/

void PopulateGenreComboBox(int panel, int control)
{
	InsertTableCellRingItem(panel, control, tagCell, -1, "Alternative");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Blues");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Classic Rock");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Classical");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Country");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Dance");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Death Metal");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Disco");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Doom Metal");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Folk");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Funk");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Grunge");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Hard Rock");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Industrial");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Instrumental");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Jazz");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Math Rock");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Metal");
	InsertTableCellRingItem(panel, control, tagCell, -1, "New Age");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Oldies");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Other");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Pop");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Post-Hardcore");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Post-Metal");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Post-Rock");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Progressive Metal");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Progressive Rock");
	InsertTableCellRingItem(panel, control, tagCell, -1, "R&B");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Rap");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Rock");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Ska");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Sludge");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Soundtrack");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Synthwave");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Techno");
	InsertTableCellRingItem(panel, control, tagCell, -1, "Thrash Metal");
	return;
}

