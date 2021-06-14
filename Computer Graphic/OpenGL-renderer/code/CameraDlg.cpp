// CameraDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "CameraDlg.h"
#include "afxdialogex.h"


// CameraDlg dialog

IMPLEMENT_DYNAMIC(CameraDlg, CDialog)

CameraDlg::CameraDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(camera_dlg, pParent)
	, edit_left_text(_T(""))
	, edit_right_text(_T(""))
	, edit_bottom_text(_T(""))
	, edit_top_text(_T(""))
	, edit_near_text(_T(""))
	, edit_far_text(_T(""))
	, orthogonal_checkbox(FALSE)
	, pres_checkbox(FALSE)
	, edit_aspect_text(_T(""))
	, edit_fovy_text(_T(""))
{

}

CameraDlg::~CameraDlg()
{
}

void CameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, edit_left, edit_left_text);
	DDX_Text(pDX, edit_right, edit_right_text);
	DDX_Text(pDX, edit_bottom, edit_bottom_text);
	DDX_Text(pDX, edit_top, edit_top_text);
	DDX_Text(pDX, edit_near, edit_near_text);
	DDX_Text(pDX, edit_far, edit_far_text);

	DDX_Check(pDX, Orthogo, orthogonal_checkbox);
	DDX_Check(pDX, Prespective, pres_checkbox);
	DDX_Text(pDX, edit_acpect, edit_aspect_text);
	DDX_Text(pDX, edit_fovy, edit_fovy_text);
}



BEGIN_MESSAGE_MAP(CameraDlg, CDialog)
	ON_BN_CLICKED(Orthogo, &CameraDlg::OnBnClickedOrthogo)
	ON_BN_CLICKED(Prespective, &CameraDlg::OnBnClickedPrespective)
END_MESSAGE_MAP()


// CameraDlg message handlers


void CameraDlg::OnBnClickedOrthogo()
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(Prespective);
	pBtn->SetCheck(0);

	GetDlgItem(edit_near)->SetWindowTextA("0");
	GetDlgItem(edit_far)->SetWindowTextA("100");
	GetDlgItem(edit_left)->SetWindowTextA("-1.0");
	GetDlgItem(edit_right)->SetWindowTextA("1.0");
	GetDlgItem(edit_bottom)->SetWindowTextA("-1.0");
	GetDlgItem(edit_top)->SetWindowTextA("1.0");

	GetDlgItem(edit_fovy)->SetWindowTextA("");
	GetDlgItem(edit_acpect)->SetWindowTextA("");
}


void CameraDlg::OnBnClickedPrespective()
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(Orthogo);
	pBtn->SetCheck(0);


	GetDlgItem(edit_near)->SetWindowTextA("0.1");
	GetDlgItem(edit_far)->SetWindowTextA("100");
	GetDlgItem(edit_left)->SetWindowTextA("-10.0");
	GetDlgItem(edit_right)->SetWindowTextA("10.0");
	GetDlgItem(edit_bottom)->SetWindowTextA("-10.0");
	GetDlgItem(edit_top)->SetWindowTextA("10.0");

	GetDlgItem(edit_fovy)->SetWindowTextA("160");
	GetDlgItem(edit_acpect)->SetWindowTextA("0.8");	
}
