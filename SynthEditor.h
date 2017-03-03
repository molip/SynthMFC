
// SynthEditor.h : main header file for the SynthEditor application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CSynthEditorApp:
// See SynthEditor.cpp for the implementation of this class
//

class CSynthEditorApp : public CWinAppEx
{
public:
	CSynthEditorApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CSynthEditorApp theApp;
