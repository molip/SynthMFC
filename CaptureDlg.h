#pragma once

#include "synth/libSynth/Player.h"

class CaptureDlg : public CDialogEx
{
	friend class Control;
public:
	CaptureDlg(Synth::Player::Capture&& capture, CWnd* pParent = nullptr);

protected:
	class Control : public CWnd
	{
	public:
	private:
		afx_msg void OnPaint();
		afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		DECLARE_MESSAGE_MAP()
	};

	virtual BOOL OnInitDialog();

	CRect GetChildRect(int id) const;
	void UpdateScrollbar();

	Synth::Player::Capture _capture;
	Control _control;
	CSize _closeOffset, _waveformOffset;
	int _zoom = 1;

	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
