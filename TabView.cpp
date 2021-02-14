#include "stdafx.h"

#include "TabView.h"
#include "SynthEditorDoc.h"
#include "SynthEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(TabView, CTabView)

BEGIN_MESSAGE_MAP(TabView, CTabView)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
END_MESSAGE_MAP()

TabView::TabView()
{
}

TabView::~TabView()
{
}

BOOL TabView::PreCreateWindow(CREATESTRUCT& cs)
{
	return __super::PreCreateWindow(cs);
}

int TabView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	AddView(RUNTIME_CLASS(CSynthEditorView), _T("View"), 100);

	return 0;
}

BOOL TabView::OnEraseBkgnd(CDC*)
{
	return true;
}

#ifdef _DEBUG
void TabView::AssertValid() const
{
	CTabView::AssertValid();
}

void TabView::Dump(CDumpContext& dc) const
{
	CTabView::Dump(dc);
}

CSynthEditorDoc* TabView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSynthEditorDoc)));
	return (CSynthEditorDoc*)m_pDocument;
}
#endif //_DEBUG
