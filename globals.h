//==============================================================================
//
// Title:		globals.h
// Purpose:		A short description of the interface.
//
// Created on:	1/28/2016 at 6:22:37 PM by Kevin.
// Copyright:	Subversive Software. All Rights Reserved.
//
//==============================================================================

#ifndef __globals_H__
#define __globals_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// Constants

#define kNumPanelControls		2
#define kNumTab1Controls		14
#define kNumTab2Controls		5
#define kNumTab3Controls		2
		
#define false	FALSE
#define true	TRUE

//==============================================================================
// Types

//==============================================================================
// External variables

extern const Point tagCell;	// combobox cell of the genre table

extern int panelHandle;
extern int tab1Handle;
extern int tab2Handle;
extern int tab3Handle;
extern int numFiles;
extern int firstFile;

//==============================================================================
// Global variables

int gUseMetaDataDiscVal;		// when saving disk vals, should we use the meta data?
int gUseMetaDataDiscSubtitleVal;// when saving disk subtitle vals, should we use the meta data?
int gUseMetaArtistFilter;		// when saving artist filter vals, should we use the meta data?

//==============================================================================
// Global functions

//int Declare_Your_Functions_Here (int x);
void UnescapeQuotes(char *string);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __globals_H__ */
