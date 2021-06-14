// SetAmbientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "SetAmbientDlg.h"
#include "afxdialogex.h"


// SetAmbientDlg dialog

IMPLEMENT_DYNAMIC(SetAmbientDlg, CDialog)

SetAmbientDlg::SetAmbientDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(SetAmbient_color, pParent)
{

}

SetAmbientDlg::~SetAmbientDlg()
{
}

void SetAmbientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MFCCOLORBUTTON1, choosed_ambient_color);
}


BEGIN_MESSAGE_MAP(SetAmbientDlg, CDialog)
END_MESSAGE_MAP()


// SetAmbientDlg message handlers

