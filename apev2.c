//==============================================================================
//
// Title:		apev2.c
// Purpose:		A short description of the implementation.
//
// Created on:	1/23/2016 at 6:50:09 PM by Kevin.
// Copyright:	Subversive Software. All Rights Reserved.
//
//==============================================================================

//==============================================================================
// Include files

#include <userint.h>
#include <ansi_c.h>
#include "apev2.h"
#include "apetag.h"
#include "mp3renamer.h"
#include "toolbox.h"
#include "globals.h"
#include "config.h"
#include "ID3v2.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// Static global variables

//==============================================================================
// Static functions

int GetAPETagData(struct ApeTag *tag, char *key, int panel, int control, int conflict, int index);
int GetAPETagUnhandledFields(struct ApeTag *tag, int index);
int ApeTag_iter_handle_fields(struct ApeTag *tag, struct ApeItem *item, void *data);
void ApeItem_handle_field(struct ApeItem *item);

void EncodeMultiValueString(char *str); // remove spaces after semi-colons

int SetAPETagUnhandledFields(struct ApeTag *tag, int index);

//==============================================================================
// Global variables

//==============================================================================
// Global functions

int LoadAPEv2Tag (int panel, char *fileName, int index)
{
	int error;
	struct ApeTag *tag = NULL;

	error = ApeInfo_process(fileName, &tag);
	GetAPETagData(tag, "title",  panelHandle, PANEL_TREE, kTreeColTrackName, index);
	GetAPETagData(tag, "album", panel, PANEL_ALBUM, PANEL_ALBUMLED, index);
	GetAPETagData(tag, "artist", panel, PANEL_ARTIST, PANEL_ARTISTLED, index);
	GetAPETagData(tag, "track", panel, PANEL_TRACKNUM, PANEL_TRACKNUMLED, index);
	GetAPETagData(tag, "year", tab1Handle, TAB1_YEAR, TAB1_YEARLED, index);
	GetAPETagData(tag, "album artist", tab1Handle, TAB1_ALBUMARTIST, TAB1_ALBUMARTISTLED, index);
	GetAPETagData(tag, "albumsortorder", tab1Handle, TAB1_ALBUMSORTORDER, TAB1_ALBUMSORTLED, index);
	GetAPETagData(tag, "artistcountry", tab1Handle, TAB1_COUNTRY, TAB1_COUNTRYLED, index);
	GetAPETagData(tag, "artistsortorder", tab1Handle, TAB1_PERFORMERSORTORDER, TAB1_PERFSORTLED, index);
	GetAPETagData(tag, "comment", tab1Handle, TAB1_COMMENT, TAB1_COMMENTLED, index);
	GetAPETagData(tag, "composer", tab1Handle, TAB1_COMPOSER, TAB1_COMPOSERLED, index);
	GetAPETagData(tag, "conductor", tab1Handle, TAB1_ARTISTFILTER, TAB1_ARTISTFILTERLED, index); // overloading the TPE3 (Conductor field) to handle my ArtistFilter tag
	GetAPETagData(tag, "copyright", tab2Handle, TAB2_COPYRIGHT, TAB2_COPYRIGHTLED, index);
	GetAPETagData(tag, "disc", tab1Handle, TAB1_DISCNUM, TAB1_DISCNUMLED, index);
	GetAPETagData(tag, "edition", tab1Handle, TAB1_EDITION, TAB1_EDITIONLED, index);
	GetAPETagData(tag, "encoded", tab2Handle, TAB2_ENCODED, TAB2_ENCODEDLED, index);
	GetAPETagData(tag, "genre", tab1Handle, TAB1_GENRE, TAB1_GENRELED, index);
	GetAPETagData(tag, "musicbrainz_artistid", tab3Handle, TAB3_ARTISTMBID, 0, index);
	GetAPETagData(tag, "musicbrainz_releasegroupid", tab3Handle, TAB3_REID, 0, index);
	GetAPETagData(tag, "original artist", tab2Handle, TAB2_ORIGARTIST, TAB2_ORIGARTISTLED, index);
	GetAPETagData(tag, "publisher", tab1Handle, TAB1_PUBLISHER, TAB1_PUBLISHERLED, index);
	GetAPETagData(tag, "releasetype", tab1Handle, TAB1_RELTYPE, TAB1_RELTYPELED, index);
	GetAPETagData(tag, "replaygain_album_gain", tab1Handle, TAB1_ALBUMGAIN, TAB1_ALBUMGAINLED, index);
	GetAPETagData(tag, "url", tab2Handle, TAB2_URL, TAB2_URLLED, index);

	//ApeTag_print(tag);	// unhandled fields
	GetAPETagUnhandledFields(tag, index);	// unhandled fields

	ApeTag_close(tag);
	
	return 0;
}

int GetAPETagData(struct ApeTag *tag, char *key, int panel, int control, int conflict, int index) 
{
	int 			len, style;
	int 			error = 0, i, j;
	char 			*data = NULL, *origData = NULL;
	struct ApeItem 	*item = NULL;
	
	if ((item = ApeTag_get_item(tag, key)) == NULL) {
		error = -1;
		goto Error;
	}
	
	nullChk(data = malloc(item->size + 21));	// allocate enough space for twenty multi-values
	//memcpy(data, item->value, item->size);
	for (i=0, j=0; i < item->size; i++) {
		if (item->value[i] == '\0') {
			data[j++] = ';';
			data[j++] = ' ';
		} else {
			data[j++] = item->value[i];
		}
	}
	data[j] = '\0';
	
	GetCtrlAttribute(panel, control, ATTR_CTRL_STYLE, &style);
	if (style == CTRL_TABLE_LS) {
		GetTableCellValLength(panel, control, tagCell, &len);
		if (len) {
			nullChk(origData = malloc(len+1));
			GetTableCellVal(panel, control, tagCell, origData);
		}
  		if (!firstFile && (len!=strlen(data) || memcmp(origData, data, len)) && conflict) {
			SetCtrlVal(panel, conflict, 1);
		} else {
			errChk(SetTableCellVal(panel, control, tagCell, data));
		}
	} else if (style == CTRL_TREE_LS) {
		errChk(SetTreeCellAttribute(panel, control, index, conflict, ATTR_LABEL_TEXT, data));	// conflict for trees is actually the column to place the value in 
	} else {
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
		if (len) {
			nullChk(origData = malloc(len+1));
			GetCtrlVal(panel, control, origData); 
		}
		if (!firstFile && (len!=strlen(data) || memcmp(origData, data, len))) {
			SetCtrlVal(panel, conflict, 1);
		} else {
			errChk(SetCtrlVal(panel, control, data));
		}
	}
	
	StoreDataVals(panel, control, data, index);
	
	ApeTag_remove_item(tag, key);

Error:
	free(origData);
	free(data);
	return error;
}

int GetAPETagUnhandledFields(struct ApeTag *tag, int index) 
{
    assert(tag != NULL);

    if (ApeTag_iter_items(tag, ApeTag_iter_handle_fields, NULL) < 0) {
       ErrorPrintf("Error getting items: %s", ApeTag_error(tag));
    }
	
	return 0;
}

int ApeTag_iter_handle_fields(struct ApeTag *tag, struct ApeItem *item, void *data) 
{
	ApeItem_handle_field(item);
	return 0;	// ApeItem_handle_field does not return an error code
}

void ApeItem_handle_field(struct ApeItem *item)
{
	int numItems, found=-1, showRG;
	int error = 0;
	char *string = NULL;

	GetCtrlVal(configHandle, OPTIONS_SHOWREPLAYGAIN, &showRG);
	if ((showRG || (stricmp(item->key, "replaygain_album_gain") && stricmp(item->key, "replaygain_track_gain") &&
					stricmp(item->key, "replaygain_album_peak") && stricmp(item->key, "replaygain_track_peak"))) &&
	   				stricmp(item->key, "musicbrainz_trackid")) {
	
		nullChk(string = malloc(sizeof(char) * item->size + 1));
		memcpy(string, item->value, item->size);
		string[item->size] = '\0';
	
		GetNumListItems(tab3Handle, TAB3_EXTENDEDTAGS, &numItems);
		if (numItems) {
			GetTreeItemFromLabel(tab3Handle, TAB3_EXTENDEDTAGS, VAL_ALL, 0, 0, VAL_NEXT_PLUS_SELF, 0, item->key, &found);
		}
		if (found==-1) {
			InsertTreeItem(tab3Handle, TAB3_EXTENDEDTAGS, VAL_SIBLING, 0, VAL_LAST, item->key, NULL, NULL, numItems);
			SetTreeItemAttribute (tab3Handle, TAB3_EXTENDEDTAGS, numItems, ATTR_MARK_STATE, 1);

			SetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, numItems, kUnhandledTreeColValue, ATTR_LABEL_TEXT, string);
			SetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, numItems, kUnhandledTreeColOrigValue, ATTR_LABEL_TEXT, string);
			SetCtrlVal(panelHandle, PANEL_TABVALS2, 1);
		}
	}
Error:
	free(string);
	return;
}

/****************************************************************/

int UpdateAPETagItem(struct ApeTag *tag, char *key, int panel, int control, int updateCtrl, char **dataPtr, int index, int useTagData)
{
	int update, len, error;
	int	style, count = 0, i, j;
	char *data = NULL, *ptr;
    struct ApeItem *item = NULL;
    
	GetCtrlVal(panel, updateCtrl, &update);
	if (update == TRUE) {
		GetCtrlAttribute(panel, control, ATTR_CTRL_STYLE, &style);
		if (style == CTRL_TABLE_LS) {
			GetTableCellValLength(panel, control, tagCell, &len);
			nullChk(data = calloc(len+1, sizeof(char)));
			errChk(GetTableCellVal(panel, control, tagCell, data));
		} else if (style == CTRL_TREE_LS) {
			GetTreeCellAttribute (panel, control, index, (int)dataPtr, ATTR_LABEL_TEXT_LENGTH, &len);
			data = calloc(len+1, sizeof(char));
			errChk(GetTreeCellAttribute(panel, control, index, (int)dataPtr, ATTR_LABEL_TEXT, data));	// dataPtr for trees is actually the column to retrieve the value from 
		} else {
			if (!useTagData) {
				GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
				nullChk(data = calloc(len+1, sizeof(char)));
				errChk(GetCtrlVal(panel, control, data));
			} else {
				len = strlen(dataPtr[index]);
				nullChk(data = calloc(len + 1, sizeof(char)));
				strcpy(data, dataPtr[index]);
			}
		}
		if (len) {
			ptr = data;
			while (ptr = strstr(ptr, "; ")) {
				count++;
				ptr += 2; // advance past "; "
			}
			
			nullChk(item = malloc(sizeof(struct ApeItem)));
			item->size = strlen(data) - count;
			item->flags = 0;
			item->key = NULL;
			item->value = NULL;
		
			nullChk(item->key = malloc(strlen(key)+1));
			nullChk(item->value = malloc(item->size));
			strcpy(item->key, key);
			for (i=0, j=0; i<strlen(data); i++) {
				if (data[i] == ';' && data[i+1] == ' ') {
					item->value[j++] = '\0';
					i++;	// skip space
				} else {
					item->value[j++] = data[i];
				}
			}
		
			//ErrorPrintf("%s - %s", dataPtr[index], data);
			ApeTag_replace_item(tag, item);
		} else {
			ApeTag_remove_item(tag, key);	// empty string means delete
		}
	
	}
	
Error:
	if (data) {
		free(data);
	}
	return 0;
}


int SetAPEv2Tag(int panel, char *filename, char *newname, int index)
{
	int error;
	struct ApeTag *tag = NULL;
	
	error = ApeInfo_open_tag(filename, &tag, "rb+");
	
	UpdateAPETagItem(tag, "title", panel, PANEL_TREE, PANEL_UPDATETITLE, kTreeColTrackName, index, FALSE);
	UpdateAPETagItem(tag, "album", panel, PANEL_ALBUM, PANEL_UPDATEALBUM, dataHandle.albumPtr, index, FALSE);
	UpdateAPETagItem(tag, "artist", panel, PANEL_ARTIST, PANEL_UPDATEARTIST, dataHandle.artistPtr, index, FALSE);
	UpdateAPETagItem(tag, "track", panel, PANEL_TREE, PANEL_UPDATETITLE, kTreeColTrackNum, index, FALSE);
	UpdateAPETagItem(tag, "year", tab1Handle, TAB1_YEAR, TAB1_UPDATEYEAR, dataHandle.yearPtr, index, FALSE);
	UpdateAPETagItem(tag, "album artist", tab1Handle, TAB1_ALBUMARTIST, TAB1_UPDATEALBUMARTIST, dataHandle.albumArtistPtr, index, FALSE);
	UpdateAPETagItem(tag, "albumsortorder", tab1Handle, TAB1_ALBUMSORTORDER, TAB1_UPDATEALBUMSORT, dataHandle.albSortOrderPtr, index, FALSE);
	UpdateAPETagItem(tag, "artistcountry", tab1Handle, TAB1_COUNTRY, TAB1_UPDATECOUNTRY, dataHandle.countryPtr, index, FALSE);
	UpdateAPETagItem(tag, "artistsortorder", tab1Handle, TAB1_PERFORMERSORTORDER, TAB1_UPDATEPERFSORT, dataHandle.perfSortOrderPtr, index, FALSE);
	UpdateAPETagItem(tag, "comment", tab1Handle, TAB1_COMMENT, TAB1_UPDATECOMMENT, dataHandle.commentPtr, index, FALSE);
	UpdateAPETagItem(tag, "composer", tab1Handle, TAB1_COMPOSER, TAB1_UPDATECOMPOSER, dataHandle.composerPtr, index, FALSE);
	UpdateAPETagItem(tag, "conductor", tab1Handle, TAB1_ARTISTFILTER, TAB1_UPDATEARTISTFILTER, dataHandle.artistFilterPtr, index, FALSE);
	UpdateAPETagItem(tag, "copyright", tab2Handle, TAB2_COPYRIGHT, TAB2_UPDATECOPYRIGHT, dataHandle.copyrightPtr, index, FALSE);
	UpdateAPETagItem(tag, "disc", tab1Handle, TAB1_DISCNUM, TAB1_UPDATEDISCNUM, dataHandle.discPtr, index, gUseMetaDataDiscVal);
	UpdateAPETagItem(tag, "edition", tab1Handle, TAB1_EDITION, TAB1_UPDATEEDITION, dataHandle.editionPtr, index, FALSE);
	UpdateAPETagItem(tag, "encoded", tab2Handle, TAB2_ENCODED, TAB2_UPDATEENCODED, dataHandle.encodedPtr, index, FALSE);
	UpdateAPETagItem(tag, "genre", tab1Handle, TAB1_GENRE, TAB1_UPDATEGENRE, dataHandle.genrePtr, index, FALSE);
	UpdateAPETagItem(tag, "musicbrainz_artistid", tab3Handle, TAB3_ARTISTMBID, TAB3_UPDATEMBID, NULL, index, FALSE);
	UpdateAPETagItem(tag, "musicbrainz_releasegroupid", tab3Handle, TAB3_REID, TAB3_UPDATEREID, NULL, index, FALSE);
	UpdateAPETagItem(tag, "original artist", tab2Handle, TAB2_ORIGARTIST, TAB2_UPDATEORIGARTIST, dataHandle.origArtistPtr, index, FALSE);
	UpdateAPETagItem(tag, "publisher", tab1Handle, TAB1_PUBLISHER, TAB1_UPDATEPUBLISHER, dataHandle.publisherPtr, index, FALSE);
	UpdateAPETagItem(tag, "releasetype", tab1Handle, TAB1_RELTYPE, TAB1_UPDATERELTYPE, dataHandle.relTypePtr, index, FALSE);
	UpdateAPETagItem(tag, "url", tab2Handle, TAB2_URL, TAB2_UPDATEURL, dataHandle.urlPtr, index, FALSE);
	
	SetAPETagUnhandledFields(tag, index);
	
	ApeTag_update(tag);
	
	ApeTag_close(tag);
	
	return 0;
}

int SetAPETagUnhandledFields(struct ApeTag *tag, int index)
{
	int	i, numItems, checked, len;
	char *key = NULL;
	
	GetNumListItems(tab3Handle, TAB3_EXTENDEDTAGS, &numItems);
	for (i=0;i<numItems;i++) {
		GetTreeItemAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, ATTR_MARK_STATE, &checked);
		if (!checked) {
			// remove this field!
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColFieldName, ATTR_LABEL_TEXT_LENGTH, &len);
			key = malloc(sizeof(char) * len + 1);
			GetTreeCellAttribute(tab3Handle, TAB3_EXTENDEDTAGS, i, kUnhandledTreeColFieldName, ATTR_LABEL_TEXT, key);
			ApeTag_remove_item(tag, key);				 
			if (key) {
				free(key);
			}
			key = NULL;
		}
	}
		
	return 1;
}
