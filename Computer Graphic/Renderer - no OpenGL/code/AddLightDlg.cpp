// AddLightDlg.cpp : implementation file
//
#pragma once
#include <sstream>
#include "material_and_light.h"
#include "stdafx.h"
//#include "pch.h"
#include "CG_skel_w_MFC.h"
#include "AddLightDlg.h"
#include "afxdialogex.h"
#include "afxcolorpickerctrl.h"
//#include "../ColorPicker/MainFrm.h"
#include <afxtoolbar.h>
#include <afxdropdowntoolbar.h>
//#include "FODropColorButton.h"
//
//enum { IDD = IDD_DIALOG_COLORPICKER };
//CFODropColorButton m_wndColor3;
//CFODropColorButton m_wndColor2;
//CFODropColorButton m_wndColor1;
//
//CFODropLineTypeButton m_wndLinePicker2;
//CFODropLineTypeButton m_wndLinePicker1;
//CFOHyperLink m_newVersion;
////}}AFX_DATA


// AddLightDlg dialog

IMPLEMENT_DYNAMIC(AddLightDlg, CDialog)

AddLightDlg::AddLightDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(Add_Light_source, pParent)
	, x_pos(_T(""))
	, y_pos(_T(""))
	, z_pos(_T(""))
	, x_dir(_T(""))
	, y_dir(_T(""))
	, z_dir(_T(""))
	, pnt_src(FALSE)
	, prl_src(FALSE)
	, use_default(TRUE)
{

}

AddLightDlg::~AddLightDlg()
{
}

void AddLightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, pos_x, x_pos);
	DDX_Text(pDX, pos_y, y_pos);
	DDX_Text(pDX, pos_z, z_pos);
	DDX_Text(pDX, dir_x, x_dir);
	DDX_Text(pDX, dir_y, y_dir);
	DDX_Text(pDX, dir_z, z_dir);
	DDX_Check(pDX, pnt_src_chexkbox, pnt_src);
	DDX_Check(pDX, prl_src_chexkbox, prl_src);
	DDX_Check(pDX, default_chek, use_default);
}


BEGIN_MESSAGE_MAP(AddLightDlg, CDialog)
	ON_BN_CLICKED(pnt_src_chexkbox, &AddLightDlg::OnBnClickedSrcPointchexkbox)
	ON_BN_CLICKED(prl_src_chexkbox, &AddLightDlg::OnBnClickedParallelSrcchexkbox)
	ON_BN_CLICKED(default_chek, &AddLightDlg::OnBnClickedUseDefaultchek)
	/*ON_EN_CHANGE(light_number_edittext, &AddLightDlg::OnEnChangeLightNumberEdittext)*/
END_MESSAGE_MAP()


void AddLightDlg::OnBnClickedSrcPointchexkbox()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(prl_src_chexkbox);
	pBtn->SetCheck(0);

	pBtn = (CButton*)GetDlgItem(default_chek);
	pBtn->SetCheck(0);

	GetDlgItem(pos_x)->SetWindowTextA("100");
	GetDlgItem(pos_y)->SetWindowTextA("100");
	GetDlgItem(pos_z)->SetWindowTextA("600");

	GetDlgItem(dir_x)->SetWindowTextA(""); 
	GetDlgItem(dir_y)->SetWindowTextA("");
	GetDlgItem(dir_z)->SetWindowTextA("");

}


void AddLightDlg::OnBnClickedParallelSrcchexkbox()
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(pnt_src_chexkbox);
	pBtn->SetCheck(0);
	pBtn = (CButton*)GetDlgItem(default_chek);
	pBtn->SetCheck(0);

	GetDlgItem(pos_x)->SetWindowTextA("");
	GetDlgItem(pos_y)->SetWindowTextA("");
	GetDlgItem(pos_z)->SetWindowTextA("");

	GetDlgItem(dir_x)->SetWindowTextA("1");
	GetDlgItem(dir_y)->SetWindowTextA("1");
	GetDlgItem(dir_z)->SetWindowTextA("1");
}

void AddLightDlg::OnBnClickedUseDefaultchek()
{
	// TODO: Add your control notification handler code here
	CButton* pBtn = (CButton*)GetDlgItem(pnt_src_chexkbox);
	pBtn->SetCheck(0);
	pBtn = (CButton*)GetDlgItem(prl_src_chexkbox);
	pBtn->SetCheck(0);

	GetDlgItem(pos_x)->SetWindowTextA("");
	GetDlgItem(pos_y)->SetWindowTextA("");
	GetDlgItem(pos_z)->SetWindowTextA("");

	GetDlgItem(dir_x)->SetWindowTextA("");
	GetDlgItem(dir_y)->SetWindowTextA("");
	GetDlgItem(dir_z)->SetWindowTextA("");
}



//void AddLightDlg::OnEnChangeLightNumberEdittext()
//{
//	// TODO:  If this is a RICHEDIT control, the control will not
//	// send this notification unless you override the CDialog::OnInitDialog()
//	// function and call CRichEditCtrl().SetEventMask()
//	// with the ENM_CHANGE flag ORed into the mask.
//
//	// TODO:  Add your control notification handler code here
//
//	//Light* active_light = set_and_return_active_light(atoi(light_number_to_set)); // get the appropaiate light
//	//
//	//
//	//bool pntt = active_light->type == PointSource;
//	//bool prll = active_light->type == ParallelLight;
//	//CButton* pBtn = (CButton*)GetDlgItem(pnt_src_chexkbox);
//	//pBtn->SetCheck(pntt);
//	//pBtn = (CButton*)GetDlgItem(prl_src_chexkbox);
//	//pBtn->SetCheck(prll);
//
//
//	//pBtn = (CButton*)GetDlgItem(default_chek);
//	//pBtn->SetCheck(false);
//
//	//float x = active_light->position.x;
//	//CString floatString; 
//	//floatString.Format(_T("%f"), x);
//	//LPCTSTR s = (LPCSTR)(LPCTSTR)floatString;
//	//GetDlgItem(pos_x)->SetWindowTextA(s);
//
//	//x = active_light->position.y;
//	//floatString;
//	//floatString.Format(_T("%f"), x);
//	//s = (LPCSTR)(LPCTSTR)floatString;
//	//GetDlgItem(pos_y)->SetWindowTextA(s);
//
//	//x = active_light->position.z;
//	//floatString;
//	//floatString.Format(_T("%f"), x);
//	//s = (LPCSTR)(LPCTSTR)floatString;
//	//GetDlgItem(pos_z)->SetWindowTextA(s);
//
//	//x = active_light->direction.x;
//	//floatString;
//	//floatString.Format(_T("%f"), x);
//	//s = (LPCSTR)(LPCTSTR)floatString;
//	//GetDlgItem(dir_x)->SetWindowTextA(s);
//
//	//x = active_light->direction.y;
//	//floatString;
//	//floatString.Format(_T("%f"), x);
//	//s = (LPCSTR)(LPCTSTR)floatString;
//	//GetDlgItem(dir_y)->SetWindowTextA(s);
//
//	//x = active_light->direction.z;
//	//floatString;
//	//floatString.Format(_T("%f"), x);
//	//s = (LPCSTR)(LPCTSTR)floatString;
//	//GetDlgItem(dir_z)->SetWindowTextA(s);
//
//}
