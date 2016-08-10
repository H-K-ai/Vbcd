// VbcdDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Vbcd.h"
#include "VbcdDlg.h"
#include "EffectDlg.h"
#include "ImgProcess.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TOOLBAR_HEIGHT 37
//////////////////////////////////////////////////////////////////////////
// 帧采集回调
LRESULT CALLBACK capFrameCallbackProc( HWND hWnd, LPVIDEOHDR lpVHdr )
{
	if( hWnd != CVbcdDlg::sm_pWnd->m_hVideo )
		return S_FALSE;

	// 通知处理程序有新数据
	CVbcdDlg::sm_pWnd->m_pcImgProc->m_lpVHdr = lpVHdr;
	SetEvent( CVbcdDlg::sm_pWnd->m_pcImgProc->m_hHasData );

	return S_OK;
}


// CVbcdDlg 对话框
//////////////////////////////////////////////////////////////////////////
CVbcdDlg* CVbcdDlg::sm_pWnd = NULL;

CVbcdDlg::CVbcdDlg( CWnd* pParent /*=NULL*/ )
	: CDialog( CVbcdDlg::IDD, pParent )
{
	// 标题栏图标
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// 自身的指针
	sm_pWnd = this;
	// 视频窗口
	m_hVideo = NULL;
	// 图像信息初始化
	ZeroMemory( &m_stBitMap, sizeof(m_stBitMap) );
	// 标志
	ZeroMemory( &m_stDataStatus, sizeof(m_stDataStatus) );

	// 效果窗口
	m_pcEffectDlg = new CEffectDlg( this );

	// 图像处理
	m_pcImgProc = new CImgProcess( this, m_pcEffectDlg );
}

// 标准析构函数
CVbcdDlg::~CVbcdDlg()
{
	// 清状态
	ZeroMemory( &m_stDataStatus, sizeof(m_stDataStatus) );

	// 释放效果窗口
	m_pcEffectDlg->DestroyWindow();
	ReleasePoint( m_pcEffectDlg );
	// 释放图像处理，销毁线程池
	ReleasePoint( m_pcImgProc );
}

BOOL CVbcdDlg::SetVideoBmpFormat()
{
	// 获取状态
	CAPSTATUS stCapStatus;
	if( !capGetStatus( m_hVideo, &stCapStatus, sizeof(CAPSTATUS) ) )
		return FALSE;

	// 如果图像信息改变
	if(
		m_stBitMap.bmHeight != stCapStatus.uiImageHeight ||
		m_stBitMap.bmWidth != stCapStatus.uiImageWidth 
		)
	{
		// 获取图像信息
		BITMAPINFO stBitmapInfo;
		capGetVideoFormat( m_hVideo, &stBitmapInfo, sizeof(BITMAPINFO) );
		m_stBitMap.bmType = 0;
		m_stBitMap.bmWidth = stBitmapInfo.bmiHeader.biWidth;
		m_stBitMap.bmHeight = stBitmapInfo.bmiHeader.biHeight;
		m_stBitMap.bmPlanes = stBitmapInfo.bmiHeader.biPlanes;
		m_stBitMap.bmBitsPixel = 32;
		m_stBitMap.bmWidthBytes = m_stBitMap.bmWidth * 4;

		// 设置窗口大小
		SetWindowPos( NULL, 0, 0, m_stBitMap.bmWidth*2 + 35, m_stBitMap.bmHeight + TOOLBAR_HEIGHT + 50, SWP_NOMOVE|SWP_NOZORDER );
		CenterWindow();
		// 设置预览区大小
		GetDlgItem( IDC_VIDEO )->SetWindowPos( NULL, 10, TOOLBAR_HEIGHT+10, m_stBitMap.bmWidth, m_stBitMap.bmHeight, SWP_NOZORDER );
		// 设置效果窗口大小
		m_pcEffectDlg->SetBitMap( m_stBitMap );
		m_pcEffectDlg->SetWindowPos( NULL, m_stBitMap.bmWidth + 15, TOOLBAR_HEIGHT+9, m_stBitMap.bmWidth + 4, m_stBitMap.bmHeight + 2, SWP_NOACTIVATE );
		// 设置 处理图像 信息
		m_pcImgProc->SetBitMap( m_stBitMap );

		// 更新界面
		RedrawWindow( NULL, NULL, RDW_INVALIDATE|RDW_ERASE|RDW_INTERNALPAINT );
	}
	return TRUE;
}

// 初始化VFW相关功能
BOOL CVbcdDlg::InitVFWBlock()
{
	// VFW预览窗口
	if( m_hVideo == NULL )
		m_hVideo = ::capCreateCaptureWindow(
			_T("VideoPreview"),
			WS_CHILDWINDOW|WS_BORDER,
			0, 0, 1, 1,
			m_hWnd, IDC_VIDEO );
	if( m_hVideo == NULL )
		return FALSE;

	// 获取系统能够使用的媒体设备驱动；驱动名、版本
	{
		CAPDRIVERCAPS stCapDvr;
		TCHAR szDeviceName[ 80 ];
		TCHAR szDeviceVersion[ 80 ];
		for ( int wIndex = 0; ; wIndex++ )
		{
			// 一一获取设备信息
			if ( ::capGetDriverDescription ( wIndex, szDeviceName, 80, szDeviceVersion, 80 ) )
			{
				// 连接设备
				if( capDriverConnect( m_hVideo, wIndex ) )
				{
					capDriverGetCaps( m_hVideo, &stCapDvr, sizeof(CAPDRIVERCAPS) );
					if( stCapDvr.fCaptureInitialized )
						// 可用，结束查找
						break;
				}
				else
				{
					CString csInfo = _T("连接设备失败，设备名称：");
					csInfo += szDeviceName;
					MessageBox( csInfo );
				}
			}
			else
			{
				MessageBox( _T("没有找到可用摄像设备！") );
				return FALSE;
			}
		}
	}

	// 捕捉参数
	{
		CAPTUREPARMS stCaptureParms;
		// 获取捕捉参数
		if( !capCaptureGetSetup( m_hVideo, &stCaptureParms, sizeof(CAPTUREPARMS) ) )
			MessageBox( _T("捕捉参数获取失败") );
		// 每帧66667微秒
		stCaptureParms.dwRequestMicroSecPerFrame = 66667;
		// 背景任务
		stCaptureParms.fYield = TRUE;
		stCaptureParms.wNumVideoRequested = 0;
		// 不捕捉音频
		stCaptureParms.fCaptureAudio = FALSE;
		stCaptureParms.wNumAudioRequested = 0;
		stCaptureParms.fAbortLeftMouse = FALSE;
		stCaptureParms.fAbortRightMouse = FALSE;
		stCaptureParms.vKeyAbort = 0;
		// 设置捕捉参数
		if( !capCaptureSetSetup( m_hVideo, &stCaptureParms, sizeof(CAPTUREPARMS) ) )
			MessageBox( _T("捕捉参数设置失败") );
	}

	// 图像信息
	SetVideoBmpFormat();

	// 注册回调函数
	//// 预览捕获
	//if( !capSetCallbackOnFrame( m_hVideo, capFrameCallbackProc ) )
	//	MessageBox( _T("回调函数注册失败"));
	// 直接捕获
	if( !capSetCallbackOnVideoStream( m_hVideo, capFrameCallbackProc ) )
		MessageBox( _T("回调函数注册失败"));

	return TRUE;
}

void CVbcdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVbcdDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_TB_SET, &CVbcdDlg::OnTbSet)
	ON_COMMAND(ID_TB_CONNECT, &CVbcdDlg::OnTbConnect)
	ON_COMMAND(ID_TB_SOURCE_SET, &CVbcdDlg::OnTbSourceSet)
	ON_COMMAND(ID_TB_FORMAT_SET, &CVbcdDlg::OnTbFormatSet)
	ON_COMMAND(ID_TB_COMPRESS_SET, &CVbcdDlg::OnTbCompressSet)

	ON_COMMAND(ID_TB_PREVIEW, &CVbcdDlg::OnTbPreview)
	ON_COMMAND(ID_TB_DIRECT_SHOW, &CVbcdDlg::OnTbDirectShow)

	ON_COMMAND_EX(IDM_ABOUTBOX, &CVbcdDlg::OnSysCommandEx)
END_MESSAGE_MAP()


// CVbcdDlg 消息处理程序
//////////////////////////////////////////////////////////////////////////
BOOL CVbcdDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			//pSysMenu->InsertMenu( 0, MF_SEPARATOR );
			pSysMenu->InsertMenu( 0, MF_STRING, IDM_ABOUTBOX, strAboutMenu );
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:

	// 创建工具栏
	if ( !m_wndToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_TOOLBAR_SYS) )
	{
		TRACE0("未能创建工具栏\n");
		return -1;      // 未能创建
	}
	m_wndToolBar.SetButtonText( 0, _T("设置") );
	m_wndToolBar.SetButtonText( 2, _T("预览") );
	m_wndToolBar.SetButtonText( 3, _T("处理效果") );
	m_wndToolBar.SetButtonText( 5, _T("关于") );
	m_wndToolBar.SetSizes( CSize( 60, TOOLBAR_HEIGHT-2 ), CSize( 16, 16 ) );
	// 按钮属性
	m_wndToolBar.SetButtonStyle( 0, TBBS_AUTOSIZE );
	m_wndToolBar.SetButtonStyle( 2, TBBS_CHECKBOX|TBBS_AUTOSIZE );
	m_wndToolBar.SetButtonStyle( 3, TBBS_CHECKBOX|TBBS_AUTOSIZE );
	m_wndToolBar.SetButtonStyle( 5, TBBS_AUTOSIZE );
	RepositionBars( AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0 );
	m_wndToolBar.ShowWindow( SW_SHOW );
	// 无效按钮
	m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_PREVIEW, FALSE );
	m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_DIRECT_SHOW, FALSE );

	// 创建效果窗口
	if	( !m_pcEffectDlg->Create( IDD_EFFECT_DIALOG, this ) )
		MessageBox( _T("显示窗口创建失败！\n") );
	else
		TRACE( "显示窗口创建成功。\n" );

	// 图像处理线程初始化
	if ( !m_pcImgProc->Init() )
		MessageBox( _T("图像处理线程创建失败！\n") );
	else
		TRACE( "图像处理线程创建成功。\n" );

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 系统菜单消息事件
void CVbcdDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CVbcdDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// 父类的初始化，系统自动生成
		{
			CPaintDC dc(this); // 用于绘制的设备上下文
			CRect rc;
			GetClientRect( &rc );

			// 绘制工具栏下划线
			CPen myPen;
			myPen.CreatePen( PS_SOLID, 1, RGB( 197, 194, 184 ) );
			CPen* pOldPen = dc.SelectObject( &myPen );
			dc.SelectStockObject( NULL_BRUSH );
			dc.MoveTo( 0, TOOLBAR_HEIGHT+3 );
			dc.LineTo( rc.Width(), TOOLBAR_HEIGHT+3 );
			dc.SelectObject( pOldPen );
			myPen.DeleteObject();

			if ( !m_stDataStatus.btConnect )
				return;
			// 没有预览时填充黑色
			if( !m_stDataStatus.btPreview )
			{
				GetDlgItem( IDC_VIDEO )->GetWindowRect( &rc );
				ScreenToClient( &rc );
				dc.FillSolidRect( &rc, 0x00000000 );
				dc.SetBkColor( RGB( 0, 0, 0 ) );
				dc.SetTextColor( RGB( 255, 255, 0 ) );
				dc.TextOut( rc.left, rc.top, _T("预览区域") );
			}
			// 没有效果显示是，填充黑色
			if ( !m_stDataStatus.btDrcShow )
			{
				m_pcEffectDlg->GetWindowRect( &rc );
				ScreenToClient( &rc );
				dc.FillSolidRect( &rc, 0x00000000 );
				dc.SetBkColor( RGB( 0, 0, 0 ) );
				dc.SetTextColor( RGB( 255, 255, 0 ) );
				dc.TextOut( rc.left, rc.top, _T("处理结果区域") );
			}
		}
		//CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CVbcdDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 设置
void CVbcdDlg::OnTbSet()
{
	// 获取按钮区域
	CRect rc;
	m_wndToolBar.GetItemRect( 0, &rc );
	ClientToScreen( &rc );

	CMenu menu;
	menu.LoadMenu( IDR_SYS_MENU );
	CMenu* pSubMenu = NULL;
	pSubMenu = menu.GetSubMenu( 0 ); // 设置菜单
	// 是否连接
	if( m_stDataStatus.btConnect == 0 )
		pSubMenu->ModifyMenu( ID_TB_CONNECT , MF_BYCOMMAND|MF_STRING, ID_TB_CONNECT, _T("连接(&C)\t") );
	else
		pSubMenu->ModifyMenu( ID_TB_CONNECT , MF_BYCOMMAND|MF_STRING, ID_TB_CONNECT, _T("断开(&C)\t") );
	// 弹出菜单
	if (pSubMenu)
		pSubMenu->TrackPopupMenu( 0, rc.left + 5, rc.bottom, this );
}

// 连接设备
void CVbcdDlg::OnTbConnect()
{
	// TODO:

	// VFW功能
	// 没有连接
	if( m_stDataStatus.btConnect == 0 )
	{
		// 标志-已连接
		m_stDataStatus.btConnect = 1;
		// 连接设备，初始化
		if( !InitVFWBlock() )
			m_stDataStatus.btConnect = 0;

		// 按钮有效
		m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_PREVIEW );
		m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_DIRECT_SHOW );
		// 更新界面
		RedrawWindow( NULL, NULL, RDW_INVALIDATE|RDW_NOERASE|RDW_INTERNALPAINT );
	}
	// 已连接
	else
	{
		// 恢复按钮
		CToolBarCtrl& ctrToolBar = m_wndToolBar.GetToolBarCtrl();
		ctrToolBar.CheckButton( ID_TB_PREVIEW, FALSE );
		ctrToolBar.CheckButton( ID_TB_DIRECT_SHOW, FALSE );
		// 按钮无效
		ctrToolBar.EnableButton( ID_TB_PREVIEW, FALSE );
		ctrToolBar.EnableButton( ID_TB_DIRECT_SHOW, FALSE );

		// 隐藏显示框
		m_pcEffectDlg->ShowWindow( SW_HIDE );
		// 停止捕获
		//capCaptureAbort( m_hVideo );
		capCaptureStop( m_hVideo );
		// 停止处理线程
		m_pcImgProc->Pause();

		// 停止预览
		capOverlay( m_hVideo, FALSE );
		capPreview( m_hVideo, FALSE );
		// 隐藏预览区
		GetDlgItem( IDC_VIDEO )->ShowWindow( SW_HIDE );

		// 清界面
		Invalidate(TRUE);
		m_stBitMap.bmHeight = 0;
		SetWindowPos( NULL, 0, 0, 363, 305, SWP_NOMOVE|SWP_NOZORDER );
		CenterWindow();

		// 结束注册
		capSetCallbackOnFrame ( m_hVideo, NULL );
		// 停止连接
		capDriverDisconnect( m_hVideo );

		// 标志-全清
		ZeroMemory( &m_stDataStatus, sizeof( m_stDataStatus ) );
	}
}

// 设置-视频源
void CVbcdDlg::OnTbSourceSet()
{
	// TODO:
	capDlgVideoSource( m_hVideo );
	// 重新计算画面
	SetVideoBmpFormat();
}

// 设置-视频格式
void CVbcdDlg::OnTbFormatSet()
{
	// TODO:
	capDlgVideoFormat( m_hVideo );
	// 重新计算画面
	SetVideoBmpFormat();
}

// 设置-视频压缩
void CVbcdDlg::OnTbCompressSet()
{
	// TODO:
	capDlgVideoCompression( m_hVideo );
	// 重新计算画面
	SetVideoBmpFormat();
}

// 视图-预览
void CVbcdDlg::OnTbPreview()
{
	// TODO:
	CAPDRIVERCAPS stCapDvr;
	if( !capDriverGetCaps( m_hVideo, &stCapDvr, sizeof(CAPDRIVERCAPS) ) )
		MessageBox( _T("获取驱动性能失败") );

	if( m_stDataStatus.btPreview == 0 )
	{
		GetDlgItem( IDC_VIDEO )->ShowWindow( SW_SHOWNOACTIVATE );

		// 设置Preview帧速率
		capPreviewRate( m_hVideo, 67 );
		if( stCapDvr.fHasOverlay )
			// 启动Overlay模式
			capOverlay( m_hVideo, TRUE );
		else
			// 启动Preview模式
			capPreview( m_hVideo, TRUE );

		// 标志-预览
		m_stDataStatus.btPreview = 1;
	}
	else
	{
		GetDlgItem( IDC_VIDEO )->ShowWindow( SW_HIDE );

		if( stCapDvr.fHasOverlay )
			// 启动Overlay模式
			capOverlay( m_hVideo, FALSE );
		else
			// 启动Preview模式
			capPreview( m_hVideo, FALSE );

		// 标志-未预览
		m_stDataStatus.btPreview = 0;
	}
}

// 视图-效果显示 消息响应函数
void CVbcdDlg::OnTbDirectShow()
{
	// TODO:

	// 判断显示窗口是否弹出
	//  没有弹出，即线程池未启动，未进行图像处理，下面启动线程池
	if ( m_stDataStatus.btDrcShow == 0 )
	{
		// 显示
		m_pcEffectDlg->ShowWindow( SW_SHOW );
		// 标志-直接显示
		m_stDataStatus.btDrcShow = 1;

		// 启动处理线程
		m_pcImgProc->Start();

		// 有文件捕获
		//capCaptureSequence( m_hVideo );
		// 无文件捕获
		capCaptureSequenceNoFile( m_hVideo );
	}
	// 已经弹出，即线程池已经启动，下面暂停线程池
	else
	{
		capCaptureAbort( m_hVideo );
		//capCaptureStop( m_hVideo );

		// 停止处理线程
		m_pcImgProc->Pause();

		// 隐藏窗口
		m_pcEffectDlg->ShowWindow( SW_HIDE );
		// 标志-未显示
		m_stDataStatus.btDrcShow = 0;
	}
}

// 发送系统菜单命令
BOOL CVbcdDlg::OnSysCommandEx(UINT uiID)
{
	// TODO:
	PostMessage( WM_SYSCOMMAND, uiID, NULL );
	return TRUE;
}

// 退出
void CVbcdDlg::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	// 关闭连接
	if( m_stDataStatus.btConnect == 1 )
		OnTbConnect();

	// 退出线程
	m_pcImgProc->Exit();

	CDialog::OnCancel();
}
