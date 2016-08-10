#pragma once


// CEffectDlg 对话框

class CEffectDlg : public CDialog
{
	DECLARE_DYNAMIC(CEffectDlg)

	// 数据操作保护
	CRITICAL_SECTION m_crtPro;
	// 图像信息
	BITMAP m_stBitMap;
	// 图像数据
	LPRGBQUAD m_lpRGB;
	// 边框
	PST_EDGE_PARA m_pstEdgePara;
	// 角点
	VEC_CORNER* m_pvecCorner;

public:

	// 标准构造函数
	CEffectDlg( CWnd* pParent = NULL );
	virtual ~CEffectDlg();

	// 设置图像信息
	inline void SetBitMap( BITMAP& stBitMap )
	{
		EnterCriticalSection( &m_crtPro );
		m_stBitMap = stBitMap;
		LeaveCriticalSection( &m_crtPro );
	};
	// 更新数据
	BOOL RefreshData( PST_PROC_DATA pstProcData );

// 对话框数据
	enum { IDD = IDD_EFFECT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
