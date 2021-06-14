#pragma once
#include "afxcolorbutton.h"


// AddLightDlg dialog

class AddLightDlg : public CDialog
{
	DECLARE_DYNAMIC(AddLightDlg)

public:
	AddLightDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~AddLightDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = Add_Light_source };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString x_pos;
	CString y_pos;
	CString z_pos;
	CString x_dir;
	CString y_dir;
	CString z_dir;
	BOOL pnt_src;
	BOOL prl_src;
	BOOL use_default;
	COLORREF color_picker_val;
	
	afx_msg void OnBnClickedSrcPointchexkbox();
	afx_msg void OnBnClickedParallelSrcchexkbox();
	afx_msg void OnBnClickedUseDefaultchek();
};
