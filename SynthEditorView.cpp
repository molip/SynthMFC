
// SynthEditorView.cpp : implementation of the CSynthEditorView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "SynthEditor.h"
#endif

#include "SynthEditorDoc.h"
#include "SynthEditorView.h"

#include "synth/libSynth/Controller.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSynthEditorView

IMPLEMENT_DYNCREATE(CSynthEditorView, CView)

BEGIN_MESSAGE_MAP(CSynthEditorView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CSynthEditorView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_FILE_UPLOAD, &CSynthEditorView::OnFileUpload)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_EDIT_UNDO, &CSynthEditorView::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CSynthEditorView::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CSynthEditorView::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CSynthEditorView::OnUpdateEditRedo)
END_MESSAGE_MAP()

// CSynthEditorView construction/destruction

CSynthEditorView::CSynthEditorView()
{
	_controller = std::make_unique<Synth::UI::Controller>();
}

CSynthEditorView::~CSynthEditorView()
{
}

CSynthEditorView* CSynthEditorView::Instance()
{
	return static_cast<CSynthEditorView*>(static_cast<CFrameWnd*>(::AfxGetMainWnd())->GetActiveView());
}

BOOL CSynthEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CSynthEditorView drawing

void CSynthEditorView::OnDraw(CDC* dc)
{
	auto MakeCRect = [] (const Synth::Model::Rect& rect)
	{
		return CRect(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
	};

	CSynthEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	for (auto& modIkon : _controller->GetModuleIkons())
	{
		auto rect = MakeCRect(modIkon.GetRect());
		dc->Rectangle(rect);
		dc->TextOut(rect.left + 10, rect.top + 10, CString(modIkon.GetName().c_str()));

		for (auto& pin : modIkon.GetInputPins())
		{
			auto pinRect = MakeCRect(pin.rect);
			dc->Rectangle(pinRect);
		}
		for (auto& pin : modIkon.GetOutputPins())
		{
			auto pinRect = MakeCRect(pin.rect);
			dc->Rectangle(pinRect);
		}
	}
}


// CSynthEditorView printing


void CSynthEditorView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CSynthEditorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CSynthEditorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CSynthEditorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CSynthEditorView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSynthEditorView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CSynthEditorView diagnostics

#ifdef _DEBUG
void CSynthEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CSynthEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSynthEditorDoc* CSynthEditorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSynthEditorDoc)));
	return (CSynthEditorDoc*)m_pDocument;
}
#endif //_DEBUG

void CSynthEditorView::OnFileUpload()
{
	GetDocument()->Upload();
}

void CSynthEditorView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (_controller->OnMouseMove(Synth::Model::Point(point.x, point.y)))
		Invalidate();
}

void CSynthEditorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	_controller->OnLButtonDown(Synth::Model::Point(point.x, point.y));
}

void CSynthEditorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	_controller->OnLButtonUp(Synth::Model::Point(point.x, point.y));
}


void CSynthEditorView::OnEditUndo()
{
	_controller->Undo();
	Invalidate();
}

void CSynthEditorView::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(_controller->CanUndo());
}

void CSynthEditorView::OnEditRedo()
{
	_controller->Redo();
	Invalidate();
}

void CSynthEditorView::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(_controller->CanRedo());
}
