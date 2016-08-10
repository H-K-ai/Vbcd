#pragma once


// CEffectDlg �Ի���

class CEffectDlg : public CDialog
{
	DECLARE_DYNAMIC(CEffectDlg)

	// ���ݲ�������
	CRITICAL_SECTION m_crtPro;
	// ͼ����Ϣ
	BITMAP m_stBitMap;
	// ͼ������
	LPRGBQUAD m_lpRGB;
	// �߿�
	PST_EDGE_PARA m_pstEdgePara;
	// �ǵ�
	VEC_CORNER* m_pvecCorner;

public:

	// ��׼���캯��
	CEffectDlg( CWnd* pParent = NULL );
	virtual ~CEffectDlg();

	// ����ͼ����Ϣ
	inline void SetBitMap( BITMAP& stBitMap )
	{
		EnterCriticalSection( &m_crtPro );
		m_stBitMap = stBitMap;
		LeaveCriticalSection( &m_crtPro );
	};
	// ��������
	BOOL RefreshData( PST_PROC_DATA pstProcData );

// �Ի�������
	enum { IDD = IDD_EFFECT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
