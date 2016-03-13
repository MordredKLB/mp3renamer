#include "toolbox.h"
#include <ansi_c.h>
#include <userint.h>
#include "titleformatting.h"
#include "ID3v2.h"
#include "mp3renamer.h"
#include "globals.h"
#include <stdarg.h>

int GetMetaDataDiscNum(const int panel, const int control, const int column, const int index, char *retVal)
{
	int len, val=0;
	char *temp, *slash;
	
	//GetCtrlVal(panel, TAB1_UPDATEDISCNUM, &val);
	if (!val && gUseMetaDataDiscVal && dataHandle.discPtr[index] != NULL && isdigit(dataHandle.discPtr[index][0])) {	// if update discnum is unchecked, use the dataHandle value
		temp = malloc(sizeof(char) * strlen(dataHandle.discPtr[index]) + 1);
		sprintf(temp, "%s\0", dataHandle.discPtr[index]);
	}
	else {
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &len);
		temp = malloc(sizeof(char) * (len + 1));
		GetCtrlVal(panel, control, temp);
	}
	slash = strchr(temp, '/');
	if (slash) {
		strncpy(retVal, temp, slash-temp);
		retVal[slash-temp] = '\0';
		}
	else
		sprintf(retVal, "%s\0", temp);
	free(temp);
	return (int)strlen(retVal);
}

int GetMetaDataTotalDiscNum(const int panel, const int control, const int column, const int index, char *retVal)
{
	char *temp, *slash;
	int		strLength, val=0;
	size_t	len;

	//GetCtrlVal(panel, TAB1_UPDATEDISCNUM, &val);
	if (!val && gUseMetaDataDiscVal && dataHandle.discPtr[index] != NULL && isdigit(dataHandle.discPtr[index][0])) {	// if update discnum is unchecked, use the dataHandle value
		temp = malloc(sizeof(char) * strlen(dataHandle.discPtr[index]) + 1);
		sprintf(temp, "%s\0", dataHandle.discPtr[index]);
	}
	else {
		GetCtrlAttribute(panel, control, ATTR_STRING_TEXT_LENGTH, &strLength);
		temp = malloc(sizeof(char) * strLength + 1);
		GetCtrlVal(panel, control, temp);
	}
	slash = strchr(temp, '/');
	if (slash) {
		slash++;
		len = strlen(temp)-(slash-temp);
		strncpy(retVal, slash, len);
		retVal[len] = '\0';
	}
	else
		retVal[0]='\0';
	free(temp);
	return (int)strlen(retVal);
}

int IfFormatFunc(char **retVal, int field1, int numParams, ...)
{
	va_list	parmInfo;
	int		i=0;
	char 	*ifVal, *thenVal, *elseVal, *returnVal, *falseVal = "\0";
	char 	**param;
	
	va_start(parmInfo, numParams);
	param = va_arg(parmInfo, char **);
	ifVal = param[i++];
	if (field1 == 1) { /* $if2 */
		thenVal = ifVal;
		}
	else
		thenVal = param[i++];
	if (numParams > 2)
		elseVal = param[i++];
	else
		elseVal = falseVal;

	if (strlen(ifVal))
		returnVal = thenVal;
	else
		returnVal = elseVal;
	
	*retVal = malloc(sizeof(char) * (strlen(returnVal) + 1));
	sprintf(*retVal, "%s\0", returnVal);
	
	va_end(parmInfo);
	return (int)strlen(*retVal);
}

int IfCompareFormatFunc(char **retVal, int field1, int numParams, ...)
{
	va_list	parmInfo;
	int		val1, val2;
	char 	*val1Str, *val2Str, *thenVal, *elseVal, *returnVal;
	char	**param;
	
	va_start(parmInfo, numParams);

	param = va_arg(parmInfo, char **);
	val1Str = param[0];
	val2Str = param[1];
	thenVal = param[2];
	elseVal = param[3];

	val1 = strtol(val1Str, NULL, 10);
	val2 = strtol(val2Str, NULL, 10);
	if ((field1==1 && val1 > val2) ||
		(field1==0 && val1 == val2))
		returnVal = thenVal;
	else
		returnVal = elseVal;
	*retVal = malloc(sizeof(char) * (strlen(returnVal) + 1));
	sprintf(*retVal, "%s\0", returnVal);
	
	va_end(parmInfo);
	return (int)strlen(*retVal);
}

int RepeatFormatFunc(char **retVal, int field1, int numParams, ...)
{
	va_list	parmInfo;
	char	*string, *numRepeat;
	int		repeat, i;
	char	**param;
	
	va_start(parmInfo, numParams);

	param = va_arg(parmInfo, char **);
	string = param[0];
	numRepeat = param[1];
	
	repeat = strtol(numRepeat, NULL, 10);
	*retVal = malloc(sizeof(char) * (strlen(string) * repeat + 1));
	*retVal[0] = '\0';
	for (i=0;i<repeat;i++)
		strcat(*retVal, string);
	
	va_end(parmInfo);
	return (int)strlen(*retVal);
}

int MathFormatFunc(char **retVal, int field1, int numParams, ...)
{
	va_list	parmInfo;
	int		sum, i;
	char	**param;

	va_start(parmInfo, numParams);
	param = va_arg(parmInfo, char **);
	
	sum = strtol(param[0], NULL, 10);
	for (i=1;i<numParams;i++)
		switch (field1) {
			case 0:	//add
				sum += strtol(param[i], NULL, 10);
				break;
			case 1:	//sub
				sum -= strtol(param[i], NULL, 10);
				break;
			case 2: //mul
				sum *= strtol(param[i], NULL, 10);
				break;
			case 3: //div
				sum /= strtol(param[i], NULL, 10);
				break;
			}
	
	*retVal = malloc(sizeof(char) * 11);
	sprintf(*retVal, "%d\0", sum);

	va_end(parmInfo);
	return (int)strlen(*retVal);
}

int CutFormatFunc(char **retVal, int field1, int numParams, ...)
{
	va_list	parmInfo;
	ssize_t	len = 0;
	char	*str, *copy;
	char	**param;

	va_start(parmInfo, numParams);
	param = va_arg(parmInfo, char **);

	str = param[0];
	copy = malloc(sizeof(char) * strlen(str)+1);
	strcpy(copy, str);
	RestoreSpecialChars(&copy);
	len = strtol(param[1], NULL, 10);
	if (len < 0 || len > strlen(copy))
		len = strlen(copy);
	
	*retVal = calloc(sizeof(char),len+1);
	strncpy(*retVal, copy, len);
	ReplaceSpecialChars(&*retVal);

	va_end(parmInfo);
	free(copy);
	return (int)strlen(*retVal);
}


/* when adding values here update kNumMetaDataVals in config.h */
void SetupMetaDataStruct(void)
{
	addMetaDataVal(&metaData[0], "%artist%", 		panelHandle, 	PANEL_ARTIST, 		-1, 				NULL);
	addMetaDataVal(&metaData[1], "%album%",			panelHandle, 	PANEL_ALBUM, 		-1,					NULL);
	addMetaDataVal(&metaData[2], "%track%", 		panelHandle, 	PANEL_TREE, 		kTreeColTrackNum,	NULL);
	addMetaDataVal(&metaData[3], "%title%", 		panelHandle, 	PANEL_TREE, 		kTreeColTrackName,	NULL);
	addMetaDataVal(&metaData[4], "%album artist%", 	tab1Handle,		TAB1_ALBUMARTIST, 	-1,					NULL);
	addMetaDataVal(&metaData[5], "%tracknumber%",	panelHandle, 	PANEL_TREE, 		kTreeColTrackNum,	NULL);
	addMetaDataVal(&metaData[6], "%discnumber%",	tab1Handle,		TAB1_DISCNUM,		-1,					GetMetaDataDiscNum);
	addMetaDataVal(&metaData[7], "%totaldiscs%",	tab1Handle,		TAB1_DISCNUM,		-1,					GetMetaDataTotalDiscNum);
}

void addMetaDataVal(metaDataValPtr dataPtr, char *name, int panel, int control, int column, 
	int (*dataFunc)(const int panel, const int control, const int column, const int index, char *retVal))
{
	strcpy(dataPtr->name, name);
	dataPtr->panel = panel;
	dataPtr->control = control;
	dataPtr->column = column;
	dataPtr->dataFunc = dataFunc;
}

void SetupFormatFunctions(void)
{
	addFormatFunction(&formatFunc[0], "$if",		0,	IfFormatFunc);
	addFormatFunction(&formatFunc[1], "$if2",		1,	IfFormatFunc);
	addFormatFunction(&formatFunc[2], "$ifequal",	0,	IfCompareFormatFunc);
	addFormatFunction(&formatFunc[3], "$ifgreater",	1,	IfCompareFormatFunc);
	addFormatFunction(&formatFunc[4], "$repeat",	0,	RepeatFormatFunc);
	addFormatFunction(&formatFunc[5], "$add",		0,	MathFormatFunc);
	addFormatFunction(&formatFunc[6], "$sub",		1,	MathFormatFunc);
	addFormatFunction(&formatFunc[7], "$mul",		2,	MathFormatFunc);
	addFormatFunction(&formatFunc[8], "$div",		3,	MathFormatFunc);
	addFormatFunction(&formatFunc[9], "$cut",		0,	CutFormatFunc);
	addFormatFunction(&formatFunc[10],"$left",		0,	CutFormatFunc);
}

void addFormatFunction(formatFuncPtr ffPtr, char *name, int field1Val, int (*func)(char **retVal, int field1, int numParams, ...))
{
	strcpy(ffPtr->name, name);
	ffPtr->func = func;
	ffPtr->field1Val = field1Val;
}

/*******************************************/

int GetFunctionVal(char *ptr, formatFuncVal funcStruct, int index, char **val)
{
	int len=0, numParams=0;
	char *closeParen, *funcCall = NULL, *value;
	char **param = NULL;
	
	closeParen = strchr(ptr, ')');
	if (!closeParen)
		goto Error;	// was not terminated so don't replace anything
	funcCall = malloc(sizeof(char) * 1 + closeParen - ptr);
	strncpy(funcCall, ptr, closeParen - ptr);
	funcCall[closeParen - ptr] = '\0';
	value = strtok(funcCall, ",");
	param = malloc(sizeof(char *) * kMaxParams);
	if (value) {
		param[numParams] = malloc(sizeof(char) * strlen(value) + 1);
		sprintf(param[numParams], "%s\0", value+1);
		numParams++;
		while ((value = strtok(NULL, ",")) && numParams < kMaxParams) {
			param[numParams] = malloc(sizeof(char) * strlen(value) + 1);
			sprintf(param[numParams], "%s\0", value);
			numParams++;
			}
		}
	len = funcStruct.func(val, funcStruct.field1Val, numParams, param);
	

Error:
	while (numParams) {
		numParams--;
		free(param[numParams]);
		}
	free(param);
	if (funcCall)
		free(funcCall);
	return len;
}

/*******************************************/

void ReplaceSpecialChars(char **val)
{
	ReplaceFormatStringToken(&*val, "(", kOpenParenReplacement);
	ReplaceFormatStringToken(&*val, ")", kCloseParenReplacement);
	ReplaceFormatStringToken(&*val, ",", kCommaReplacement);
	ReplaceFormatStringToken(&*val, "'", kSingleQuoteReplacement);
	ReplaceFormatStringToken(&*val, "$", kDollarSignReplacement);
}

void RestoreSpecialChars(char **val)
{
	ReplaceFormatStringToken(&*val, kOpenParenReplacement, "(");
	ReplaceFormatStringToken(&*val, kCloseParenReplacement, ")");
	ReplaceFormatStringToken(&*val, kCommaReplacement, ",");
	ReplaceFormatStringToken(&*val, kSingleQuoteReplacement, "'");
	ReplaceFormatStringToken(&*val, kDollarSignReplacement, "$");
	ReplaceFormatStringToken(&*val, kBlankSpaceReplacement, "");
}

/*******************************************/
int GetMetaDataValue(char **val, metaDataVal dataStruct, int index)
{
	int len=0;

	if (dataStruct.column == -1) {
		GetCtrlAttribute(dataStruct.panel, dataStruct.control, ATTR_STRING_TEXT_LENGTH, &len);
		if (len < 5) len = 5;	// minimum size so that we don't get out of bounds errors when using %totaldiscs% or %discnumber%
	}
	else
		GetTreeCellAttribute(dataStruct.panel, dataStruct.control, index, dataStruct.column, ATTR_LABEL_TEXT_LENGTH, &len);
	*val = malloc(sizeof(char) * (len + 1));
	if (dataStruct.dataFunc)
		len = dataStruct.dataFunc(dataStruct.panel, dataStruct.control, dataStruct.column, index, *val);
	else if (dataStruct.column == -1)
		GetCtrlVal(dataStruct.panel, dataStruct.control, *val);
	else
		GetTreeCellAttribute(dataStruct.panel, dataStruct.control, index, dataStruct.column, ATTR_LABEL_TEXT, *val);
	
	ReplaceSpecialChars(&*val);
	return (int)strlen(*val);
}

void ReplaceFormatStringToken(char **string, char *token, char *val)
{
	char *newStr = NULL;
	char *ptr;
	
	while (ptr = strstr(*string, token)) {
		newStr = malloc(sizeof(char) * (strlen(*string) - strlen(token) + strlen(val) + 1));
		newStr[0] = '\0';
		strncpy(newStr, *string, ptr-*string);
		newStr[ptr-*string] = '\0';
		strcat(newStr, val);
		strcat(newStr, ptr + strlen(token));
		free(*string);
		*string = newStr;
		}
}
