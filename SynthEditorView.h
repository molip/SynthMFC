
// SynthEditorView.h : interface of the CSynthEditorView class
//

#pragma once

#include "synth/libSynth/View.h"

#include "Serial.h"

namespace Synth
{
	namespace UI
	{
		class Controller;
	}
}

class EditCtrlDialog;
class CSynthEditorDoc;

class CSynthEditorView : public CView, public Synth::UI::View
{
protected: // create from serialization only
	CSynthEditorView();
	DECLARE_DYNCREATE(CSynthEditorView)

// Attributes
public:
	CSynthEditorDoc* GetDocument() const;
	Synth::UI::Controller* GetController() const;
	SerialPort& GetSerial() { return _serial; }

	static CSynthEditorView* Instance() { return _instance; }

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC) {}
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// Synth::UI::View overrides.
	virtual void InvalidateAll() override;
	virtual void SetCapture(bool capture) override;
	virtual void StartValueEdit(const Synth::Model::Rect& rect, const std::string& str) override;
	virtual bool UploadData(const Synth::Buffer& buffer) override;
	virtual void SetModified(bool modified) override;

// Implementation
public:
	virtual ~CSynthEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	static CSynthEditorView* _instance;
	std::unique_ptr<EditCtrlDialog> _editCtrlDialog;
	CFont _font, _smallFont;
	SerialPort _serial;

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC*);
	afx_msg void OnFileUpload();
	afx_msg void OnFileTest();
	afx_msg void OnFilePolyTest();
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
	afx_msg void OnEditCancel();
	afx_msg void OnEditCommit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnToolsStopMIDI();
};

#ifndef _DEBUG  // debug version in SynthEditorView.cpp
inline CSynthEditorDoc* CSynthEditorView::GetDocument() const
   { return reinterpret_cast<CSynthEditorDoc*>(m_pDocument); }
#endif

