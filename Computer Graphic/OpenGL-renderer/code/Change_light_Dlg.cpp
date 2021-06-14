// Change_light_Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "Change_light_Dlg.h"
#include "afxdialogex.h"


// Change_light_Dlg dialog

IMPLEMENT_DYNAMIC(Change_light_Dlg, CDialog)

Change_light_Dlg::Change_light_Dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(Change_Light_details, pParent)
	, x_pos(_T(""))
	, y_pos(_T(""))
	, z_pos(_T(""))
	, x_dir(_T(""))
	, y_dir(_T(""))
	, z_dir(_T(""))
	, pnt_src(FALSE)
	, prl_src(FALSE)
	, use_default(TRUE)
	, on_or_off(TRUE)
	, light_number(_T(""))
{

}

Change_light_Dlg::~Change_light_Dlg()
{
}

void Change_light_Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, pos_x_c, x_pos);
	DDX_Text(pDX, pos_y_c, y_pos);
	DDX_Text(pDX, pos_z_c, z_pos);
	DDX_Text(pDX, dir_x_c, x_dir);
	DDX_Text(pDX, dir_y_c, y_dir);
	DDX_Text(pDX, dir_z_C, z_dir);
	DDX_Check(pDX, pnt_src_chexkbox_c, pnt_src);
	DDX_Check(pDX, prl_src_chexkbox_c, prl_src);
	DDX_Check(pDX, default_chek_c, use_default);
	DDX_Check(pDX, on_edit_text_c, on_or_off);
	DDX_Text(pDX, light_number_edittext_c, light_number);
}


BEGIN_MESSAGE_MAP(Change_light_Dlg, CDialog)
	ON_BN_CLICKED(pnt_src_chexkbox_c, &Change_light_Dlg::OnBnClickedsrcchexkboxc)
	ON_BN_CLICKED(prl_src_chexkbox_c, &Change_light_Dlg::OnBnClickedPrlSrcChexkboxc)
	ON_BN_CLICKED(default_chek_c, &Change_light_Dlg::OnBnClickedUseDefaultchek)
	ON_BN_CLICKED(IDOK, &Change_light_Dlg::OnBnClickedOk)
END_MESSAGE_MAP()


// Change_light_Dlg message handlers


void Change_light_Dlg::OnBnClickedsrcchexkboxc()
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(prl_src_chexkbox_c);
	pBtn->SetCheck(0);

	pBtn = (CButton*)GetDlgItem(default_chek_c);
	pBtn->SetCheck(0);

	GetDlgItem(pos_x_c)->SetWindowTextA("100");
	GetDlgItem(pos_y_c)->SetWindowTextA("100");
	GetDlgItem(pos_z_c)->SetWindowTextA("600");

	GetDlgItem(dir_x_c)->SetWindowTextA("");
	GetDlgItem(dir_y_c)->SetWindowTextA("");
	GetDlgItem(dir_z_C)->SetWindowTextA("");

}


void Change_light_Dlg::OnBnClickedPrlSrcChexkboxc()
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(pnt_src_chexkbox_c);
	pBtn->SetCheck(0);
	pBtn = (CButton*)GetDlgItem(default_chek_c);
	pBtn->SetCheck(0);

	GetDlgItem(pos_x_c)->SetWindowTextA("");
	GetDlgItem(pos_y_c)->SetWindowTextA("");
	GetDlgItem(pos_z_c)->SetWindowTextA("");

	GetDlgItem(dir_x_c)->SetWindowTextA("1");
	GetDlgItem(dir_y_c)->SetWindowTextA("1");
	GetDlgItem(dir_z_C)->SetWindowTextA("1");
}


void Change_light_Dlg::OnBnClickedUseDefaultchek()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(pnt_src_chexkbox_c);
	pBtn->SetCheck(0);
	pBtn = (CButton*)GetDlgItem(prl_src_chexkbox_c);
	pBtn->SetCheck(0);

	GetDlgItem(pos_x_c)->SetWindowTextA("");
	GetDlgItem(pos_y_c)->SetWindowTextA("");
	GetDlgItem(pos_z_c)->SetWindowTextA("");

	GetDlgItem(dir_x_c)->SetWindowTextA("");
	GetDlgItem(dir_y_c)->SetWindowTextA("");
	GetDlgItem(dir_z_C)->SetWindowTextA("");
}


void Change_light_Dlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
