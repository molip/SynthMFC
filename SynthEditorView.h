
// SynthEditorView.h : interface of the CSynthEditorView class
//

#pragma once

#include "synth/libSynth/View.h"

namespace Synth
{
	namespace UI
	{
		class Controller;
	}
}

class CSynthEditorView : public CView, private Synth::UI::View
{
protected: // create from serialization only
	CSynthEditorView();
	DECLARE_DYNCREATE(CSynthEditorView)

// Attributes
public:
	CSynthEditorDoc* GetDocument() const;
	Synth::UI::Controller* GetController() const;

	static CSynthEditorView* Instance();

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	virtual void InvalidateAll();
	virtual void SetCapture(bool capture);
	virtual void CancelValueEdit();

// Implementation
public:
	virtual ~CSynthEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnFileUpload();
	afx_msg void OnInsertModule(UINT id);
	afx_msg void OnDeleteModule();
	afx_msg void OnUpdateDeleteModule(CCmdUI *pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);
	afx_msg void OnToolsUploadMIDIFile();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG  // debug version in SynthEditorView.cpp
inline CSynthEditorDoc* CSynthEditorView::GetDocument() const
   { return reinterpret_cast<CSynthEditorDoc*>(m_pDocument); }
#endif

