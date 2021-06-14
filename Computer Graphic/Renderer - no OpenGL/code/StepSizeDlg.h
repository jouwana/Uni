#pragma once


// StepSizeDlg dialog

class StepSizeDlg : public CDialog
{
	DECLARE_DYNAMIC(StepSizeDlg)

public:
	StepSizeDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~StepSizeDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString edit_stepSize_text;
	afx_msg void OnBnClickedButton();
	afx_msg void OnBnClickedOk();
};
