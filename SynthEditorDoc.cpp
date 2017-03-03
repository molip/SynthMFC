
// SynthEditorDoc.cpp : implementation of the CSynthEditorDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SynthEditor.h"
#endif

#include "SynthEditorDoc.h"
#include "Serial.h"

//#include "libKernel/Serial.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSynthEditorDoc

IMPLEMENT_DYNCREATE(CSynthEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CSynthEditorDoc, CDocument)
END_MESSAGE_MAP()

CSynthEditorDoc::CSynthEditorDoc()
{
}

CSynthEditorDoc::~CSynthEditorDoc()
{
}

CSynthEditorDoc* CSynthEditorDoc::Instance()
{
	if (CFrameWnd* frame = static_cast<CFrameWnd*>(::AfxGetMainWnd()))
		if (CView* view = frame->GetActiveView())
			return static_cast<CSynthEditorDoc*>(view->GetDocument());
	return nullptr;
}

BOOL CSynthEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

BOOL CSynthEditorDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	//Kernel::Serial::SaveClass("C:\\Users\\jxxwh\\test.synth", *m_graph);

	std::wstring s = lpszPathName;
	return false;
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CSynthEditorDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CSynthEditorDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CSynthEditorDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CSynthEditorDoc diagnostics

#ifdef _DEBUG
void CSynthEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSynthEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

void CSynthEditorDoc::Upload() const
{
	SerialPort serial;
	if (serial.Open())
	{
		//auto buffer = m_graph->Export();
		//serial.Write(&buffer[0], (DWORD)buffer.size());
		serial.Close();
	}
}
