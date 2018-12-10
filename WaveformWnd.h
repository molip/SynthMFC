#pragma once

#include <vector>

class WaveformWnd : public CWnd
{
public:
	WaveformWnd();
	virtual ~WaveformWnd();

	bool Create(CWnd* parent);

private:
	using AudioBuffer = std::vector<int16_t>;

	void DrawBuffer(CDC& dc, const AudioBuffer& buffer) const;


	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
};

