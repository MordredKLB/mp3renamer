#include "cvixml.h"


int GetChildElementByIndex(CVIXMLElement *currentElem, int index);
int GetChildElementByTag(CVIXMLElement *currentElem, char *tag);
void GetParentElement(CVIXMLElement *currentElem);
void DisplayActiveXErrorMessageOnFailure (HRESULT error);

/*****************************************************************************/
/* Macros and constants                                                      */
/*****************************************************************************/
#define hrChk(f) if (error = (f), FAILED (error)) {Breakpoint(); DisplayActiveXErrorMessageOnFailure (error); goto Error;} else
