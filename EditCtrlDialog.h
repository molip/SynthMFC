#pragma once

class EditCtrlDialog : public CDialogEx
{
public:
	EditCtrlDialog(CWnd* pParent);
	virtual ~EditCtrlDialog();

	void Create(const CRect& rect, const CString& text, CFont& font);

	CString GetText() const;

	virtual void OnCancel() override;
	virtual void OnOK() override;

private:
	CEdit _edit;
	CWnd* _owner = nullptr;
	CString _text;
	bool _dead = false;

	void OnKillFocus();
	DECLARE_MESSAGE_MAP()
};
