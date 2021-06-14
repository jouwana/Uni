#pragma once


// SetAmbientDlg dialog

class SetAmbientDlg : public CDialog
{
	DECLARE_DYNAMIC(SetAmbientDlg)

public:
	SetAmbientDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~SetAmbientDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = SetAmbient_color };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	COLORREF choosed_ambient_color;
};
