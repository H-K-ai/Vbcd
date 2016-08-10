// VbcdDlg.h : 头文件
//

#pragma once

class CEffectDlg;
class CImgProcess;

// CVbcdDlg 对话框
class CVbcdDlg : public CDialog
{
	// 自身的指针
	static CVbcdDlg* sm_pWnd;

	// 视频窗口句柄
	HWND m_hVideo;
	// 图像信息
	BITMAP m_stBitMap;

	// 标志
	struct _ST_STATUS_FLAG
	{
		BYTE btConnect: 1;		// 是否连接
		BYTE btPreview: 1;		// 是否预览
		BYTE btDrcShow: 1;		// 是否直显
	} m_stDataStatus;

	// 效果窗口
	CEffectDlg* m_pcEffectDlg;
	// 图像处理，线程池类的实体
	CImgProcess* m_pcImgProc;

// 构造
public:
	CVbcdDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CVbcdDlg();

// 对话框数据
	enum { IDD = IDD_VBCD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	// 程序图标
	HICON m_hIcon;
	// 工具栏
	CToolBar m_wndToolBar;

	// 帧采集回调
	friend LRESULT CALLBACK capFrameCallbackProc( HWND hWnd, LPVIDEOHDR lpVHdr );
	// 获取图像信息
	BOOL SetVideoBmpFormat();
	// 初始化VFW功能
	BOOL InitVFWBlock();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	// 设置
	afx_msg void OnTbSet();
	// 设置-连接
	afx_msg void OnTbConnect();
	// 设置-视频源
	afx_msg void OnTbSourceSet();
	// 设置-视频格式
	afx_msg void OnTbFormatSet();
	// 设置-压缩
	afx_msg void OnTbCompressSet();

	// 视图-预览
	afx_msg void OnTbPreview();
	// 视图-显示
	afx_msg void OnTbDirectShow();

	// 系统菜单命令中转
	afx_msg BOOL OnSysCommandEx(UINT uiID);

	// 退出
	virtual void OnCancel();
};
