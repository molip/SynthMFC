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

CaptureDlg::CaptureDlg(Synth::Player::Capture&& capture, CWnd* pParent) : CDialogEx(IDD_CAPTURE, pParent), _capture(std::move(capture))
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
	si.nMax = int(_capture.audio.size()) / _zoom;
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
	const auto& buffer = dlg._capture.audio;
	const auto& monitors = dlg._capture.monitors;

	ASSERT(buffer.size() == monitors.size());

	CRect r;
	GetClientRect(r);

	const int midY = r.Height() / 2;
	const size_t start = GetScrollPos(SB_HORZ);

	auto getY = [midY](int16_t val)
	{
		return midY - (val >> 8);
	};

	MemoryDC dc(*this);
	dc.FillSolidRect(r, 0x808080);

	const size_t count = std::min(size_t(r.Width()), buffer.size() / dlg._zoom - start);
	const size_t zoom = size_t(dlg._zoom);
	const size_t startX = start * zoom;

	int colourIndex = 1;
	// Assume all monitor samples have the same number of monitors and channels.
	for (size_t iMonitor = 0; iMonitor < monitors.front().size(); ++iMonitor)
	{
		CPen monitorPen(PS_SOLID, 1, RGB(255 * (colourIndex & 1), 255 * ((colourIndex & 2) >> 1), 255 * ((colourIndex & 4) >> 2)));
		for (size_t iChannel = 0; iChannel < monitors.front()[iMonitor].size(); ++iChannel)
		{
			dc.SelectObject(&monitorPen);
			dc.MoveTo(0, getY(int16_t(monitors[startX][iMonitor][iChannel] * 0x7fff)));
			for (int x = 1; x < count; ++x)
				dc.LineTo(x, getY(int16_t(monitors[startX + x * zoom][iMonitor][iChannel] * 0x7fff)));
		}

		colourIndex = colourIndex == 7 ? 1 : colourIndex + 1;
	}

	CPen audioPen(PS_SOLID, 1, 0x000000ul);
	dc.SelectObject(&audioPen);
	dc.MoveTo(0, getY(buffer[startX]));
	for (int x = 1; x < count; ++x)
		dc.LineTo(x, getY(buffer[startX + x * zoom]));

	dc.SelectStockObject(BLACK_PEN);
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
