#pragma once


// SetMaterial_Dlg dialog

class SetMaterial_Dlg : public CDialog
{
	DECLARE_DYNAMIC(SetMaterial_Dlg)

public:
	SetMaterial_Dlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~SetMaterial_Dlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = set_material_dlg };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString Emmisive_val;
	CString diffuse_value;
	CString specular_val;
	BOOL use_default_check;

	afx_msg void OnEnChangeedittext();
	/*afx_msg void OnBnClickedcheckbox();*/
	BOOL self_lighting;
	BOOL non_uniform;
};
