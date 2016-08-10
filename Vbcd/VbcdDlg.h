// VbcdDlg.h : ͷ�ļ�
//

#pragma once

class CEffectDlg;
class CImgProcess;

// CVbcdDlg �Ի���
class CVbcdDlg : public CDialog
{
	// �����ָ��
	static CVbcdDlg* sm_pWnd;

	// ��Ƶ���ھ��
	HWND m_hVideo;
	// ͼ����Ϣ
	BITMAP m_stBitMap;

	// ��־
	struct _ST_STATUS_FLAG
	{
		BYTE btConnect: 1;		// �Ƿ�����
		BYTE btPreview: 1;		// �Ƿ�Ԥ��
		BYTE btDrcShow: 1;		// �Ƿ�ֱ��
	} m_stDataStatus;

	// Ч������
	CEffectDlg* m_pcEffectDlg;
	// ͼ�����̳߳����ʵ��
	CImgProcess* m_pcImgProc;

// ����
public:
	CVbcdDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CVbcdDlg();

// �Ի�������
	enum { IDD = IDD_VBCD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	// ����ͼ��
	HICON m_hIcon;
	// ������
	CToolBar m_wndToolBar;

	// ֡�ɼ��ص�
	friend LRESULT CALLBACK capFrameCallbackProc( HWND hWnd, LPVIDEOHDR lpVHdr );
	// ��ȡͼ����Ϣ
	BOOL SetVideoBmpFormat();
	// ��ʼ��VFW����
	BOOL InitVFWBlock();

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	// ����
	afx_msg void OnTbSet();
	// ����-����
	afx_msg void OnTbConnect();
	// ����-��ƵԴ
	afx_msg void OnTbSourceSet();
	// ����-��Ƶ��ʽ
	afx_msg void OnTbFormatSet();
	// ����-ѹ��
	afx_msg void OnTbCompressSet();

	// ��ͼ-Ԥ��
	afx_msg void OnTbPreview();
	// ��ͼ-��ʾ
	afx_msg void OnTbDirectShow();

	// ϵͳ�˵�������ת
	afx_msg BOOL OnSysCommandEx(UINT uiID);

	// �˳�
	virtual void OnCancel();
};
