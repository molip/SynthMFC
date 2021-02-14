#include "stdafx.h"

#include "TabDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(TabDoc, CDocument)

BEGIN_MESSAGE_MAP(TabDoc, CDocument)
END_MESSAGE_MAP()

TabDoc::TabDoc()
{
}

TabDoc::~TabDoc()
{
}

TabDoc* TabDoc::Instance()
{
	if (CFrameWnd* frame = static_cast<CFrameWnd*>(::AfxGetMainWnd()))
		if (CView* view = frame->GetActiveView())
			return static_cast<TabDoc*>(view->GetDocument());
	return nullptr;
}

BOOL TabDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

BOOL TabDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return false;
}

BOOL TabDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	return true;
}

#ifdef _DEBUG
void TabDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void TabDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
