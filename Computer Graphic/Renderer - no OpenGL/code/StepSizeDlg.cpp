// StepSizeDlg.cpp : implementation file
//
#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "StepSizeDlg.h"
#include "afxdialogex.h"


// StepSizeDlg dialog

IMPLEMENT_DYNAMIC(StepSizeDlg, CDialog)

StepSizeDlg::StepSizeDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG1, pParent)
	, edit_stepSize_text(_T(""))
{

}

StepSizeDlg::~StepSizeDlg()
{
}

void StepSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, edit_step_size, edit_stepSize_text);
}


BEGIN_MESSAGE_MAP(StepSizeDlg, CDialog)
	ON_BN_CLICKED(OK_Button, &StepSizeDlg::OnBnClickedButton)
	ON_BN_CLICKED(IDOK, &StepSizeDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// StepSizeDlg message handlers


void StepSizeDlg::OnBnClickedButton()
{
	// TODO: Add your control notification handler code here
	EndDialog(0);
}


void StepSizeDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
