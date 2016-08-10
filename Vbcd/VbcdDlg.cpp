// VbcdDlg.cpp : ʵ���ļ�
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
// ֡�ɼ��ص�
LRESULT CALLBACK capFrameCallbackProc( HWND hWnd, LPVIDEOHDR lpVHdr )
{
	if( hWnd != CVbcdDlg::sm_pWnd->m_hVideo )
		return S_FALSE;

	// ֪ͨ���������������
	CVbcdDlg::sm_pWnd->m_pcImgProc->m_lpVHdr = lpVHdr;
	SetEvent( CVbcdDlg::sm_pWnd->m_pcImgProc->m_hHasData );

	return S_OK;
}


// CVbcdDlg �Ի���
//////////////////////////////////////////////////////////////////////////
CVbcdDlg* CVbcdDlg::sm_pWnd = NULL;

CVbcdDlg::CVbcdDlg( CWnd* pParent /*=NULL*/ )
	: CDialog( CVbcdDlg::IDD, pParent )
{
	// ������ͼ��
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// �����ָ��
	sm_pWnd = this;
	// ��Ƶ����
	m_hVideo = NULL;
	// ͼ����Ϣ��ʼ��
	ZeroMemory( &m_stBitMap, sizeof(m_stBitMap) );
	// ��־
	ZeroMemory( &m_stDataStatus, sizeof(m_stDataStatus) );

	// Ч������
	m_pcEffectDlg = new CEffectDlg( this );

	// ͼ����
	m_pcImgProc = new CImgProcess( this, m_pcEffectDlg );
}

// ��׼��������
CVbcdDlg::~CVbcdDlg()
{
	// ��״̬
	ZeroMemory( &m_stDataStatus, sizeof(m_stDataStatus) );

	// �ͷ�Ч������
	m_pcEffectDlg->DestroyWindow();
	ReleasePoint( m_pcEffectDlg );
	// �ͷ�ͼ���������̳߳�
	ReleasePoint( m_pcImgProc );
}

BOOL CVbcdDlg::SetVideoBmpFormat()
{
	// ��ȡ״̬
	CAPSTATUS stCapStatus;
	if( !capGetStatus( m_hVideo, &stCapStatus, sizeof(CAPSTATUS) ) )
		return FALSE;

	// ���ͼ����Ϣ�ı�
	if(
		m_stBitMap.bmHeight != stCapStatus.uiImageHeight ||
		m_stBitMap.bmWidth != stCapStatus.uiImageWidth 
		)
	{
		// ��ȡͼ����Ϣ
		BITMAPINFO stBitmapInfo;
		capGetVideoFormat( m_hVideo, &stBitmapInfo, sizeof(BITMAPINFO) );
		m_stBitMap.bmType = 0;
		m_stBitMap.bmWidth = stBitmapInfo.bmiHeader.biWidth;
		m_stBitMap.bmHeight = stBitmapInfo.bmiHeader.biHeight;
		m_stBitMap.bmPlanes = stBitmapInfo.bmiHeader.biPlanes;
		m_stBitMap.bmBitsPixel = 32;
		m_stBitMap.bmWidthBytes = m_stBitMap.bmWidth * 4;

		// ���ô��ڴ�С
		SetWindowPos( NULL, 0, 0, m_stBitMap.bmWidth*2 + 35, m_stBitMap.bmHeight + TOOLBAR_HEIGHT + 50, SWP_NOMOVE|SWP_NOZORDER );
		CenterWindow();
		// ����Ԥ������С
		GetDlgItem( IDC_VIDEO )->SetWindowPos( NULL, 10, TOOLBAR_HEIGHT+10, m_stBitMap.bmWidth, m_stBitMap.bmHeight, SWP_NOZORDER );
		// ����Ч�����ڴ�С
		m_pcEffectDlg->SetBitMap( m_stBitMap );
		m_pcEffectDlg->SetWindowPos( NULL, m_stBitMap.bmWidth + 15, TOOLBAR_HEIGHT+9, m_stBitMap.bmWidth + 4, m_stBitMap.bmHeight + 2, SWP_NOACTIVATE );
		// ���� ����ͼ�� ��Ϣ
		m_pcImgProc->SetBitMap( m_stBitMap );

		// ���½���
		RedrawWindow( NULL, NULL, RDW_INVALIDATE|RDW_ERASE|RDW_INTERNALPAINT );
	}
	return TRUE;
}

// ��ʼ��VFW��ع���
BOOL CVbcdDlg::InitVFWBlock()
{
	// VFWԤ������
	if( m_hVideo == NULL )
		m_hVideo = ::capCreateCaptureWindow(
			_T("VideoPreview"),
			WS_CHILDWINDOW|WS_BORDER,
			0, 0, 1, 1,
			m_hWnd, IDC_VIDEO );
	if( m_hVideo == NULL )
		return FALSE;

	// ��ȡϵͳ�ܹ�ʹ�õ�ý���豸���������������汾
	{
		CAPDRIVERCAPS stCapDvr;
		TCHAR szDeviceName[ 80 ];
		TCHAR szDeviceVersion[ 80 ];
		for ( int wIndex = 0; ; wIndex++ )
		{
			// һһ��ȡ�豸��Ϣ
			if ( ::capGetDriverDescription ( wIndex, szDeviceName, 80, szDeviceVersion, 80 ) )
			{
				// �����豸
				if( capDriverConnect( m_hVideo, wIndex ) )
				{
					capDriverGetCaps( m_hVideo, &stCapDvr, sizeof(CAPDRIVERCAPS) );
					if( stCapDvr.fCaptureInitialized )
						// ���ã���������
						break;
				}
				else
				{
					CString csInfo = _T("�����豸ʧ�ܣ��豸���ƣ�");
					csInfo += szDeviceName;
					MessageBox( csInfo );
				}
			}
			else
			{
				MessageBox( _T("û���ҵ����������豸��") );
				return FALSE;
			}
		}
	}

	// ��׽����
	{
		CAPTUREPARMS stCaptureParms;
		// ��ȡ��׽����
		if( !capCaptureGetSetup( m_hVideo, &stCaptureParms, sizeof(CAPTUREPARMS) ) )
			MessageBox( _T("��׽������ȡʧ��") );
		// ÿ֡66667΢��
		stCaptureParms.dwRequestMicroSecPerFrame = 66667;
		// ��������
		stCaptureParms.fYield = TRUE;
		stCaptureParms.wNumVideoRequested = 0;
		// ����׽��Ƶ
		stCaptureParms.fCaptureAudio = FALSE;
		stCaptureParms.wNumAudioRequested = 0;
		stCaptureParms.fAbortLeftMouse = FALSE;
		stCaptureParms.fAbortRightMouse = FALSE;
		stCaptureParms.vKeyAbort = 0;
		// ���ò�׽����
		if( !capCaptureSetSetup( m_hVideo, &stCaptureParms, sizeof(CAPTUREPARMS) ) )
			MessageBox( _T("��׽��������ʧ��") );
	}

	// ͼ����Ϣ
	SetVideoBmpFormat();

	// ע��ص�����
	//// Ԥ������
	//if( !capSetCallbackOnFrame( m_hVideo, capFrameCallbackProc ) )
	//	MessageBox( _T("�ص�����ע��ʧ��"));
	// ֱ�Ӳ���
	if( !capSetCallbackOnVideoStream( m_hVideo, capFrameCallbackProc ) )
		MessageBox( _T("�ص�����ע��ʧ��"));

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


// CVbcdDlg ��Ϣ�������
//////////////////////////////////////////////////////////////////////////
BOOL CVbcdDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�
	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:

	// ����������
	if ( !m_wndToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_TOOLBAR_SYS) )
	{
		TRACE0("δ�ܴ���������\n");
		return -1;      // δ�ܴ���
	}
	m_wndToolBar.SetButtonText( 0, _T("����") );
	m_wndToolBar.SetButtonText( 2, _T("Ԥ��") );
	m_wndToolBar.SetButtonText( 3, _T("����Ч��") );
	m_wndToolBar.SetButtonText( 5, _T("����") );
	m_wndToolBar.SetSizes( CSize( 60, TOOLBAR_HEIGHT-2 ), CSize( 16, 16 ) );
	// ��ť����
	m_wndToolBar.SetButtonStyle( 0, TBBS_AUTOSIZE );
	m_wndToolBar.SetButtonStyle( 2, TBBS_CHECKBOX|TBBS_AUTOSIZE );
	m_wndToolBar.SetButtonStyle( 3, TBBS_CHECKBOX|TBBS_AUTOSIZE );
	m_wndToolBar.SetButtonStyle( 5, TBBS_AUTOSIZE );
	RepositionBars( AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0 );
	m_wndToolBar.ShowWindow( SW_SHOW );
	// ��Ч��ť
	m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_PREVIEW, FALSE );
	m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_DIRECT_SHOW, FALSE );

	// ����Ч������
	if	( !m_pcEffectDlg->Create( IDD_EFFECT_DIALOG, this ) )
		MessageBox( _T("��ʾ���ڴ���ʧ�ܣ�\n") );
	else
		TRACE( "��ʾ���ڴ����ɹ���\n" );

	// ͼ�����̳߳�ʼ��
	if ( !m_pcImgProc->Init() )
		MessageBox( _T("ͼ�����̴߳���ʧ�ܣ�\n") );
	else
		TRACE( "ͼ�����̴߳����ɹ���\n" );

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// ϵͳ�˵���Ϣ�¼�
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�
void CVbcdDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// ����ĳ�ʼ����ϵͳ�Զ�����
		{
			CPaintDC dc(this); // ���ڻ��Ƶ��豸������
			CRect rc;
			GetClientRect( &rc );

			// ���ƹ������»���
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
			// û��Ԥ��ʱ����ɫ
			if( !m_stDataStatus.btPreview )
			{
				GetDlgItem( IDC_VIDEO )->GetWindowRect( &rc );
				ScreenToClient( &rc );
				dc.FillSolidRect( &rc, 0x00000000 );
				dc.SetBkColor( RGB( 0, 0, 0 ) );
				dc.SetTextColor( RGB( 255, 255, 0 ) );
				dc.TextOut( rc.left, rc.top, _T("Ԥ������") );
			}
			// û��Ч����ʾ�ǣ�����ɫ
			if ( !m_stDataStatus.btDrcShow )
			{
				m_pcEffectDlg->GetWindowRect( &rc );
				ScreenToClient( &rc );
				dc.FillSolidRect( &rc, 0x00000000 );
				dc.SetBkColor( RGB( 0, 0, 0 ) );
				dc.SetTextColor( RGB( 255, 255, 0 ) );
				dc.TextOut( rc.left, rc.top, _T("����������") );
			}
		}
		//CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
HCURSOR CVbcdDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ����
void CVbcdDlg::OnTbSet()
{
	// ��ȡ��ť����
	CRect rc;
	m_wndToolBar.GetItemRect( 0, &rc );
	ClientToScreen( &rc );

	CMenu menu;
	menu.LoadMenu( IDR_SYS_MENU );
	CMenu* pSubMenu = NULL;
	pSubMenu = menu.GetSubMenu( 0 ); // ���ò˵�
	// �Ƿ�����
	if( m_stDataStatus.btConnect == 0 )
		pSubMenu->ModifyMenu( ID_TB_CONNECT , MF_BYCOMMAND|MF_STRING, ID_TB_CONNECT, _T("����(&C)\t") );
	else
		pSubMenu->ModifyMenu( ID_TB_CONNECT , MF_BYCOMMAND|MF_STRING, ID_TB_CONNECT, _T("�Ͽ�(&C)\t") );
	// �����˵�
	if (pSubMenu)
		pSubMenu->TrackPopupMenu( 0, rc.left + 5, rc.bottom, this );
}

// �����豸
void CVbcdDlg::OnTbConnect()
{
	// TODO:

	// VFW����
	// û������
	if( m_stDataStatus.btConnect == 0 )
	{
		// ��־-������
		m_stDataStatus.btConnect = 1;
		// �����豸����ʼ��
		if( !InitVFWBlock() )
			m_stDataStatus.btConnect = 0;

		// ��ť��Ч
		m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_PREVIEW );
		m_wndToolBar.GetToolBarCtrl().EnableButton( ID_TB_DIRECT_SHOW );
		// ���½���
		RedrawWindow( NULL, NULL, RDW_INVALIDATE|RDW_NOERASE|RDW_INTERNALPAINT );
	}
	// ������
	else
	{
		// �ָ���ť
		CToolBarCtrl& ctrToolBar = m_wndToolBar.GetToolBarCtrl();
		ctrToolBar.CheckButton( ID_TB_PREVIEW, FALSE );
		ctrToolBar.CheckButton( ID_TB_DIRECT_SHOW, FALSE );
		// ��ť��Ч
		ctrToolBar.EnableButton( ID_TB_PREVIEW, FALSE );
		ctrToolBar.EnableButton( ID_TB_DIRECT_SHOW, FALSE );

		// ������ʾ��
		m_pcEffectDlg->ShowWindow( SW_HIDE );
		// ֹͣ����
		//capCaptureAbort( m_hVideo );
		capCaptureStop( m_hVideo );
		// ֹͣ�����߳�
		m_pcImgProc->Pause();

		// ֹͣԤ��
		capOverlay( m_hVideo, FALSE );
		capPreview( m_hVideo, FALSE );
		// ����Ԥ����
		GetDlgItem( IDC_VIDEO )->ShowWindow( SW_HIDE );

		// �����
		Invalidate(TRUE);
		m_stBitMap.bmHeight = 0;
		SetWindowPos( NULL, 0, 0, 363, 305, SWP_NOMOVE|SWP_NOZORDER );
		CenterWindow();

		// ����ע��
		capSetCallbackOnFrame ( m_hVideo, NULL );
		// ֹͣ����
		capDriverDisconnect( m_hVideo );

		// ��־-ȫ��
		ZeroMemory( &m_stDataStatus, sizeof( m_stDataStatus ) );
	}
}

// ����-��ƵԴ
void CVbcdDlg::OnTbSourceSet()
{
	// TODO:
	capDlgVideoSource( m_hVideo );
	// ���¼��㻭��
	SetVideoBmpFormat();
}

// ����-��Ƶ��ʽ
void CVbcdDlg::OnTbFormatSet()
{
	// TODO:
	capDlgVideoFormat( m_hVideo );
	// ���¼��㻭��
	SetVideoBmpFormat();
}

// ����-��Ƶѹ��
void CVbcdDlg::OnTbCompressSet()
{
	// TODO:
	capDlgVideoCompression( m_hVideo );
	// ���¼��㻭��
	SetVideoBmpFormat();
}

// ��ͼ-Ԥ��
void CVbcdDlg::OnTbPreview()
{
	// TODO:
	CAPDRIVERCAPS stCapDvr;
	if( !capDriverGetCaps( m_hVideo, &stCapDvr, sizeof(CAPDRIVERCAPS) ) )
		MessageBox( _T("��ȡ��������ʧ��") );

	if( m_stDataStatus.btPreview == 0 )
	{
		GetDlgItem( IDC_VIDEO )->ShowWindow( SW_SHOWNOACTIVATE );

		// ����Preview֡����
		capPreviewRate( m_hVideo, 67 );
		if( stCapDvr.fHasOverlay )
			// ����Overlayģʽ
			capOverlay( m_hVideo, TRUE );
		else
			// ����Previewģʽ
			capPreview( m_hVideo, TRUE );

		// ��־-Ԥ��
		m_stDataStatus.btPreview = 1;
	}
	else
	{
		GetDlgItem( IDC_VIDEO )->ShowWindow( SW_HIDE );

		if( stCapDvr.fHasOverlay )
			// ����Overlayģʽ
			capOverlay( m_hVideo, FALSE );
		else
			// ����Previewģʽ
			capPreview( m_hVideo, FALSE );

		// ��־-δԤ��
		m_stDataStatus.btPreview = 0;
	}
}

// ��ͼ-Ч����ʾ ��Ϣ��Ӧ����
void CVbcdDlg::OnTbDirectShow()
{
	// TODO:

	// �ж���ʾ�����Ƿ񵯳�
	//  û�е��������̳߳�δ������δ����ͼ�������������̳߳�
	if ( m_stDataStatus.btDrcShow == 0 )
	{
		// ��ʾ
		m_pcEffectDlg->ShowWindow( SW_SHOW );
		// ��־-ֱ����ʾ
		m_stDataStatus.btDrcShow = 1;

		// ���������߳�
		m_pcImgProc->Start();

		// ���ļ�����
		//capCaptureSequence( m_hVideo );
		// ���ļ�����
		capCaptureSequenceNoFile( m_hVideo );
	}
	// �Ѿ����������̳߳��Ѿ�������������ͣ�̳߳�
	else
	{
		capCaptureAbort( m_hVideo );
		//capCaptureStop( m_hVideo );

		// ֹͣ�����߳�
		m_pcImgProc->Pause();

		// ���ش���
		m_pcEffectDlg->ShowWindow( SW_HIDE );
		// ��־-δ��ʾ
		m_stDataStatus.btDrcShow = 0;
	}
}

// ����ϵͳ�˵�����
BOOL CVbcdDlg::OnSysCommandEx(UINT uiID)
{
	// TODO:
	PostMessage( WM_SYSCOMMAND, uiID, NULL );
	return TRUE;
}

// �˳�
void CVbcdDlg::OnCancel()
{
	// TODO: �ڴ����ר�ô����/����û���

	// �ر�����
	if( m_stDataStatus.btConnect == 1 )
		OnTbConnect();

	// �˳��߳�
	m_pcImgProc->Exit();

	CDialog::OnCancel();
}
