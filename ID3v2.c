#include "combobox.h"
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <userint.h>
#include "mp3renamer.h"
#include "ID3v2.h"
#include "toolbox.h"

#include "globals.h"

#include "tag_c.h"
#include "json.h"

/*** Globals ***/

char PADDING[3] = {0,0,0};
const Point tagCell = {1,1};
char gFilename[MAX_FILENAME_LEN];	// needed to get filename from file.c


/*** Prototypes ***/

int GetTitleData(jsmntok_t *tokens, char *pmapJSON, char *frameType, int panel, int control, int index);
int GetTextData(jsmntok_t *tokens, char *pmapJSON, char *frameType, int panel, int control, int conflict, int index);
int GetUnhandledFields(jsmntok_t *tokens, char *pmapJSON, int index);
int GetPictureData(TagLib_File *taglibfile, char *frameType, int panel, int control, int conflict, int index);

size_t SearchJSONForKey(jsmntok_t *tokens, char *pmapJSON, char *frameType, char **string);


int SetTextData(TagLib_File *taglibfile, char *frameType, int panel, int control, int updateCtrl, int index, int multi);
int SetUnhandledFields(TagLib_File *taglibfile, int panel, int control, int index);
int SetPictureData(TagLib_File *taglibfile, int panel, int control, int updateCtrl, int clearCtrl);

int isHandledFrameType(char *id);

int CVICALLBACK StringCompare(void *item1, void *item2);
int CheckForDuplicates(char **stringVals, int nStrings);
void AllocAndCopyStr(char **string, char *val);

/*** Code ***/


int GetID3v2Tag(int panel, char *filename, int index)
{
	TagLib_File *taglibfile;
	TagLib_Tag *tag;
	char *pmapJSON = NULL;
	size_t len;
	
	taglib_set_strings_unicode(FALSE);
	taglibfile = taglib_file_new(filename);
	tag = taglib_file_tag(taglibfile);

	len = taglib_file_property_map_to_JSON_length(taglibfile);
	pmapJSON = malloc(len * sizeof(char) + 1);
	strcpy(pmapJSON, taglib_file_property_map_to_JSON(taglibfile));
	jsmntok_t *tokens = json_tokenise(pmapJSON, len);

	
	GetTitleData(tokens,pmapJSON, "TITLE", panel, PANEL_TREE, index);
	GetTextData(tokens, pmapJSON, "ALBUM", panel, PANEL_ALBUM, PANEL_ALBUMLED, index);
	GetTextData(tokens, pmapJSON, "ARTIST", panel, PANEL_ARTIST, PANEL_ARTISTLED, index);
	GetTextData(tokens, pmapJSON, "ALBUMSORT", tab1Handle, TAB1_ALBUMSORTORDER, TAB1_ALBUMSORTLED, index);
	GetTextData(tokens, pmapJSON, "ALBUMARTIST", tab1Handle, TAB1_ALBUMARTIST, TAB1_ALBUMARTISTLED, index);
	GetTextData(tokens, pmapJSON, "ARTISTCOUNTRY", tab1Handle, TAB1_COUNTRY, TAB1_COUNTRYLED, index);
	GetTextData(tokens, pmapJSON, "ARTISTFILTER", tab1Handle, TAB1_ARTISTFILTER, TAB1_ARTISTFILTERLED, index); // overloading the TPE3 (Conductor field) to handle my ArtistFilter tag
	GetTextData(tokens, pmapJSON, "ARTISTSORT", tab1Handle, TAB1_PERFORMERSORTORDER, TAB1_PERFSORTLED, index);
	GetTextData(tokens, pmapJSON, "COPYRIGHT", tab2Handle, TAB2_COPYRIGHT, TAB2_COPYRIGHTLED, index);
	GetTextData(tokens, pmapJSON, "COMMENT", tab1Handle, TAB1_COMMENT, TAB1_COMMENTLED, index);
	GetTextData(tokens, pmapJSON, "COMPOSER", tab2Handle, TAB2_COMPOSER, TAB2_COMPOSERLED, index);
	GetTextData(tokens, pmapJSON, "DATE", tab1Handle, TAB1_YEAR, TAB1_YEARLED, index);
	GetTextData(tokens, pmapJSON, "DISCNUMBER", tab1Handle, TAB1_DISCNUM, TAB1_DISCNUMLED, index);
	GetTextData(tokens, pmapJSON, "DISCSUBTITLE", tab1Handle, TAB1_DISCSUBTITLE, TAB1_DISCSUBTITLELED, index);
	GetTextData(tokens, pmapJSON, "EDITION", tab1Handle, TAB1_EDITION, TAB1_EDITIONLED, index);
	GetTextData(tokens, pmapJSON, "ENCODEDBY", tab2Handle, TAB2_ENCODED, TAB2_ENCODEDLED, index);
	GetTextData(tokens, pmapJSON, "GENRE", tab1Handle, TAB1_GENRE, TAB1_GENRELED, index);
	GetTextData(tokens, pmapJSON, "PUBLISHER", tab1Handle, TAB1_PUBLISHER, TAB1_PUBLISHERLED, index);
	GetTextData(tokens, pmapJSON, "LABEL", tab1Handle, TAB1_PUBLISHER, TAB1_PUBLISHERLED, index);
	GetTextData(tokens, pmapJSON, "MUSICBRAINZ_ARTISTID", tab3Handle, TAB3_ARTISTMBID, 0, index);
	GetTextData(tokens, pmapJSON, "MUSICBRAINZ_RELEASEGROUPID", tab3Handle, TAB3_REID, 0, index);
	GetTextData(tokens, pmapJSON, "ORIGINALARTIST", tab2Handle, TAB2_ORIGARTIST, TAB2_ORIGARTISTLED, index);
	GetTextData(tokens, pmapJSON, "RELEASETYPE", tab1Handle, TAB1_RELTYPE, TAB1_RELTYPELED, index);
	GetTextData(tokens, pmapJSON, "REPLAYGAIN_ALBUM_GAIN", tab1Handle, TAB1_ALBUMGAIN, TAB1_ALBUMGAINLED, index);
	GetTextData(tokens, pmapJSON, "TRACKNUMBER", panel, PANEL_TRACKNUM, PANEL_TRACKNUMLED, index);
	GetTextData(tokens, pmapJSON, "URL", tab2Handle, TAB2_URL, TAB2_URLLED, index);
	
	GetPictureData(taglibfile, "APIC", tab2Handle, TAB2_ARTWORK, TAB2_ARTWORKLED, index);
	GetUnhandledFields(tokens, pmapJSON, index);
	free(tokens);

Error:
	if (pmapJSON)
		free(pmapJSON);
	if (taglibfile)
		taglib_file_free(taglibfile);
	taglib_tag_free_strings();
	return 0;
}

int GetTitleData(jsmntok_t *tokens, char *pmapJSON, char *frameType, int panel, int control, int index)
{
	int			found = 0;
	char 		*string = NULL;
	size_t		len;

	len = SearchJSONForKey(tokens, pmapJSON, frameType, &string);
	if (len) {
		SetTreeCellAttribute(panel, control, index, kTreeColTrackName, ATTR_LABEL_TEXT, string);
		found = 1;
	}

Error:
	if (string)
		free(string);
	return found;
}

void UnescapeQuotes(char *string)
{
	char 	*ptr, *temp = NULL;
	size_t	len;
	
	len = strlen(string);
	temp = malloc(sizeof(char) * len + 1);
	while (ptr = strstr(string, "\\\"")) {
		strcpy(temp, ptr+1);	// skip the backslash
		strcpy(ptr, temp);
	}
	if (temp)
		free (temp);
}

size_t SearchJSONForKey(jsmntok_t *tokens, char *pmapJSON, char *frameType, char **string)
{
	int i=0;
	size_t len = 0;
	
	jsmntok_t *t = &tokens[i];
	while (t->type > JSMN_UNDEFINED) {
		if (t->type == JSMN_STRING && json_token_streq(pmapJSON, t, frameType)) {
			t = &tokens[++i];	// value of key
			len = strlen(json_token_tostr(pmapJSON, t));
			*string = malloc(sizeof(char) * len + 1);
			strcpy(*string, json_token_tostr(pmapJSON, t));
			UnescapeQuotes(*string);
			break;
		}
		t = &tokens[++i];
	}
	
	return len;
}

int GetTextData(jsmntok_t *tokens, char *pmapJSON, char *frameType, int panel, int control, int conflict, int index)
{
	int		error, found = 0, style;
	int		len = 0;
	char 	*origData = NULL, *string = NULL;

	
	len = SearchJSONForKey(tokens, pmapJSON, frameType, &string);
	if (len) {
		
		GetCtrlAttribute(panel, control, ATTR_CTRL_STYLE, &style);
		switch (style) {
			case CTRL_STRING:
			case CTRL_STRING_LS:
			case CTRL_TEXT_BOX:
			case CTRL_TEXT_BOX_LS:
				GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
				nullChk(origData = malloc(len+1));
				GetCtrlVal(panel, control, origData);
				break;
			case CTRL_TABLE_LS:
				GetTableCellValLength(panel, control, tagCell, &len);
				nullChk(origData = malloc(len+1));
				GetTableCellVal(panel, control, tagCell, origData);
				break;
		}

		if (!firstFile && (len!=strlen(string) || memcmp(origData, string, len))) {
			SetCtrlVal(panel, conflict, 1);
		} else {
			switch (style) {
				case CTRL_TEXT_BOX:
				case CTRL_TEXT_BOX_LS:
					errChk(ResetTextBox(panel, control, string));
					break;
				case CTRL_STRING:
				case CTRL_STRING_LS:
					errChk(SetCtrlVal(panel, control, string));
					break;
				case CTRL_TABLE_LS:
					SetTableCellVal(panel, control, tagCell, string);
					break;
			}
		}

		StoreDataVals(panel, control, string, index);
		found = 1;
	}

Error:
	if (origData)
		free(origData);
	if (string)
		free(string);
	return found;
}

int isHandledFrame(char *key)
{
	if (!stricmp("TITLE", key) ||
		!stricmp("ALBUM", key) ||
		!stricmp("ALBUMARTIST", key) ||
		!stricmp("ALBUMSORT", key) ||
		!stricmp("ARTIST", key) ||
		!stricmp("ARTISTCOUNTRY", key) ||
		!stricmp("ARTISTFILTER", key) ||
		!stricmp("ARTISTSORT", key) ||
		!stricmp("ENCODEDBY", key) ||
		!stricmp("COPYRIGHT", key) ||
		!stricmp("COMMENT", key) ||
		!stricmp("COMPOSER", key) ||
		!stricmp("DATE", key) ||
		!stricmp("DISCNUMBER", key) ||
		!stricmp("DISCSUBTITLE", key) ||
		!stricmp("EDITION", key) ||
		!stricmp("GENRE", key) ||
		!stricmp("LABEL", key) ||
		!stricmp("MUSICBRAINZ_RELEASEGROUPID", key) ||
		!stricmp("MUSICBRAINZ_ARTISTID", key) ||
		!stricmp("ORIGINALARTIST", key) ||
		!stricmp("RELEASETYPE", key) ||
		!stricmp("REPLAYGAIN_ALBUM_GAIN", key) ||
		!stricmp("REPLAYGAIN_ALBUM_PEAK", key) ||
		!stricmp("REPLAYGAIN_TRACK_GAIN", key) ||
		!stricmp("REPLAYGAIN_TRACK_PEAK", key) ||
		!stricmp("TRACKNUMBER", key) ||
		!stricmp("URL", key))
		return 1;
	else
		return 0;
}

int GetUnhandledFields(jsmntok_t *tokens, char *pmapJSON, int index)
{
	int 	i=0, numItems, found = -1;
	size_t	len;
	char	frameType[255];
	char	*string = NULL;
	jsmntok_t *t = &tokens[i];

	while (t->type > JSMN_UNDEFINED) {
		if (t->type == JSMN_STRING) {
			strcpy(frameType, json_token_tostr(pmapJSON, t));
			if (!isHandledFrame(frameType)) {

				t = &tokens[++i];	// value of key
				len = strlen(json_token_tostr(pmapJSON, t));
				string = malloc(sizeof(char) * len + 1);
				strcpy(string, json_token_tostr(pmapJSON, t));
				UnescapeQuotes(string);
			
				GetNumListItems(tab3Handle, TAB3_EXTENDEDTAGS, &numItems);
				if (numItems) {
					GetTreeItemFromLabel(tab3Handle, TAB3_EXTENDEDTAGS, VAL_ALL, 0, 0, VAL_NEXT_PLUS_SELF, 0, frameType, &found);
				}
				if (found==-1) {	// only insert fields into tree if they don't already exist
					InsertTreeItem(tab3Handle, TAB3_EXTENDEDTAGS, VAL_SIBLING, 0, VAL_LAST, frameType, NULL, NULL, numItems);
					SetTreeItemAttribute (tab3Handle, TAB3_EXTENDEDTAGS, numItems, ATTR_MARK_STATE, 1);
					SetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, numItems, kUnhandledTreeColValue, ATTR_LABEL_TEXT, string);
					SetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, numItems, kUnhandledTreeColOrigValue, ATTR_LABEL_TEXT, string);
					SetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, numItems, kUnhandledTreeColOrigField, ATTR_LABEL_TEXT, frameType);
					SetCtrlVal(panelHandle, PANEL_TABVALS2, 1);
				}
				free(string);
				string = NULL;
			} else {
				t = &tokens[++i];	// this does not handle arrays or objects
			}
		}
		t = &tokens[++i];
	}

	if (string)
		free(string);
	return 0;
}


int GetImageTypeExtension(char *type, char ext[4])
{
	int found=0;
	char errMsg[255];

	if (stristr(type, "png")) {
		strcpy(ext, "png\0");
	} else if (stristr(type, "jpeg") || stristr(type, "jpg")) {
		strcpy(ext, "jpg\0");
	} else if (stristr(type, "gif")) {
		// We can't handle .gif files
		//strcpy(ext, "gif\0");
		found = -1;
	} else {
		sprintf(errMsg, "Undefined mimeType = %s", type);
		MessagePopup("ERROR",errMsg);
		found = -1;
	}

	return found;
}

int GetPictureData(TagLib_File *taglibfile, char *frameType, int panel, int control, int conflict, int index)
{
	int			error;
	unsigned int pictureType;
	int			width, height, bitmap = 0;
	char 		mimeType[255], imageNameStr[MAX_FILENAME_LEN], ext[4], sizeStr[100];
	char		*desc = NULL;
	char		*data = NULL;

	strcpy(mimeType, taglib_mp3_file_picture_attrs(taglibfile, &pictureType));
	if (strlen(mimeType)) {
		errChk(GetImageTypeExtension(mimeType, ext));
		sprintf(imageNameStr, "imageFile%d.%s", index, ext);
		taglib_mp3_file_picture(taglibfile, imageNameStr);

		DisableBreakOnLibraryErrors();
		if (!DisplayImageFile(panel, control, imageNameStr)) {
			SetCtrlAttribute(panel, TAB2_IMAGECORRUPTEDMSG, ATTR_VISIBLE, 0);
			GetCtrlBitmap(panel, control, 0, &bitmap);
			GetBitmapData(bitmap, NULL, NULL, &width, &height, NULL, NULL, NULL);
			sprintf(sizeStr, kPictureSizeStr, width, height);
			SetCtrlVal(panel, TAB2_IMAGESIZEMSG, sizeStr);
		} else {
			SetCtrlAttribute(panel, TAB2_IMAGECORRUPTEDMSG, ATTR_VISIBLE, 1);
			sprintf(sizeStr, kPictureSizeStr, 0, 0);
			SetCtrlVal(panel, TAB2_IMAGESIZEMSG, sizeStr);
		}
		EnableBreakOnLibraryErrors();
		SetCtrlVal(panelHandle, PANEL_TABVALS, 1);
		SetCtrlAttribute(panel, TAB2_CLEARARTWORK, ATTR_VISIBLE, 1);
	} else {
		DisableBreakOnLibraryErrors();
		DeleteImage(panel, control);
		EnableBreakOnLibraryErrors();
	}

Error:
	if (desc)
		free(desc);
	if (data)
		free(data);
	if (imageNameStr)
		DeleteFile(imageNameStr);
	if (bitmap)
		DiscardBitmap(bitmap);
	return 1;
}


void StoreDataVals(int panel, int control, char *string, int index)
{
	size_t size, len;
	char *val = NULL;

	size = strlen(string)+1;
	if (panel == panelHandle) {
		switch (control) {
			case PANEL_ARTIST:
				dataHandle.artistPtr[index] = calloc(size + 2, sizeof(char));	// we need a fudge factor for TPE2 field
				strcpy(dataHandle.artistPtr[index], string);
				SetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColArtistName, ATTR_LABEL_TEXT, string);
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
			case TAB1_GENRE:
				dataHandle.genrePtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.genrePtr[index], string);
				break;
			case TAB1_YEAR:
				dataHandle.yearPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.yearPtr[index], string);
				break;
			case TAB1_DISCNUM:
				dataHandle.discPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.discPtr[index], string);
				gUseMetaDataDiscVal = TRUE;
				break;
			case TAB1_DISCSUBTITLE:
				dataHandle.discSubtitlePtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.discSubtitlePtr[index], string);
				gUseMetaDataDiscVal = TRUE;
				break;
			case TAB1_ARTISTFILTER:
				dataHandle.artistFilterPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.artistFilterPtr[index], string);
				gUseMetaArtistFilter = TRUE;
				break;
			case TAB1_PUBLISHER:
				dataHandle.publisherPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.publisherPtr[index], string);
				break;
			case TAB1_EDITION:
				dataHandle.editionPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.editionPtr[index], string);
				break;
			case TAB1_COUNTRY:
				dataHandle.countryPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.countryPtr[index], string);
				break;
			case TAB1_ALBUMARTIST:
				dataHandle.albumArtistPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.albumArtistPtr[index], string);
				GetCtrlAttribute(panelHandle, PANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &len);
				val = malloc(sizeof(char) * len + 1);
				GetCtrlVal(panelHandle, PANEL_ARTIST, val);
				if (!stricmp(val, string) && stricmp(val, "Various Artists")) {
					SetCtrlVal(panel, control, "");	// clear out album artist because artist is already set to same value
					SetCtrlVal(panel, TAB1_ALBUMARTISTLED, false);
				}
				break;
			case TAB1_PERFORMERSORTORDER:
				dataHandle.perfSortOrderPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.perfSortOrderPtr[index], string);
				break;
			case TAB1_ALBUMSORTORDER:
				dataHandle.albSortOrderPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.albSortOrderPtr[index], string);
				break;
			case TAB1_COMMENT:
				dataHandle.commentPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.commentPtr[index], string);
				break;
			case TAB1_ALBUMGAIN:
				dataHandle.albumGainPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.albumGainPtr[index], string);
				break;
			case TAB1_RELTYPE:
				dataHandle.relTypePtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.relTypePtr[index], string);
				break;
			}
		}
	else if (panel == tab2Handle) {
		switch (control) {
			case TAB2_COMPOSER:
				dataHandle.composerPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.composerPtr[index], string);
				break;
			case TAB2_COPYRIGHT:
				dataHandle.copyrightPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.copyrightPtr[index], string);
				break;
			case TAB2_ENCODED:
				dataHandle.encodedPtr[index] = calloc(size, sizeof(char));
				strcpy(dataHandle.encodedPtr[index], string);
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
	
	if (val)
		free(val);
}

/**************************************************/
/*** Functions below here are Set Tag Functions ***/
/**************************************************/

int SetID3v2Tag(int panel, char *filename, char *newname, int index)
{
	int 	error=0;
	TagLib_File *taglibfile;
	TagLib_Tag *tag;
	
	taglib_set_strings_unicode(FALSE);
	taglibfile = taglib_file_new(filename);
	tag = taglib_file_tag(taglibfile);
	
	
	strcpy(gFilename, filename);	// needed for file.c

	SetTextData(taglibfile, "TITLE", panel, PANEL_TREE, PANEL_UPDATETITLE, index, false);
	SetTextData(taglibfile, "TRACKNUMBER", panel, PANEL_TREE, PANEL_UPDATETRACKNUM, index, false);
	SetTextData(taglibfile, "ALBUM", panel, PANEL_ALBUM, PANEL_UPDATEALBUM, index, false);
	SetTextData(taglibfile, "ALBUMARTIST", tab1Handle, TAB1_ALBUMARTIST, TAB1_UPDATEALBUMARTIST, index, true);
	SetTextData(taglibfile, "ALBUMSORT", tab1Handle, TAB1_ALBUMSORTORDER, TAB1_UPDATEALBUMSORT, index, false);
	SetTextData(taglibfile, "ARTIST", panel, PANEL_ARTIST, PANEL_UPDATEARTIST, index, true);
	SetTextData(taglibfile, "ARTISTCOUNTRY", tab1Handle, TAB1_COUNTRY, TAB1_UPDATECOUNTRY, index, true);
	SetTextData(taglibfile, "ARTISTSORT", tab1Handle, TAB1_PERFORMERSORTORDER, TAB1_UPDATEPERFSORT, index, false);
	SetTextData(taglibfile, "COMMENT", tab1Handle, TAB1_COMMENT, TAB1_UPDATECOMMENT, index, false);
	SetTextData(taglibfile, "COMPOSER", tab2Handle, TAB2_COMPOSER, TAB2_UPDATECOMPOSER, index, true);
	SetTextData(taglibfile, "ARTISTFILTER", tab1Handle, TAB1_ARTISTFILTER, TAB1_UPDATEARTISTFILTER, index, true);
	SetTextData(taglibfile, "COPYRIGHT", tab2Handle, TAB2_COPYRIGHT, TAB2_UPDATECOPYRIGHT, index, true);
	SetTextData(taglibfile, "DATE", tab1Handle, TAB1_YEAR, TAB1_UPDATEYEAR, index, false);
	SetTextData(taglibfile, "DISCNUMBER", tab1Handle, TAB1_DISCNUM, TAB1_UPDATEDISCNUM, index, false);
	SetTextData(taglibfile, "DISCSUBTITLE", tab1Handle, TAB1_DISCSUBTITLE, TAB1_UPDATEDISCSUBTITLE, index, false);
	SetTextData(taglibfile, "EDITION", tab1Handle, TAB1_EDITION, TAB1_UPDATEEDITION, index, false);
	SetTextData(taglibfile, "ENCODEDBY", tab2Handle, TAB2_ENCODED, TAB2_UPDATEENCODED, index, true);
	SetTextData(taglibfile, "GENRE", tab1Handle, TAB1_GENRE, TAB1_UPDATEGENRE, index, true);
	SetTextData(taglibfile, "LABEL", tab1Handle, TAB1_PUBLISHER, TAB1_UPDATEPUBLISHER, index, true);
	SetTextData(taglibfile, "MUSICBRAINZ_ARTISTID", tab3Handle, TAB3_ARTISTMBID, TAB3_UPDATEMBID, index, true);
	SetTextData(taglibfile, "MUSICBRAINZ_RELEASEGROUPID", tab3Handle, TAB3_REID, TAB3_UPDATEREID, index, false);
	SetTextData(taglibfile, "ORIGINALARTIST", tab2Handle, TAB2_ORIGARTIST, TAB2_UPDATEORIGARTIST, index, true);
	SetTextData(taglibfile, "RELEASETYPE", tab1Handle, TAB1_RELTYPE, TAB1_UPDATERELTYPE, index, false);
	SetTextData(taglibfile, "URL", tab2Handle, TAB2_URL, TAB2_UPDATEURL, index, true);
	SetPictureData(taglibfile, tab2Handle, TAB2_ARTWORK, TAB2_UPDATEARTWORK, TAB2_CLEARARTWORK);
	SetUnhandledFields(taglibfile, tab3Handle, TAB3_EXTENDEDTAGS, index);
	
	error = taglib_file_save(taglibfile);
	
Error:
	if (taglibfile)
		taglib_file_free(taglibfile);
	taglib_tag_free_strings();	// do we need this?

	return error;
}

#define kMaxMultipleValues	10

int SetTextData(TagLib_File *taglibfile, char *frameType, int panel, int control, int updateCtrl, int index, int multi)
{
	int 			error, len, update, needsVarious=0, style;
	char 			*data=NULL, *strVal=NULL, *tmpStr = NULL;

	errChk(GetCtrlVal(panel, updateCtrl, &update));
	if (!update)
		goto Error;

	if (!strcmp(frameType, "DISCNUMBER")) {
		if (gUseMetaDataDiscVal && dataHandle.discPtr[index]) {	// disc num
			len = strlen(dataHandle.discPtr[index]);
			nullChk(data = calloc(len+1, sizeof(char)));
			strcpy(data, dataHandle.discPtr[index]);
		} else {
			GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
			nullChk(data = malloc(len+1));
			GetCtrlVal(panel, control, data);
			free(dataHandle.discPtr[index]);
			nullChk(dataHandle.discPtr[index] = malloc(len+1));
			strcpy(dataHandle.discPtr[index], data);
		}
		if (!strcmp(dataHandle.discPtr[index], "1/1") || !strcmp(dataHandle.discPtr[index], "01/01")) {
			strcpy(data, "");	// don't save 1/1 discs
			SetCtrlVal(panel, control, "");
		}
	} else if (!stricmp(frameType, "DISCSUBTITLE")) {
		if (gUseMetaDataDiscSubtitleVal && dataHandle.discSubtitlePtr[index]) {
			len = strlen(dataHandle.discSubtitlePtr[index]);
			nullChk(data = calloc(len+1, sizeof(char)));
			strcpy(data, dataHandle.discSubtitlePtr[index]);
		} else {
			GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
			nullChk(data = malloc(len+1));
			GetCtrlVal(panel, control, data);
			free(dataHandle.discSubtitlePtr[index]);
			nullChk(dataHandle.discSubtitlePtr[index] = malloc(len+1));
			strcpy(dataHandle.discSubtitlePtr[index], data);
		}
	} else if (!stricmp(frameType, "TITLE")) {
		GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColTrackName, ATTR_LABEL_TEXT_LENGTH, &len);
		nullChk(data = calloc(len+1, sizeof(char)));
		errChk(GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColTrackName, ATTR_LABEL_TEXT, data));
	} else if (!stricmp(frameType, "TRACKNUMBER")) {
		GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColTrackNum, ATTR_LABEL_TEXT_LENGTH, &len);
		nullChk(data = calloc(len+1, sizeof(char)));
		errChk(GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColTrackNum, ATTR_LABEL_TEXT, data));
	} else if (!stricmp(frameType, "ARTISTFILTER")) {			// If we're setting the custom "Artist Filter" check to see if Album Artist is VA/Soundtrack and add that, along with Artist
		//GetCtrlVal(panelHandle, PANEL_SHOWTRACKARTISTS, &trackArtists);
		GetCtrlAttribute(tab1Handle, TAB1_ALBUMARTIST, ATTR_STRING_TEXT_LENGTH, &len);
		if (!len && gUseMetaArtistFilter) 
			goto Error;	// don't update if no album artist
		else if (!len && !gUseMetaArtistFilter)
			goto Default;	// artist filter was explicitly edited, so set it
		nullChk(data = calloc(len+1, sizeof(char)));
		GetCtrlVal(tab1Handle, TAB1_ALBUMARTIST, data);
		if (stristr(data, "Various Artists")) {
			needsVarious = 1;
		} else if (stristr(data, "Soundtrack")) {
			needsVarious = 2;
		}
		free(data);
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
		if (len) {
			nullChk(data = calloc(len+20, sizeof(char)));
			errChk(GetCtrlVal(panel, control, data));
		} else if (needsVarious) {
			GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColArtistName, ATTR_LABEL_TEXT_LENGTH, &len);
			nullChk(data = calloc(len+20, sizeof(char)));
			errChk(GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColArtistName, ATTR_LABEL_TEXT, data));
		} else {
			GetCtrlAttribute(tab1Handle, TAB1_PERFORMERSORTORDER, ATTR_STRING_TEXT_LENGTH, &len);
			if (len > 0) {
				nullChk(data = calloc(len+20, sizeof(char)));	// +20 so we can add a comma & "; Various Artists" as needed
				errChk(GetCtrlVal(tab1Handle, TAB1_PERFORMERSORTORDER, data));
			} else {
				GetCtrlAttribute(panelHandle, PANEL_ARTIST, ATTR_STRING_TEXT_LENGTH, &len);
				nullChk(data = calloc(len+20, sizeof(char)));
				errChk(GetCtrlVal(panelHandle, PANEL_ARTIST, data));
			}
		}
		if (!strncmp("The ", data, 4) || !strncmp("the ", data, 4)) {
			memmove(data, data+4, strlen(data)-3);	// include NULL
			strcat(data, ", The");
		}

		if (needsVarious == 1 && !stristr(data, "Various Artists")) {
			strcat(data, "; Various Artists");	
		} else if (needsVarious == 2 && !stristr(data, "Soundtrack")) {
			strcat(data, "; Soundtrack");	
		}
		
	} else {		// handle everything but disc num and Artist Filter
Default:
		int val;
		GetCtrlVal(panelHandle, PANEL_SHOWTRACKARTISTS, &val);
		if (!stricmp(frameType, "ARTIST") && val == TRUE) {
			GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColArtistName, ATTR_LABEL_TEXT_LENGTH, &len);
			nullChk(data = calloc(len+1, sizeof(char)));
			errChk(GetTreeCellAttribute(panelHandle, PANEL_TREE, index, kTreeColArtistName, ATTR_LABEL_TEXT, data));
		} else {
			GetCtrlAttribute(panel, control, ATTR_CTRL_STYLE, &style);
			switch (style) {
				case CTRL_STRING:
				case CTRL_STRING_LS:
				case CTRL_TEXT_BOX:
				case CTRL_TEXT_BOX_LS:
					GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
					nullChk(data = malloc(len+1));
					GetCtrlVal(panel, control, data);
					break;
				case CTRL_TABLE_LS:
					GetTableCellValLength(panel, control, tagCell, &len);
					nullChk(data = malloc(len+1));
					GetTableCellVal(panel, control, tagCell, data);
					break;
			}
		}
	}	

	taglib_file_set_property(taglibfile, frameType, data, multi);
	
Error:
	if (data)
		free(data);
	if (tmpStr)
		free(tmpStr);
	if (strVal)
		free(strVal);

	return error;
}

int SetUnhandledFields(TagLib_File *taglibfile, int panel, int control, int index)
{
	int i, numItems, checked, len;
	char *key=NULL, *val=NULL, *origVal=NULL, *origKey=NULL;
	
	GetNumListItems(panel, control, &numItems);
	for (i=0;i<numItems;i++) {
		GetTreeItemAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, ATTR_MARK_STATE, &checked);
		GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColOrigField, ATTR_LABEL_TEXT_LENGTH, &len);	// name might have changed so use origKey
		origKey = malloc(sizeof(char) * len + 1);
		GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColOrigField, ATTR_LABEL_TEXT, origKey);
		if (checked) {
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColValue, ATTR_LABEL_TEXT_LENGTH, &len);
			val = malloc(sizeof(char) * len + 1);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColValue, ATTR_LABEL_TEXT, val);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColOrigValue, ATTR_LABEL_TEXT_LENGTH, &len);
			origVal = malloc(sizeof(char) * len + 1);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColOrigValue, ATTR_LABEL_TEXT, origVal);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColFieldName, ATTR_LABEL_TEXT_LENGTH, &len);
			key = malloc(sizeof(char) * len + 1);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColFieldName, ATTR_LABEL_TEXT, key);
			if (strcmp(key, origKey)) {
				taglib_file_set_property(taglibfile, origKey, "", false);
			}
			if (strcmp(val, origVal) || strcmp(key, origKey)) {
				// val was changed, so update in tags
				taglib_file_set_property(taglibfile, key, val, false);
			}
		} else {
			// remove this field!
			taglib_file_set_property(taglibfile, origKey, "", false);  // clears field
		}
		if (val)
			free(val);
		if (origVal)
			free(origVal);
		if (key)
			free(key);
		if (origKey)
			free(origKey);
		val = origVal = key = origKey = NULL;
	}

	return 1;
}

// currently just removes frame
// TODO: Ability to set pictures
int SetPictureData(TagLib_File *taglibfile, int panel, int control, int updateCtrl, int clearCtrl)
{
	int 			error, update, clear;

	errChk(GetCtrlVal(panel, updateCtrl, &update));
	errChk(GetCtrlVal(panel, clearCtrl, &clear));

	if (update) {
		if (clear) {
			taglib_mp3_file_remove_picture(taglibfile);
		}
	}

Error:
	return 0;
}


/****************************************/

// CheckForDuplicates was used in the ArtistFilter to prevent the same artist showing up twice
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

/* The "conflict" LED is set when data is read, so if one is set populate a tooltip with the conflict values */
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
		} else {
			ledList[j]=0;
		}
	}
	for (i=0;i<sizeof(tab1LEDList);i++,j++) {
		GetCtrlVal(tab1Handle, tab1LEDList[i], &val);
		if (val) {
			ledList[j]=tab1LEDList[i];
			panelList[j]=tab1Handle;
		} else {
			ledList[j]=0;
		}
	}
	for (i=0;i<sizeof(tab2LEDList);i++,j++) {
		GetCtrlVal(tab2Handle, tab2LEDList[i], &val);
		if (val) {
			ledList[j]=tab2LEDList[i];
			panelList[j]=tab2Handle;
		} else {
			ledList[j]=0;
		}
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
	dataPtr[kNumPanelControls + 4] 						= dataHandle.discSubtitlePtr;
	dataPtr[kNumPanelControls + 5] 						= dataHandle.publisherPtr;
	dataPtr[kNumPanelControls + 6] 						= dataHandle.editionPtr;
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
	dataPtr[kNumPanelControls + kNumTab1Controls + 3]	= dataHandle.encodedPtr;
	dataPtr[kNumPanelControls + kNumTab1Controls + 4] 	= dataHandle.composerPtr;

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

