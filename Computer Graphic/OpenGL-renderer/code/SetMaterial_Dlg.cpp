// SetMaterial_Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "CG_skel_w_MFC.h"
#include "SetMaterial_Dlg.h"
#include "afxdialogex.h"


// SetMaterial_Dlg dialog

IMPLEMENT_DYNAMIC(SetMaterial_Dlg, CDialog)

SetMaterial_Dlg::SetMaterial_Dlg(CWnd* pParent /*=nullptr*/)
	: CDialog(set_material_dlg, pParent)
	, Emmisive_val(_T(""))
	, diffuse_value(_T(""))
	, specular_val(_T(""))
	, use_default_check(TRUE)
	, self_lighting(FALSE)
	, non_uniform(FALSE)
{
	//GetDlgItem(emmisive_edittext)->SetWindowTextA("0.5");
	//GetDlgItem(diffuse_edittext)->SetWindowTextA("0.5");
	//GetDlgItem(specular_edittext)->SetWindowTextA("0.5");
}

SetMaterial_Dlg::~SetMaterial_Dlg()
{
}

void SetMaterial_Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, emmisive_edittext, Emmisive_val);
	DDX_Text(pDX, diffuse_edittext, diffuse_value);
	DDX_Text(pDX, specular_edittext, specular_val);
	DDX_Check(pDX, default_checkbox, use_default_check);
	DDX_Check(pDX, use_self_light_checkBox, self_lighting);
	DDX_Check(pDX, nonUnif_Check, non_uniform);
}


BEGIN_MESSAGE_MAP(SetMaterial_Dlg, CDialog)
	ON_EN_CHANGE(emmisive_edittext, &SetMaterial_Dlg::OnEnChangeedittext)
	/*ON_BN_CLICKED(default_checkbox, &SetMaterial_Dlg::OnBnClickedcheckbox)*/
END_MESSAGE_MAP()


// SetMaterial_Dlg message handlers


//void SetMaterial_Dlg::OnEnChangeedittext()
//{
//	// TODO:  If this is a RICHEDIT control, the control will not
//	// send this notification unless you override the CDialog::OnInitDialog()
//	// function and call CRichEditCtrl().SetEventMask()
//	// with the ENM_CHANGE flag ORed into the mask.
//
//	// TODO:  Add your control notification handler code here
//
//	//CButton* pBtn = (CButton*)GetDlgItem(default_checkbox);
//	//pBtn->SetCheck(0);
//
//}


void SetMaterial_Dlg::OnEnChangeedittext()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(default_checkbox);
	pBtn->SetCheck(0);
}


//void SetMaterial_Dlg::OnBnClickedcheckbox()
//{
//	// TODO: Add your control notification handler code here
//	if (use_default_check == true)
//	{
//		GetDlgItem(emmisive_edittext)->SetWindowTextA("");
//		GetDlgItem(diffuse_edittext)->SetWindowTextA("");
//		GetDlgItem(specular_edittext)->SetWindowTextA("");
//	}
//}
