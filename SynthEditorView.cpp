
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
#include "Serial.h"

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
	ON_COMMAND_RANGE(ID_INSERT_MIDI, ID_INSERT_TARGET, &CSynthEditorView::OnInsertModule)
	ON_COMMAND(ID_EDIT_DELETE, &CSynthEditorView::OnDeleteModule)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &CSynthEditorView::OnUpdateDeleteModule)
	ON_COMMAND(ID_TOOLS_UPLOADMIDIFILE, &CSynthEditorView::OnToolsUploadMIDIFile)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// CSynthEditorView construction/destruction

CSynthEditorView::CSynthEditorView()
{
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

	auto MakeCPoint = [] (const Synth::Model::Point& point)
	{
		return CPoint(point.x, point.y);
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

	auto DrawPin = [&] (const Synth::UI::ModuleIkon::Pin& pin)
	{
		dc->SelectObject(&GetPen(pin.colour));
		auto rect = MakeCRect(pin.connectionRect);
		int base = pin.isOutput ? rect.left : rect.right;
		dc->MoveTo(base, rect.top);
		dc->LineTo(MakeCPoint(pin.GetConnectionPoint()));
		dc->LineTo(base, rect.bottom);

		CRect labelRect = MakeCRect(pin.labelRect);
		labelRect.left += 2;
		dc->DrawText(CString(pin.name.c_str()), labelRect, DT_VCENTER | DT_SINGLELINE);

		if (pin.showValue)
		{
			dc->SelectStockObject(BLACK_PEN);
			CRect valueRect = MakeCRect(pin.valueRect);
			dc->Rectangle(valueRect);
			valueRect.right -= 2;
			dc->DrawText(CString(pin.value.c_str()), valueRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}
	};

	CSynthEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	for (auto& modIkon : GetController()->GetModuleIkons())
	{
		dc->SelectObject(&font);
		dc->SelectObject(&GetPen(modIkon.GetColour()));
		auto rect = MakeCRect(modIkon.GetRect());
		dc->Rectangle(rect);

		UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
		CString str(modIkon.GetName().c_str());
		auto labelRect = MakeCRect(modIkon.GetLabelRect());
		dc->DrawText(str, labelRect, format);

		if (modIkon.IsSelected())
		{
			dc->DrawText(str, labelRect, format | DT_CALCRECT);
			
			labelRect.OffsetRect((rect.Width() - labelRect.Width()) / 2, 0); // Why is textRect not centred?

			dc->SelectStockObject(BLACK_PEN);
			dc->MoveTo(labelRect.left, labelRect.bottom);
			dc->LineTo(labelRect.right, labelRect.bottom);
		}

		dc->SelectObject(&smallFont);

		for (auto& pin : modIkon.GetInputPins())
			DrawPin(pin);

		for (auto& pin : modIkon.GetOutputPins())
			DrawPin(pin);
	}

	dc->SelectStockObject(BLACK_PEN);
	dc->SelectStockObject(DEFAULT_GUI_FONT);

	auto DrawConnection = [&](const Synth::UI::Controller::Connection& conn)
	{
		CPoint points[3];
		points[0] = MakeCPoint(conn.first);
		points[1] = MakeCPoint(conn.second);
		points[2] = MakeCPoint(conn.second);

		points[0].x -= 100;
		points[1].x += 100;

		dc->MoveTo(MakeCPoint(conn.first));
		dc->PolyBezierTo(points, 3);
	};

	if (const auto& conn = GetController()->GetLiveConnection())
		DrawConnection(*conn);

	for (const auto& conn : GetController()->GetConnections())
		DrawConnection(conn);
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

void CSynthEditorView::InvalidateAll()
{
	Invalidate();
}

void CSynthEditorView::SetCapture(bool capture)
{
	if (capture)
		CView::SetCapture();
	else if (this == CWnd::GetCapture())
		ReleaseCapture();
}

void CSynthEditorView::CancelValueEdit()
{
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

Synth::UI::Controller* CSynthEditorView::GetController() const
{
	return m_pDocument ? &GetDocument()->GetController() : nullptr;
}

void CSynthEditorView::OnFileUpload()
{
	SerialPort serial;
	if (serial.Open())
	{
		auto buffer = GetController()->Export();
		serial.Write(&buffer->front(), (DWORD)buffer->size());
		serial.Close();
	}
}

void CSynthEditorView::OnInsertModule(UINT id)
{
	int index = id - ID_INSERT_MIDI;
	const auto types = { "midi", "envl", "oscl", "pmix", "trgt" };
	if (index >= 0 && index < types.size())
	{
		GetController()->InsertModule(*(types.begin() + index));
	}
}

void CSynthEditorView::OnDeleteModule()
{
	GetController()->DeleteModule();
}

void CSynthEditorView::OnUpdateDeleteModule(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetController()->CanDeleteModule());
}

void CSynthEditorView::OnMouseMove(UINT nFlags, CPoint point)
{
	GetController()->OnMouseMove(Synth::Model::Point(point.x, point.y));
}

void CSynthEditorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	GetController()->OnLButtonDown(Synth::Model::Point(point.x, point.y));
}

void CSynthEditorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	GetController()->OnLButtonUp(Synth::Model::Point(point.x, point.y));
}

void CSynthEditorView::OnEditUndo()
{
	GetController()->Undo();
}

void CSynthEditorView::OnUpdateEditUndo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetController()->CanUndo());
}

void CSynthEditorView::OnEditRedo()
{
	GetController()->Redo();
}

void CSynthEditorView::OnUpdateEditRedo(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetController()->CanRedo());
}

void CSynthEditorView::OnToolsUploadMIDIFile()
{
	CFileDialog dlg(true, nullptr, nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"MIDI files (*.mid)|*.mid|All Files (*.*)|*.*||", ::AfxGetMainWnd());
	if (dlg.DoModal() == IDOK)
	{
		try
		{
			auto buffer = GetController()->ExportMIDIFile(dlg.GetPathName().GetBuffer());
			SerialPort serial;
			if (serial.Open())
				serial.Write(&buffer->front(), (DWORD)buffer->size());
		}
		catch (Synth::Exception& e)
		{
			::AfxMessageBox(CString(e.what()), MB_ICONWARNING);
		}
	}
}


int CSynthEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	GetDocument()->GetController().SetView(this);

	return 0;
}
