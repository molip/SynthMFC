#pragma once

class CSynthEditorDoc;

class TabView : public CTabView
{
protected:
	TabView();
	DECLARE_DYNCREATE(TabView)

public:
	CSynthEditorDoc* GetDocument() const;

public:
	virtual void OnDraw(CDC* pDC) {}
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
	virtual ~TabView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC*);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SynthEditorView.cpp
inline CSynthEditorDoc* TabView::GetDocument() const
   { return reinterpret_cast<CSynthEditorDoc*>(m_pDocument); }
#endif

