#include "stdafx.h"
#include "WaveformWnd.h"
#include "SynthEditorView.h"

#include "synth/libSynth/Controller.h"
#include "synth/libSynth/Player.h"


BEGIN_MESSAGE_MAP(WaveformWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()

WaveformWnd::WaveformWnd()
{
}

WaveformWnd::~WaveformWnd()
{
}

bool WaveformWnd::Create(CWnd* parent)
{
	if (CWnd::Create(::AfxRegisterWndClass(0), L"", WS_VISIBLE, CRect(0, 0, 0, 0), parent, 100))
	{
		SetTimer(1, 100, nullptr);
		return true;
	}

	return false;
}

void WaveformWnd::OnPaint()
{
	CRect r;
	GetClientRect(r);
	
	CPaintDC dc(this);
	dc.FillSolidRect(r, 0x808080);

	if (auto* view = CSynthEditorView::Instance())
		if (auto* controller = view->GetController())
			if (auto* player = controller->GetPlayer())
				if (auto* buffer = player->GetLastBuffer())
					DrawBuffer(dc, *buffer);
}

void WaveformWnd::DrawBuffer(CDC& dc, const AudioBuffer& buffer) const
{
	int start = 0;
	for (int i = 1; i < buffer.size(); ++i)
		if (buffer[i] < 0 && buffer[i - 1] >= 0)
		{
			start = i;
			break;
		}


	const int midY = 200;

	auto getY = [midY](int16_t val)
	{
		return midY + (val >> 8);
	};

	dc.MoveTo(0, getY(buffer[start]));

	for (int i = start + 1; i < buffer.size(); ++i)
		dc.LineTo(i - start, getY(buffer[i]));
}

void WaveformWnd::OnTimer(UINT_PTR nIDEvent)
{
	Invalidate();
	UpdateWindow();
}
