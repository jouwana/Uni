#pragma once


// CameraDlg dialog

class CameraDlg : public CDialog
{
	DECLARE_DYNAMIC(CameraDlg)

public:
	CameraDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CameraDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = camera_dlg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString edit_left_text;
	CString edit_right_text;
	CString edit_bottom_text;
	CString edit_top_text;
	CString edit_near_text;
	CString edit_far_text;
	BOOL orthogonal_checkbox;
	BOOL pres_checkbox;
	CString edit_aspect_text;
	CString edit_fovy_text;
	afx_msg void OnBnClickedOrthogo();
	afx_msg void OnBnClickedPrespective();
};
