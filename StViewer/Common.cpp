
#include "stdafx.h"
#include "Common.h"


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void OnException(const GenICam::GenericException &e)
{
	//Get the exception contents.
	GenICam::gcstring strSourceFileName(e.GetSourceFileName());
	const unsigned int iSourceLine = e.GetSourceLine();
	GenICam::gcstring strDescription(e.GetDescription());
	
	//Make message string.
	CString strMessage;
	strMessage.Append(GCSTRING_2_LPCTSTR(strSourceFileName));
	strMessage.AppendFormat(TEXT("[%u]\r\n"), iSourceLine);
	strMessage.Append(GCSTRING_2_LPCTSTR(strDescription));

	//Show message box.
	AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);
}
