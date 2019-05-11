#include "stdafx.h"
#include "CaptureDlg.h"
#include "Resource.h"
#include "MemoryDC.h"

#include <algorithm>

#undef min
#undef max

BEGIN_MESSAGE_MAP(CaptureDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CaptureDlg::Control, CWnd)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

CaptureDlg::CaptureDlg(Synth::Player::AudioBuffer&& buffer, CWnd* pParent) : CDialogEx(IDD_CAPTURE, pParent), _buffer(std::move(buffer))
{
}

void CaptureDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (_control.GetSafeHwnd())
	{
		CRect client;
		GetClientRect(client);

		CRect rWaveform = GetChildRect(IDC_WAVEFORM);
		rWaveform.BottomRight() = client.BottomRight() - _waveformOffset;
		_control.MoveWindow(&rWaveform);

		CRect rCancel = GetChildRect(IDCANCEL);
		rCancel.OffsetRect(-rCancel.TopLeft() - rCancel.Size() + client.BottomRight() - _closeOffset);
		GetDlgItem(IDCANCEL)->MoveWindow(&rCancel);

		UpdateScrollbar();
	}
}

BOOL CaptureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect client;
	GetClientRect(client);

	CRect rWaveform = GetChildRect(IDC_WAVEFORM);
	GetDlgItem(IDC_WAVEFORM)->DestroyWindow();
	_control.Create(::AfxRegisterWndClass(0), L"", WS_VISIBLE | WS_HSCROLL, rWaveform, this, IDC_WAVEFORM);

	_waveformOffset = client.BottomRight() - rWaveform.BottomRight();
	_closeOffset = client.BottomRight() - GetChildRect(IDCANCEL).BottomRight();

	UpdateScrollbar();

	return TRUE;
}

void CaptureDlg::UpdateScrollbar()
{
	CRect rWaveform = GetChildRect(IDC_WAVEFORM);
	SCROLLINFO si{ sizeof si, SIF_RANGE | SIF_PAGE };
	si.nMax = int(_buffer.size()) / _zoom;// -rWaveform.Width();
	si.nPage = rWaveform.Width();
	_control.SetScrollInfo(SB_HORZ, &si);
}

CRect CaptureDlg::GetChildRect(int id) const
{
	CRect r;
	GetDlgItem(id)->GetWindowRect(r);
	ScreenToClient(r);
	return r;
}



void CaptureDlg::Control::OnPaint()
{
	const auto& dlg = static_cast<CaptureDlg&>(*GetParent());
	const auto& buffer = dlg._buffer;

	CRect r;
	GetClientRect(r);

	const int midY = r.Height() / 2;
	const int start = GetScrollPos(SB_HORZ);

	auto getY = [midY](int16_t val)
	{
		return midY - (val >> 8);
	};

	MemoryDC dc(*this);
	dc.FillSolidRect(r, 0x808080);

	dc.MoveTo(0, getY(buffer[start * dlg._zoom]));

	const int count = std::min(r.Width(), int(buffer.size()) / dlg._zoom - start);

	for (int x = 1; x < count; ++x)
		dc.LineTo(x, getY(buffer[(start + x) * dlg._zoom]));
}

void CaptureDlg::Control::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si{ sizeof si, SIF_ALL };
	GetScrollInfo(SB_HORZ, &si);

	switch (nSBCode)
	{
	case SB_LINELEFT:	--si.nPos; break;
	case SB_LINERIGHT:	++si.nPos; break;
	case SB_PAGELEFT:	si.nPos -= si.nPage; break;
	case SB_PAGERIGHT:	si.nPos += si.nPage; break;
	case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
	}

	SetScrollPos(SB_HORZ, si.nPos);

	Invalidate();
}


BOOL CaptureDlg::Control::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	auto& dlg = static_cast<CaptureDlg&>(*GetParent());

	dlg._zoom = std::max(1, dlg._zoom + (zDelta < 0 ? 1 : -1));
	dlg.UpdateScrollbar();
	Invalidate();
	return true;
}
