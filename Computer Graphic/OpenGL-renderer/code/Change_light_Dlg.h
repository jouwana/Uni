#pragma once


// Change_light_Dlg dialog

class Change_light_Dlg : public CDialog
{
	DECLARE_DYNAMIC(Change_light_Dlg)

public:
	Change_light_Dlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~Change_light_Dlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = Change_Light_details };
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
	BOOL on_or_off;
	afx_msg void OnBnClickedsrcchexkboxc();
	afx_msg void OnBnClickedPrlSrcChexkboxc();
	afx_msg void OnBnClickedUseDefaultchek();
	CString light_number;
	afx_msg void OnBnClickedOk();
};
