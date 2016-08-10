// EffectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Vbcd.h"
#include "EffectDlg.h"


// CEffectDlg �Ի���

IMPLEMENT_DYNAMIC(CEffectDlg, CDialog)

CEffectDlg::CEffectDlg( CWnd* pParent /*=NULL*/)
	: CDialog( CEffectDlg::IDD, pParent )
{
	// ���ݲ�������
	InitializeCriticalSection( &m_crtPro );

	// ͼ������
	m_lpRGB = new RGBQUAD[ RESO_XY ];
	ZeroMemory( m_lpRGB, RESO_XY*sizeof(RGBQUAD) );
	// �߿�
	m_pstEdgePara = new ST_EDGE_PARA;
	ZeroMemory( m_pstEdgePara, sizeof(ST_EDGE_PARA) );
	// �ǵ�
	m_pvecCorner = new VEC_CORNER;
	m_pvecCorner->clear();
}

CEffectDlg::~CEffectDlg()
{
	// ɾ���ؼ������
	DeleteCriticalSection( &m_crtPro );
	// �ͷ��ڴ�
	ReleaseArray( m_lpRGB );
	ReleasePoint( m_pstEdgePara );
	ReleasePoint( m_pvecCorner );
}

// ���� ����
BOOL CEffectDlg::RefreshData( PST_PROC_DATA pstProcData )
{
	EnterCriticalSection( &m_crtPro );
	LPVOID lpVoid = NULL;

	// ͼ������
	lpVoid = m_lpRGB;
	m_lpRGB = pstProcData->lpRGB;
	pstProcData->lpRGB = (LPRGBQUAD)lpVoid;
	// �߿�
	lpVoid = m_pstEdgePara;
	m_pstEdgePara = pstProcData->pstEdgePara;
	pstProcData->pstEdgePara = (PST_EDGE_PARA)lpVoid;
	// �ǵ�
	m_pvecCorner->clear();
	lpVoid = m_pvecCorner;
	m_pvecCorner = pstProcData->pVecCorner;
	pstProcData->pVecCorner = (VEC_CORNER*)lpVoid;
	LeaveCriticalSection( &m_crtPro );

	// ˢ��
	RedrawWindow( NULL, NULL, RDW_NOERASE|RDW_INVALIDATE|RDW_INTERNALPAINT );

	return TRUE;
}

void CEffectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEffectDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CEffectDlg ��Ϣ�������

void CEffectDlg::OnPaint()
{
	// �Ի����ͼDC
	CPaintDC dc(this);

	if( !TryEnterCriticalSection( &m_crtPro ) )
		return;

	// ����λͼ
	CBitmap bmpTmp;
	if ( !bmpTmp.CreateBitmap( m_stBitMap.bmWidth, m_stBitMap.bmHeight, m_stBitMap.bmPlanes, m_stBitMap.bmBitsPixel, m_lpRGB ) )
	{
		MessageBox( _T("λͼ����ʧ��") );
		ShowWindow( SW_HIDE );
		return;
	}

	// �ڴ�DC
	CDC dcMem;
	dcMem.CreateCompatibleDC( &dc );
	CBitmap* pOldBmp = dcMem.SelectObject( &bmpTmp );

	if( m_pstEdgePara->bValid )
	{
		// ����ı߿�
		{
			CPen PenRed;
			PenRed.CreatePen( PS_SOLID, 2, RGB(255,255,0) );
			CPen* pOldPen = dcMem.SelectObject( &PenRed );
			dcMem.MoveTo( m_pstEdgePara->point[0].x, m_pstEdgePara->point[0].y );
			dcMem.LineTo( m_pstEdgePara->point[1].x, m_pstEdgePara->point[1].y );
			dcMem.LineTo( m_pstEdgePara->point[3].x, m_pstEdgePara->point[3].y );
			dcMem.LineTo( m_pstEdgePara->point[2].x, m_pstEdgePara->point[2].y );
			dcMem.LineTo( m_pstEdgePara->point[0].x, m_pstEdgePara->point[0].y );

			dcMem.SelectObject( pOldPen );
			PenRed.DeleteObject();
		}
		// ���ǵ�
		if( m_pvecCorner->size() )
		{
			CPen PenRed;
			PenRed.CreatePen( PS_SOLID, 1, RGB(255,0,255) );
			CPen* pOldPen = dcMem.SelectObject( &PenRed );
			long j = 0, i = 0;
			for( VEC_CORNER::iterator iter = m_pvecCorner->begin(); iter != m_pvecCorner->end(); iter++ )
			{
				j = (*iter).x;
				i = (*iter).y;
				//�ڽǵ㴦��ʮ�ֲ��Ա�ע
				dcMem.MoveTo( j-3, i );
				dcMem.LineTo( j+4, i );
				dcMem.MoveTo( j, i-3 );
				dcMem.LineTo( j, i+4 );
			}
			dcMem.SelectObject( pOldPen );
			PenRed.DeleteObject();
		}
	}
	else
	{
		dcMem.SetBkColor( RGB( 0, 0, 0 ) );
		dcMem.SetTextColor( RGB( 255, 255, 0 ) );
		dcMem.TextOut( m_stBitMap.bmWidth/2 - 63, m_stBitMap.bmHeight/2 - 16, _T("û���ҵ��鱾�߿�") );
	}

	LeaveCriticalSection( &m_crtPro );

	// ѡ����ʾDC
	dc.BitBlt( 1, 1, m_stBitMap.bmWidth, m_stBitMap.bmHeight, &dcMem, 0, 0, SRCCOPY );
	{
		CPen PenRed;
		PenRed.CreatePen( PS_SOLID, 1, RGB(255,255,255) );
		CPen* pOldPen = dc.SelectObject( &PenRed );
		dc.SelectStockObject( NULL_BRUSH );
		dc.Rectangle( 0, 0, m_stBitMap.bmWidth+2, m_stBitMap.bmHeight );
		dc.SelectObject( pOldPen );
		PenRed.DeleteObject();
	}

	// �ͷ�
	dcMem.SelectObject( pOldBmp );
	bmpTmp.DeleteObject();

}

BOOL CEffectDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	return TRUE;
	return CDialog::OnEraseBkgnd(pDC);
}
