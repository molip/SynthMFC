
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
#include "Resource.h"
#include "EditCtrlDialog.h"
#include "Messages.h"
#include "MemoryDC.h"
#include "CaptureDlg.h"

#include "synth/libSynth/Player.h"
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
	ON_COMMAND(ID_FILE_POLYTEST, &CSynthEditorView::OnFilePolyTest)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE_VOID(Message::CancelEdit, OnEditCancel)
	ON_MESSAGE_VOID(Message::CommitEdit, OnEditCommit)
	ON_COMMAND(ID_EDIT_UNDO, &CSynthEditorView::OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CSynthEditorView::OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CSynthEditorView::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CSynthEditorView::OnUpdateEditRedo)
	ON_COMMAND_RANGE(ID_INSERT_MIDI, ID_INSERT_SEQUENCE, &CSynthEditorView::OnInsertModule)
	ON_COMMAND(ID_EDIT_DELETE, &CSynthEditorView::OnDeleteModule)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, &CSynthEditorView::OnUpdateDeleteModule)
	ON_COMMAND(ID_TOOLS_UPLOADMIDIFILE, &CSynthEditorView::OnToolsUploadMIDIFile)
	ON_COMMAND(ID_TOOLS_STOPMIDI, &CSynthEditorView::OnToolsStopMIDI)
	ON_COMMAND(ID_TOOLS_CAPTURE, &CSynthEditorView::OnCapture)
	ON_WM_MOUSEWHEEL()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_WM_CREATE()
END_MESSAGE_MAP()

namespace
{
	CRect MakeCRect(const Synth::Model::Rect& rect)
	{
		return CRect(rect.Left(), rect.Top(), rect.Right(), rect.Bottom());
	}

	CPoint MakeCPoint(const Synth::Model::Point& point)
	{
		return CPoint(point.x, point.y);
	}

	Synth::Model::Point MakePoint(const CPoint& point)
	{
		return Synth::Model::Point(point.x, point.y);
	}

	namespace Timer { enum Type { Capture = 1, Monitor }; }
}

CSynthEditorView* CSynthEditorView::_instance;

CSynthEditorView::CSynthEditorView()
{
	KERNEL_ASSERT(!_instance);
	_instance = this;

	_font.CreateStockObject(DEFAULT_GUI_FONT);

	LOGFONT lf;
	_font.GetLogFont(&lf);

	_smallFont.CreateFontIndirect(&lf);
}

CSynthEditorView::~CSynthEditorView()
{
	_instance = nullptr;
}

BOOL CSynthEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

int CSynthEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(::Timer::Monitor, 20, nullptr);

	return 0;
}

// CSynthEditorView drawing

void CSynthEditorView::OnPaint()
{
	MemoryDC memDC(*this);
	memDC.FillSolidRect(memDC.GetRect(), 0xffffff);

	CDC* dc = &memDC;

	CPen blackPen(PS_SOLID, 1, RGB(0, 0, 0));
	CPen bluePen(PS_SOLID, 1, RGB(0, 0, 255));
	CPen redPen(PS_SOLID, 1, RGB(255, 0, 0));

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

		auto DrawValue = [&](const Synth::Model::Rect& rect, const std::string& str)
		{
			dc->SelectStockObject(BLACK_PEN);
			CRect valueRect = MakeCRect(rect);
			dc->Rectangle(valueRect);
			valueRect.right -= 2;
			dc->DrawText(CString(str.c_str()), valueRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		};

		if (pin.showOffset)
			DrawValue(pin.offsetRect, pin.offset);

		if (pin.showScale)
			DrawValue(pin.scaleRect, pin.scale);
	};

	auto DrawField = [&] (const Synth::UI::ModuleIkon::Field& field)
	{
		dc->SelectStockObject(BLACK_PEN);
		CRect fieldRect = MakeCRect(field.rect);
		fieldRect.DeflateRect(2, 0);
		dc->Rectangle(fieldRect);
		dc->DrawText(CString(field.content.c_str()), fieldRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
	};

	if (!GetController())
		return;

	_monitorAreas.clear();

	for (auto& modIkon : GetController()->GetModuleIkons())
	{
		dc->SelectObject(&_font);
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

		dc->SelectObject(&_smallFont);

		for (auto& field : modIkon.GetFields())
			DrawField(field);

		for (auto& pin : modIkon.GetInputPins())
			DrawPin(pin);

		for (auto& pin : modIkon.GetOutputPins())
		{
			DrawPin(pin);
			if (!pin.monitorArea.rect.IsEmpty())
				_monitorAreas.push_back(pin.monitorArea);
		}
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

BOOL CSynthEditorView::OnEraseBkgnd(CDC*)
{
	return true;
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

void CSynthEditorView::StartValueEdit(const Synth::Model::Rect & rect, const std::string& str)
{
	KERNEL_ASSERT(!_editCtrlDialog);

	CRect screenRect = MakeCRect(rect);
	ClientToScreen(screenRect);

	_editCtrlDialog = std::make_unique<EditCtrlDialog>(this);
	_editCtrlDialog->Create(screenRect, CString(str.c_str()), _smallFont);
}

bool CSynthEditorView::UploadData(const Synth::Buffer& buffer)
{
	if (_serial.Open())
	{
		_serial.Write(buffer);
		return true;
	}

	return false;
}

void CSynthEditorView::SetModified(bool modified)
{
	GetDocument()->SetModifiedFlag(modified);
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
	GetController()->Export();
}

void CSynthEditorView::OnFilePolyTest()
{
	GetController()->ExportPolyTest();
}

void CSynthEditorView::OnInsertModule(UINT id)
{
	int index = id - ID_INSERT_MIDI;
	const auto types = { "midi", "envl", "oscl", "pmix", "trgt", "filt", "math", "mixr", "pitc", "lfo", "dely", "arpe", "mult", "knob", "crsh", "sqnc" };
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
	GetController()->OnMouseMove(MakePoint(point));
}

void CSynthEditorView::OnLButtonDown(UINT nFlags, CPoint point)
{
	GetController()->OnLButtonDown(MakePoint(point));
}

void CSynthEditorView::OnLButtonUp(UINT nFlags, CPoint point)
{
	GetController()->OnLButtonUp(MakePoint(point));
}

BOOL CSynthEditorView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	ScreenToClient(&pt);
	GetController()->OnMouseWheel(MakePoint(pt), zDelta < 0, nFlags & MK_CONTROL);
	return true;
}

void CSynthEditorView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((::GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0)
		GetController()->OnKeyDown(nChar);
}

void CSynthEditorView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((::GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0)
		GetController()->OnKeyUp(nChar);
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
			GetController()->ExportMIDIFile(dlg.GetPathName().GetBuffer());
		}
		catch (Synth::Exception& e)
		{
			::AfxMessageBox(CString(e.what()), MB_ICONWARNING);
		}
	}
}

void CSynthEditorView::OnEditCancel()
{
	_editCtrlDialog.reset();
}

void CSynthEditorView::OnEditCommit()
{
	CStringA text(_editCtrlDialog->GetText());
	GetController()->CommitValueEdit(std::string(text));
	_editCtrlDialog.reset();
}

BOOL CSynthEditorView::PreTranslateMessage(MSG* pMsg)
{
	return __super::PreTranslateMessage(pMsg);
}


void CSynthEditorView::OnToolsStopMIDI()
{
	GetController()->StopMIDIFilePlayback();
}

void CSynthEditorView::OnCapture()
{
	auto* player = GetController() ? GetController()->GetPlayer() : nullptr;
	if (!player)
		return;
			
	if (player->IsCapturing())
	{
		player->StopCapture();
		SetTimer(::Timer::Capture, 1000, nullptr);
	}
	else
		player->StartCapture();
}

void CSynthEditorView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == ::Timer::Capture)
	{
		KillTimer(nIDEvent);

		auto* player = GetController() ? GetController()->GetPlayer() : nullptr;
		if (!player)
			return;

		if (player->IsCapturing())
			return;

		CaptureDlg dlg(player->HarvestCapture());
		dlg.DoModal();
	}
	else if (nIDEvent == ::Timer::Monitor)
	{
		CDC* dc = GetDC();
		DrawMonitors(*dc);
		ReleaseDC(dc);
	}
}

void CSynthEditorView::DrawMonitors(CDC& dc) const
{
	for (auto& monitorArea : _monitorAreas)
	{
		auto ikon = GetController()->GetMonitorIkon(monitorArea);
		if (ikon.rect.IsEmpty())
			continue;

		auto rect = MakeCRect(ikon.rect);

		dc.SelectStockObject(WHITE_BRUSH);
		dc.SelectStockObject(BLACK_PEN);
		dc.Rectangle(rect);

		dc.SelectStockObject(BLACK_BRUSH);
		dc.SelectStockObject(NULL_PEN);
		auto r1 = rect;
		for (size_t i = 0; i < ikon.activeWidths.size(); ++i)
		{
			r1.right = r1.left + ikon.activeWidths[i];
			r1.bottom = int(rect.top + (i + 1) * rect.Height() / ikon.activeWidths.size());
			dc.Rectangle(r1);
			r1.top = r1.bottom;
		}
	}
}
