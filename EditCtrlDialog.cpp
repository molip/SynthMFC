#include "stdafx.h"
#include "EditCtrlDialog.h"
#include "afxdialogex.h"
#include "Resource.h"
#include "Messages.h"

namespace
{
	enum { CtrlID_Edit = 100 };
}

BEGIN_MESSAGE_MAP(EditCtrlDialog, CDialogEx)
	ON_EN_KILLFOCUS(CtrlID_Edit, OnKillFocus)
END_MESSAGE_MAP()

EditCtrlDialog::EditCtrlDialog(CWnd* owner) : CDialogEx(IDD_BLANK, nullptr), _owner(owner)
{
}

EditCtrlDialog::~EditCtrlDialog()
{
}

void EditCtrlDialog::Create(const CRect& rect, const CString& text, CFont& font)
{
	__super::Create(IDD_BLANK, ::AfxGetMainWnd());
	::AfxGetMainWnd()->SendMessage(WM_NCACTIVATE, TRUE);

	_edit.Create(WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT, CRect(0, 0, rect.Width(), rect.Height()), this, CtrlID_Edit);
	_edit.SetFont(&font);
	_edit.SetWindowText(text);
	_edit.SetFocus();
	_edit.SetSel(0, -1);

	MoveWindow(rect);
	ShowWindow(SW_SHOW);
}

CString EditCtrlDialog::GetText() const
{
	CString text;
	_edit.GetWindowText(text);
	return text;
}

void EditCtrlDialog::OnCancel()
{
	_dead = true;
	_owner->SendMessage(Message::CancelEdit);
}

void EditCtrlDialog::OnOK()
{
	_dead = true;
	_owner->SendMessage(Message::CommitEdit);
}

void EditCtrlDialog::OnKillFocus()
{
	if (!_dead)
		OnOK();
}
