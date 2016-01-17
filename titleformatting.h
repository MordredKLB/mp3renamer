/* General Configuration and Ini File functions for MP3Renamer */

#ifndef TITLEFORMATTING_H
#define TITLEFORMATTING_H

#define kNumMetaDataVals	8
#define kNumFormatFuncs		11

#define kMaxParams			10



#define kOpenParenReplacement	"!xOPEN_PARENx!"
#define kCloseParenReplacement	"!xCLOSE_PARENx!"
#define kCommaReplacement		"!xCOMMAx!"
#define kSingleQuoteReplacement	"!xSINGLE_QUOTEx!"
#define kDollarSignReplacement	"!xDOLLAR_SIGNx!"
#define kDoubleCommaReplacement ",!xBLANK_SPACEx!,"
#define kCommaCloseReplacement ",!xBLANK_SPACEx!)"
#define kBlankSpaceReplacement	"!xBLANK_SPACEx!"

typedef struct {
	char 	name[50];
	int		panel;
	int		control;
	int		column;
	int		(*dataFunc)(const int panel, const int control, const int column, const int index, char *retVal);
} metaDataVal, *metaDataValPtr;

typedef struct {
	char	name[50];
	int		field1Val;
	int		(*func)(char **retVal, int field1, int numParams, ...);
} formatFuncVal, *formatFuncPtr;

void SetupMetaDataStruct(void);
void SetupFormatFunctions(void);
void addMetaDataVal(metaDataValPtr dataPtr, char *name, int panel, int control, int column,
	int (*dataFunc)(const int panel, const int control, const int column, const int index, char *retVal));
void addFormatFunction(formatFuncPtr ffPtr, char *name, int field1Val, int (*func)(char **retVal, int field1, int numParams, ...));
int GetMetaDataValue(char **val, metaDataVal dataStr, int index);
int GetFunctionVal(char *ptr, formatFuncVal funcStruct, int index, char **val);

void ReplaceFormatStringToken(char **string, char *token, char *val);
void ReplaceSpecialChars(char **val);
void RestoreSpecialChars(char **val);

metaDataVal		metaData[kNumMetaDataVals];
formatFuncVal	formatFunc[kNumFormatFuncs];

#endif /* TITLEFORMATTING_H */
