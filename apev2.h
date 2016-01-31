//==============================================================================
//
// Title:		apev2.h
// Purpose:		A short description of the interface.
//
// Created on:	1/23/2016 at 6:50:09 PM by Kevin.
// Copyright:	Subversive Software. All Rights Reserved.
//
//==============================================================================

#ifndef __apev2_H__
#define __apev2_H__

#ifdef __cplusplus
    extern "C" {
#endif

//==============================================================================
// Include files

#include "cvidef.h"

//==============================================================================
// Constants

//==============================================================================
// Types

//==============================================================================
// External variables

//==============================================================================
// Global functions

int LoadAPEv2Tag (int panel, char *fileName, int index);
int SetAPEv2Tag(int panel, char *filename, char *newname, int index);
int ApeInfo_process(char *filename, struct ApeTag **tag);
int ApeInfo_open_tag(char *filename, struct ApeTag **tag, char *mode);
void ApeTag_print(struct ApeTag *tag);
void ApeTag_close(struct ApeTag *tag);

#ifdef __cplusplus
    }
#endif

#endif  /* ndef __apev2_H__ */
