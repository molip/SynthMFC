
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
#include "synth/libKernel/Debug.h"

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
	CPen blackPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen bluePen(PS_SOLID, 1, RGB(0, 0, 255));
	CPen redPen(PS_SOLID, 1, RGB(255, 0, 0));

	CFont font;
	font.CreateStockObject(DEFAULT_GUI_FONT);

	LOGFONT lf;
	font.GetLogFont(&lf);
	lf.lfHeight = long(lf.lfHeight * 0.7);

	CFont smallFont;
	smallFont.CreateFontIndirect(&lf);

	auto MakeCRect = [] (const Synth::Model::Rect& rect)
	{
		return CRect(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
	};

	auto GetPen = [&] (Synth::UI::Colour colour) -> CPen&
	{
		switch(colour)
		{
			case Synth::UI::Colour::Red: return redPen;
			case Synth::UI::Colour::Blue: return bluePen;
			default: KERNEL_ASSERT(false);
		}
		return blackPen;
	};

	auto DrawPin = [&] (const auto& pin)
	{
		dc->SelectObject(&GetPen(pin.colour));
		auto pinRect = MakeCRect(pin.rect);
		dc->Rectangle(pinRect);
		dc->DrawText(CString(pin.name.c_str()), pinRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	};

	CSynthEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	for (auto& modIkon : _controller->GetModuleIkons())
	{
		dc->SelectObject(&smallFont);

		for (auto& pin : modIkon.GetInputPins())
			DrawPin(pin);

		for (auto& pin : modIkon.GetOutputPins())
			DrawPin(pin);

		dc->SelectObject(&font);
		dc->SelectObject(&GetPen(modIkon.GetColour()));
		auto rect = MakeCRect(modIkon.GetRect());
		dc->Rectangle(rect);
		dc->DrawText(CString(modIkon.GetName().c_str()), rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	dc->SelectStockObject(BLACK_PEN);
	dc->SelectStockObject(DEFAULT_GUI_FONT);
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
