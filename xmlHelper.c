#include "cvixml.h"
#include "xmlHelper.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/

#define ERR_MSG_SIZE 512
#define ERR_MSG_TITLE	"Error Message"

/*****************************************************************************/

int GetChildElementByIndex(CVIXMLElement *currentElem, int index)
{
	HRESULT error = S_OK;
	CVIXMLElement tempElem = 0;
	
	error = CVIXMLGetChildElementByIndex(*currentElem, index, &tempElem);
	if (error != S_OK)
		goto Error;
	CVIXMLDiscardElement(*currentElem);
	*currentElem = tempElem;

Error:	
	if (error) {
		if (tempElem)
			CVIXMLDiscardElement(tempElem);
		}
	return error;
}

int GetChildElementByTag(CVIXMLElement *currentElem, char *tag)
{
	HRESULT error = S_OK;
	CVIXMLElement tempElem = 0;
	
	hrChk(CVIXMLGetChildElementByTag(*currentElem, tag, &tempElem));
	if (tempElem) {
		CVIXMLDiscardElement(*currentElem);
		*currentElem = tempElem;
	}
	else
		error = S_FALSE;

Error:	
	if (error) {
		if (tempElem)
			CVIXMLDiscardElement(tempElem);
		}
	return error;
}

int GetAttributeByName(CVIXMLElement currentElem, char *attributeName, char *attrVal)
{
	HRESULT error = S_OK;
	CVIXMLAttribute curAttr = 0;

	hrChk(CVIXMLGetAttributeByName(currentElem, attributeName, &curAttr));
	if (curAttr) {
		CVIXMLGetAttributeValue(curAttr, attrVal);
		CVIXMLDiscardAttribute(curAttr);
	} else {
		strcpy(attrVal, "");
		error = S_FALSE;
	}

Error:
	return error;
}

void GetParentElement(CVIXMLElement *currentElem)
{
	HRESULT error = S_OK;
	CVIXMLElement tempElem = 0;
	
	hrChk(CVIXMLGetParentElement(*currentElem, &tempElem));
	CVIXMLDiscardElement(*currentElem);
	*currentElem = tempElem;

Error:	
	if (error) {
		if (tempElem)
			CVIXMLDiscardElement(tempElem);
		}
	return;
}

void DisplayActiveXErrorMessageOnFailure (HRESULT error)
{
    if (FAILED (error))
    {
        char errBuf [ERR_MSG_SIZE];
        CVIXMLGetErrorString (error, errBuf, sizeof (errBuf));
        MessagePopup (ERR_MSG_TITLE, errBuf);
    }

}
