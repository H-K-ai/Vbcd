
#include "StdAfx.h"

#include <math.h>
#include <algorithm>

#include "Vbcd.h"
#include "VbcdDlg.h"
#include "EffectDlg.h"
#include "ImgProcess.h"

#define EQ_SIZE	65535	// �ȼ۶������С

CImgProcess::CImgProcess( CVbcdDlg* pMainWnd, CEffectDlg* pEffectWnd )
	: m_pMainWnd( pMainWnd )		// ������
	, m_pEffectWnd( pEffectWnd )	// ��ʾ����
{
	// ��ͼ����Ϣ
	ZeroMemory( &m_stBitMap, sizeof( BITMAP ) );
	// ��Ե����ڴ�ṹ
	ZeroMemory( &m_stEdgeDetect, sizeof( m_stEdgeDetect ) );
	// �ǵ����ڴ�ṹ
	ZeroMemory( &m_stCornerDtec, sizeof( m_stCornerDtec ) );
}

CImgProcess::~CImgProcess(void)
{
}

// ��ȡ����
BOOL CImgProcess::GetData( PST_PROC_DATA pstProcData )
{
	// ͼ��Ŀ��
	long lbmW = m_stBitMap.bmWidth;
	long lbmH = m_stBitMap.bmHeight;
	// ��������
	LPRGBQUAD pstRgb = pstProcData->lpRGB;
	LPBYTE pbyPixel = m_lpVHdr->lpData;

	int iY = 0;
	int iU = 0;
	int iV = 0;
	float fT = 0.0f;
	// ��ȡ���ݣ��Ѳ����ͼ��ת��ΪRGBͼ��ͬʱ����һ�ݻҶ�ͼ��
	for( long lr = 0; lr < lbmH; lr++ )
	{
		for( long lc = 0; lc < lbmW; lc ++ )
		{
			// ��YUY2��ʽͼ��ȡYUV����
			if( lc%2 == 0 )
			{
				iY = (int)*pbyPixel - 16;
				iU = (int)*(pbyPixel + 1) - 128;
				iV = (int)*(pbyPixel + 3) - 128;
			}
			else
			{
				iY = (int)*pbyPixel - 16;
				iU = (int)*(pbyPixel - 1) - 128;
				iV = (int)*(pbyPixel + 1) - 128;
			}

			// YUV->RGB����YUY2��ʽͼ��ת��ΪRGB��ʽͼ��
			fT = 1.164383f * iY + 1.596027f * iV;
			if( fT > 255 )
				fT = 255;
			else if( fT < 0 )
				fT = 0;
			pstRgb->rgbRed = static_cast<BYTE>(fT);
			fT = 1.164383f * iY - 0.391762f * iU - 0.812969f * iV;
			if( fT > 255 )
				fT = 255;
			else if( fT < 0 )
				fT = 0;
			pstRgb->rgbGreen = static_cast<BYTE>(fT);
			fT = 1.164383f * iY  + 2.017230f * iU;
			if( fT > 255 )
				fT = 255;
			else if( fT < 0 )
				fT = 0;
			pstRgb->rgbBlue = static_cast<BYTE>(fT);

			// TUV->GRAY������һ�ݻҶ�ͼ��
			pstRgb->rgbReserved = *pbyPixel;

			// ƫ��ָ��
			pbyPixel += 2;
			pstRgb ++;
		}
	}
	return TRUE;
}

// �����Ե���
BOOL CImgProcess::HoughEdge()
{
	// ����32λ����
	if( m_stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// �� ��Ե��� �ڴ�
	ZeroMemory( m_stEdgeDetect.pbyEgDtc, RESO_XY );

	// ��Ե���
	{
		// ��ֵ
		long lThre = 100;
		// �ճ���1��
		LPRGBQUAD lpSrc[9] = { NULL };

		LPBYTE pbyMem = m_stEdgeDetect.pbyEgDtc + m_stBitMap.bmWidth;
		short sDTemp = 0;
		for( long i = 1; i < m_stBitMap.bmHeight - 1; i++ )		// ÿ��
		{
			// �ճ���1��
			pbyMem ++;
			for( long j = 1; j < m_stBitMap.bmWidth - 1; j++ )	// ÿ��
			{
				// ָ��DIB��i�У���j�����ص�ָ��
				lpSrc[4] = m_stEdgeDetect.lpRGB + i*m_stBitMap.bmWidth + j; lpSrc[3] = lpSrc[4] - 1; lpSrc[5] = lpSrc[4] + 1;
				lpSrc[0] = lpSrc[3] - m_stBitMap.bmWidth; lpSrc[1] = lpSrc[0] + 1; lpSrc[2] = lpSrc[1] + 1;
				lpSrc[6] = lpSrc[3] + m_stBitMap.bmWidth; lpSrc[7] = lpSrc[6] + 1; lpSrc[8] = lpSrc[7] + 1;

				// Hough��ּ��
				sDTemp = abs( lpSrc[2]->rgbReserved - lpSrc[0]->rgbReserved + lpSrc[5]->rgbReserved - lpSrc[3]->rgbReserved + lpSrc[8]->rgbReserved - lpSrc[6]->rgbReserved ) +
					abs( lpSrc[5]->rgbReserved - lpSrc[1]->rgbReserved + lpSrc[8]->rgbReserved - lpSrc[0]->rgbReserved + lpSrc[7]->rgbReserved - lpSrc[3]->rgbReserved ) +
					abs( lpSrc[6]->rgbReserved - lpSrc[0]->rgbReserved + lpSrc[7]->rgbReserved - lpSrc[1]->rgbReserved + lpSrc[8]->rgbReserved - lpSrc[2]->rgbReserved ) +
					abs( lpSrc[3]->rgbReserved - lpSrc[1]->rgbReserved + lpSrc[6]->rgbReserved - lpSrc[2]->rgbReserved + lpSrc[7]->rgbReserved - lpSrc[5]->rgbReserved );

				// ��ֵ��
				if( sDTemp < lThre )
					*pbyMem = 0;
				else
					*pbyMem = 255;

				pbyMem ++;
			}
			// �ճ����1��
			pbyMem ++;
		}
		// �ճ����һ��
	}

	return TRUE;
}

// ����
BOOL CImgProcess::Expand()
{
	// ����32λ����
	if( m_stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// �� �ɰ� �ڴ�
	ZeroMemory( m_stEdgeDetect.pbyFilte, RESO_XY );

	// ��ʼ����
	LPBYTE lpSrc[4] = { NULL };
	LPBYTE lpDst = NULL;
	long lPixs = 0;
	for( long i = 1; i < m_stBitMap.bmHeight - 1; i++ )	// ��(��ȥ��Ե����)
	{
		lPixs = i*m_stBitMap.bmWidth;
		for( long j = 1; j < m_stBitMap.bmWidth - 1; j++ )	// ��(��ȥ��β����)
		{
			// ָ��Ŀ��ͼ���� ����( lc, lr) ��ָ��
			lpDst = m_stEdgeDetect.pbyFilte + lPixs + j;

			// ��ȡ ��Եͼ������( j, i ) �˲�������
			lpSrc[1] = m_stEdgeDetect.pbyEgDtc + lPixs + j - 1;
			lpSrc[2] = lpSrc[1] + 2;
			lpSrc[3] = lpSrc[2] + m_stBitMap.bmWidth - 1;
			lpSrc[0] = lpSrc[2] - m_stBitMap.bmWidth - 1;

			// ���ͼ�⣨��������һ���׾Ͱף�
			if ( *lpSrc[0] || *lpSrc[1] || *lpSrc[2] || *lpSrc[3] )
				*lpDst = 255;
		}
	}

	return TRUE;
}

// �����ͨ����
BOOL CImgProcess::MaxConDo()
{
	// ����32λ����
	if( m_stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// �ܹ���ǵ����� �� ֵ
	BYTE byMaking = 255;

	m_stEdgeDetect.wFlagNum = 0;
	m_stEdgeDetect.wEqNum = 0;
	ZeroMemory( m_stEdgeDetect.pwFlag, RESO_XY*sizeof(WORD) );
	//==========================��һ�α�ǿ�ʼ==========================
	{
		// ���ͼ��ĵ�һ�С���һ�е�����(ֻ����һ������)
		if ( m_stEdgeDetect.pbyFilte[0] == byMaking )
			m_stEdgeDetect.pwFlag[0] = ++m_stEdgeDetect.wFlagNum;

		// ���ͼ��ĵ�һ�У���ʱ������ֵȼ۵����
		for ( long i = 1; i < m_stBitMap.bmWidth; i++ )
		{
			// ��Ҫ��ǵ����
			if ( m_stEdgeDetect.pbyFilte[ i ] == byMaking )
			{
				// ǰ��û�б���ǹ�����ʼһ���µı��
				if ( m_stEdgeDetect.pwFlag[ i - 1 ] == 0 )
					m_stEdgeDetect.pwFlag[ i ] = ++m_stEdgeDetect.wFlagNum;
				// ǰ�汻��ǹ��������ǰһ�����
				else
					m_stEdgeDetect.pwFlag[ i ] = m_stEdgeDetect.pwFlag[ i - 1 ];
			}
		}

		// ����һ��֮��ı�ǣ���ʱ����ֵȼ۵Ĺ�ϵ
		for ( long j = 1; j < m_stBitMap.bmHeight; j++ )
		{
			// ��j��ǰ���������
			long lPixs = j*m_stBitMap.bmWidth;

			// �ȶԸ��еĵ�һ����������ֻ��Ҫ�����ϣ����������㣬���������ܴ��ڵȼ۶�
			if ( m_stEdgeDetect.pbyFilte[ lPixs ] == byMaking )
			{
				// <��>λ�ñ���ǹ�
				if ( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ] != 0 )
				{
					// ����<��>���
					m_stEdgeDetect.pwFlag[ lPixs ] = m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ];
					// ���<��> != <����>����<����>�ѱ�ǣ����ڵȼ۶�
					if ( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ] != m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] &&
						m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] != 0 )
						// <��><����>�ȼ۱��
						AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ], m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] );
				}
				// <��>û�б�ǣ���ʱһ��������ڵȼ۹�ϵ
				else
				{
					if ( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] != 0 )
						m_stEdgeDetect.pwFlag[ lPixs ] = m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] != 0;   //����<����>���
					// <��>��<����>��û�б�ǣ���ʼ�µı��
					else
						m_stEdgeDetect.pwFlag[ lPixs ] = ++m_stEdgeDetect.wFlagNum;
				}
			}

			// ��ÿ�е��м������Ǵ�����ʱ����<��>��<����>��<��>��<����> 4�����
			for ( long i = 1; i < m_stBitMap.bmWidth - 1; i++ )
			{
				long lSself = lPixs + i;	// ��ǰ��
				long lLeft = lSself - 1;	// <��>
				long lLfTop = lLeft - m_stBitMap.bmWidth;	// <����>
				long lTop = lLfTop + 1;		// <��>
				long lRtTop = lTop + 1;		// <����>
				// ��Ҫ���
				if ( m_stEdgeDetect.pbyFilte[ lSself ] == byMaking )
				{
					// <��>����ǹ�
					if ( m_stEdgeDetect.pwFlag[ lLeft ] != 0 )
					{
						m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lLeft ];	//����<��>

						if ( m_stEdgeDetect.pwFlag[ lLfTop ] != 0 && m_stEdgeDetect.pwFlag[ lLeft ] != m_stEdgeDetect.pwFlag[ lLfTop ] )
							// ���<��>��<����>�ȼ�
							AddEqualMark( m_stEdgeDetect.pwFlag[ lLeft ], m_stEdgeDetect.pwFlag[ lLfTop ] );

						if ( m_stEdgeDetect.pwFlag[ lTop ] != 0 && m_stEdgeDetect.pwFlag[ lLeft ] != m_stEdgeDetect.pwFlag[ lTop ] )
							//���<��>��<��>�ȼ�
							AddEqualMark( m_stEdgeDetect.pwFlag[ lLeft], m_stEdgeDetect.pwFlag[ lTop ] );

						if ( m_stEdgeDetect.pwFlag[ lRtTop ] != 0 && m_stEdgeDetect.pwFlag[ lLeft ] != m_stEdgeDetect.pwFlag[ lRtTop ] )
							//���<��>��<����>�ȼ�
							AddEqualMark( m_stEdgeDetect.pwFlag[ lLeft ], m_stEdgeDetect.pwFlag[ lRtTop ] );
					}
					// <��>δ����ǹ�
					else
					{
						// <����>����ǹ�
						if ( m_stEdgeDetect.pwFlag[ lLfTop ] != 0 )
						{
							m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lLfTop ];	// ����<����>

							if ( m_stEdgeDetect.pwFlag[ lTop ] != 0 && m_stEdgeDetect.pwFlag[ lLfTop ] != m_stEdgeDetect.pwFlag[ lTop ] )
								//���<����>��<��>�ȼ�
								AddEqualMark( m_stEdgeDetect.pwFlag[ lLfTop ], m_stEdgeDetect.pwFlag[ lTop ] );

							if ( m_stEdgeDetect.pwFlag[ lRtTop ] != 0 && m_stEdgeDetect.pwFlag[ lLfTop ] != m_stEdgeDetect.pwFlag[ lRtTop ] )
								//���<����>��<����>�ȼ�
								AddEqualMark( m_stEdgeDetect.pwFlag[ lLfTop ], m_stEdgeDetect.pwFlag[ lRtTop ] );
						}
						// <��>��<����>δ��ǹ�
						else
						{
							if ( m_stEdgeDetect.pwFlag[ lTop ] !=0 )
							{
								m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lTop ];	//����<��>���

								if ( m_stEdgeDetect.pwFlag[ lRtTop ] != 0 && m_stEdgeDetect.pwFlag[ lTop ] != m_stEdgeDetect.pwFlag[ lRtTop ] )
									// ���<��>��<����>�ȼ�
									AddEqualMark( m_stEdgeDetect.pwFlag[ lTop ], m_stEdgeDetect.pwFlag[ lRtTop ] );
							}
							// <��>��<����>��<��>δ��ǹ�����ʱ�����ڵȼ۹�ϵ
							else
							{
								if ( m_stEdgeDetect.pwFlag[ lRtTop ] !=0 )
									m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lRtTop ];	//����<����>���
								// <��>��<����>��<��>��<����>δ��ǹ�����ʼ�µı��ֵ
								else
									m_stEdgeDetect.pwFlag[ lSself ] = ++m_stEdgeDetect.wFlagNum;
							}
							// <��>��<����>��<��>δ��ǹ�����
						}
						// <��>��<����>δ��ǹ�����
					}
					// <��>δ����ǹ�����
				}
				// else ����Ҫ���
			}
			// �м�㴦��Ľ���

			// ���Ը��е����һ������������ʱ����<��>��<����>��<��> 3�����
			// ��Ҫ���
			if ( m_stEdgeDetect.pbyFilte[ lPixs + m_stBitMap.bmWidth - 1 ] == byMaking )
			{
				// <��>����ǹ�
				if ( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ] != 0 )
				{
					m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ];	// ����<��>

					if ( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ] != m_stEdgeDetect.pwFlag[ lPixs - 2 ] &&
						m_stEdgeDetect.pwFlag[ lPixs - 2 ] != 0 )
						// ���<��>��<����>�ȼ�
						AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ], m_stEdgeDetect.pwFlag[ lPixs - 2 ] );

					if ( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ] != m_stEdgeDetect.pwFlag[ lPixs - 1 ] &&
						m_stEdgeDetect.pwFlag[ lPixs - 1 ] != 0 )
						// ���<��>��<��>�ȼ�
						AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ], m_stEdgeDetect.pwFlag[ lPixs - 1 ] );
				}
				// <��>δ����ǹ�
				else
				{
					if ( m_stEdgeDetect.pwFlag[ lPixs - 2 ] != 0 )
					{
						m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = m_stEdgeDetect.pwFlag[ lPixs - 2 ];	//����<����>

						if ( m_stEdgeDetect.pwFlag[ lPixs - 2 ] != m_stEdgeDetect.pwFlag[ lPixs - 1 ] && m_stEdgeDetect.pwFlag[ lPixs - 1 ] != 0 )
							// ���<����>��<��>�ȼ�
							AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs  - 2 ], m_stEdgeDetect.pwFlag[ lPixs - 1 ] );
					}
					// <��>��<����>δ��ǹ�����ʱ�����ڵȼ۶�
					else
					{
						if ( m_stEdgeDetect.pwFlag[ lPixs - 1 ] != 0 )
							m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = m_stEdgeDetect.pwFlag[ lPixs - 1 ];	//����<��>���
						//<��>��<����>��<��>δ��ǹ�,��ʼ�µı��ֵ
						else
							m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = ++m_stEdgeDetect.wFlagNum;
					}
				}
			}
			// ��ÿ�е����һ���� ���� ����
		}
		// "����һ��֮��ı��"�Ľ���
	}
	if ( m_stEdgeDetect.wFlagNum == NULL )
		return FALSE;
	//==========================��һ�α�ǽ���==========================
	// ͼ��������
	long lPixNum = m_stBitMap.bmHeight*m_stBitMap.bmWidth;
	if ( m_stEdgeDetect.wEqNum )
	{
		// ������
		m_stEdgeDetect.wFlagNum = AdjustEMak();
		if ( m_stEdgeDetect.wFlagNum == NULL )
			return FALSE;

		// �ڶ��α��
		for( long i = 0; i < lPixNum; ++i )
		{
			WORD& wFlag = m_stEdgeDetect.pwFlag[i];
			if ( wFlag )
				wFlag = m_stEdgeDetect.punEq[ wFlag - 1 ].wMarkB;
		}
	}
	// �±��ͳ�����,��������������򣬲���������������
	{
		// ��һͳ�Ʊ������������m_stEdgeDetect.punEq��
		ZeroMemory( m_stEdgeDetect.punEq, 65535*sizeof(UN_EQUAL_MARK) );
		LPDWORD lpdwAera = (LPDWORD)m_stEdgeDetect.punEq;
		for( long j = 0; j < lPixNum; ++j )
		{
			WORD& wFlag = m_stEdgeDetect.pwFlag[j];
			if ( wFlag )
				lpdwAera[ wFlag - 1 ] += 1;
		}
		// �����������ı��
		DWORD dwAeraMax = 0;		// ����������
		WORD wMaxIndex[2] = { 0 };	// �������ı��
		for ( WORD i = 1; i <= m_stEdgeDetect.wFlagNum; ++i )
		{
			if( dwAeraMax < lpdwAera[ i - 1 ] )
			{
				dwAeraMax = lpdwAera[ i - 1 ];
				wMaxIndex[1] = wMaxIndex[0];
				wMaxIndex[0] = i;
			}
		}
		if( wMaxIndex[1] == NULL )
			wMaxIndex[1] = wMaxIndex[0];
		// ��������������
		//LPRGBQUAD lpDst = m_stEdgeDetect.lpRGB;
		//RGBQUAD black = { 0, 0, 0, 0 }, white = { 255, 255, 255, 0 };
		for ( long i = 0; i < lPixNum; ++i )
		{
			if ( m_stEdgeDetect.pwFlag[i] != wMaxIndex[0] && m_stEdgeDetect.pwFlag[i] != wMaxIndex[1] )
			{
				m_stEdgeDetect.pbyFilte[i] = 0;
				//*lpDst = black;
			}
			//else
			//	*lpDst = white;
			//++lpDst;
		}
	}

	return TRUE;
}

// ��ӵȼ۶�
BOOL CImgProcess::AddEqualMark( WORD a, WORD b )
{
	// ��Ŀ����
	if( m_stEdgeDetect.wEqNum == 65535 )
	{
		AfxMessageBox( _T("�ȼ۶���Ŀ����֧�ַ�Χ��\n") );
		return FALSE;
	}

	// �Ƿ�����һ���ȼ۶���ͬ
	if ( m_stEdgeDetect.wEqNum &&
		( m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkA == a &&
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkB == b ) ||
		( m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkA ==  b &&
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkB == a ) )
		return TRUE;

	// �����µĵȼ۶ԣ�С��ΪA�����ΪB
	if( a < b )
	{
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum ].wMarkA = a;
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum ].wMarkB = b;
		m_stEdgeDetect.wEqNum ++;
	}
	else if( a > b )
	{
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum ].wMarkA = b;
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum ].wMarkB = a;
		m_stEdgeDetect.wEqNum ++;
	}
	else
		return FALSE;

	return TRUE;
}

// �����ȼ۶�
WORD CImgProcess::AdjustEMak()
{
	// �ȼ۶�����
	std::sort( m_stEdgeDetect.punEq, m_stEdgeDetect.punEq + m_stEdgeDetect.wEqNum, &CImgProcess::MarkCompare );
	// ȥ���м���
	for( WORD j = 0; j < m_stEdgeDetect.wEqNum; j++ )
	{
		WORD A = m_stEdgeDetect.punEq[j].wMarkA;
		WORD B = m_stEdgeDetect.punEq[j].wMarkB;
		for( WORD i = 0; i < m_stEdgeDetect.wEqNum; i++ )
		{
			WORD& a = m_stEdgeDetect.punEq[i].wMarkA;
			WORD& b = m_stEdgeDetect.punEq[i].wMarkB;
			if( a == B )
			{
				a = A;
			}
			else if ( b == B && a != A )
			{
				if( a <= A )
					b = A;
				else
				{
					b = a;
					a = A;
				}
			}
		}
	}
	// �ȼ۶��ٴ�����
	std::sort( m_stEdgeDetect.punEq, m_stEdgeDetect.punEq + m_stEdgeDetect.wEqNum, &CImgProcess::MarkCompare );
	// ȥ���ظ��ȼ۶�
	WORD wIndex = 0;
	for( WORD i = 1; i < m_stEdgeDetect.wEqNum; i++ )
	{
		if( m_stEdgeDetect.punEq[i].dwMark != m_stEdgeDetect.punEq[wIndex].dwMark )
			m_stEdgeDetect.punEq[++wIndex].dwMark = m_stEdgeDetect.punEq[i].dwMark;
	}
	m_stEdgeDetect.wEqNum = wIndex + 1;
	// �����ڶ��α��
	wIndex = 0;
	for( WORD wA = 0, wNum = m_stEdgeDetect.wEqNum, i = 0; i < wNum; i++ )
	{
		if( wA < m_stEdgeDetect.punEq[i].wMarkA )
		{
			wIndex ++;
			wA = m_stEdgeDetect.punEq[i].wMarkA;
			// ������
			m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkA = wIndex;
			m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkB = wA;

			m_stEdgeDetect.wEqNum ++;
		}
		if( wA == m_stEdgeDetect.punEq[i].wMarkA )
			m_stEdgeDetect.punEq[i].wMarkA = wIndex;
	}
	// ��Ӳ��ǵȼ۶Եı��
	for ( WORD wNum = m_stEdgeDetect.wEqNum, i = 1; i <= m_stEdgeDetect.wFlagNum; i++ )
	{
		for( WORD j = 0; j < wNum; j++ )
		{
			// �ǵȼ۱�ǣ�����
			if ( m_stEdgeDetect.punEq[j].wMarkB == i )
				break;
			// û�ҵ������ǵȼ۱��
			if( j == wNum - 1 )
			{
				// �����±��
				wIndex ++;
				m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkA = wIndex;
				m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkB = i;

				m_stEdgeDetect.wEqNum ++;
				break;
			}
		}
	}
	// ����A��B��֮��A���ϱ�ǣ�B���±��
	for ( WORD wT = 0, i = 0; i < m_stEdgeDetect.wEqNum; i++ )
	{
		wT = m_stEdgeDetect.punEq[i].wMarkA;
		m_stEdgeDetect.punEq[i].wMarkA = m_stEdgeDetect.punEq[i].wMarkB;
		m_stEdgeDetect.punEq[i].wMarkB = wT;
	}
	// �����ϱ������ʹ�ϱ�Ǵ�1��ʼ����һ��ȥ���Է����������±����
	std::sort( m_stEdgeDetect.punEq, m_stEdgeDetect.punEq + m_stEdgeDetect.wEqNum, &CImgProcess::MarkCompare );

	// �����±����
	return wIndex;
}

// ��ǱȽ�
bool CImgProcess::MarkCompare( UN_EQUAL_MARK& a, UN_EQUAL_MARK& b )
{
	return ( a.dwMark < b.dwMark );
}

// ֱ�����
BOOL CImgProcess::LineFitting( ST_EDGE_PARA& stEdgePara )
{
	// Ѱ��4���ǵ�
	{
		int iCorner[4] = {0}, iDistance = 0;
		int iDisTemp[4] = {0};
		// ��ʼ��Ϊ��Զ�ľ���
		iCorner[3] = iCorner[2] = iCorner[1] = iCorner[0] = m_stBitMap.bmHeight*m_stBitMap.bmHeight + m_stBitMap.bmWidth*m_stBitMap.bmWidth;
		long lPixs = 0;
		for( short i = 0; i < m_stBitMap.bmHeight; i++ )
		{
			lPixs = i*m_stBitMap.bmWidth;
			for( short j = 0; j < m_stBitMap.bmWidth; j++ )
			{
				if ( m_stEdgeDetect.pbyFilte[ lPixs + j ] == 255 )
				{
					iDisTemp[0] = i*i;
					iDisTemp[1] = j*j;
					iDisTemp[2] = ( m_stBitMap.bmHeight - 1 - i )*( m_stBitMap.bmHeight - 1 - i );
					iDisTemp[3] = ( m_stBitMap.bmWidth - 1 - j )*( m_stBitMap.bmWidth - 1 - j );
					// 0 ��� ���ϵ㣬��ͼ�� ���ϵ� ����ĵ�
					iDistance = iDisTemp[0] + iDisTemp[1];
					if( iCorner[0] > iDistance )
					{
						iCorner[0] = iDistance;
						stEdgePara.point[0].x = j;
						stEdgePara.point[0].y = i;
					}
					// 1 ���ϵ�
					iDistance = iDisTemp[0] + iDisTemp[3];
					if( iCorner[1] > iDistance )
					{
						iCorner[1] = iDistance;
						stEdgePara.point[1].x = j;
						stEdgePara.point[1].y = i;
					}
					// 2 ���µ�
					iDistance = iDisTemp[2] + iDisTemp[1];
					if( iCorner[2] > iDistance )
					{
						iCorner[2] = iDistance;
						stEdgePara.point[2].x = j;
						stEdgePara.point[2].y = i;
					}
					// 3 ���µ�
					iDistance = iDisTemp[2] + iDisTemp[3];
					if( iCorner[3] > iDistance )
					{
						iCorner[3] = iDistance;
						stEdgePara.point[3].x = j;
						stEdgePara.point[3].y = i;
					}
				}
			}
		}
	}
	// ����ĸ���Ե
	{
		// ��Ե�㼯
		PST_POINT pstPoints = (PST_POINT)m_stEdgeDetect.pwFlag;
		// ��Ե�������α�
		int iId = 0;
		// ����м����
		int iDisTemp[4] = { ( stEdgePara.point[0].y + stEdgePara.point[1].y )/2,
			( stEdgePara.point[2].y + stEdgePara.point[3].y )/2,
			( stEdgePara.point[0].x + stEdgePara.point[2].x )/2,
			( stEdgePara.point[1].x + stEdgePara.point[1].x )/2 };
		double kx = 0, ky = 0, kx2 = 0, ky2 = 0, kxy = 0;
		// ��������
		int iPixs = 0;
		// ===================�ϱ�Ե��� y = a0x + b0;
		for( int x = stEdgePara.point[0].x; x <= stEdgePara.point[1].x; x += 5 )
		{
			iPixs = x;
			for( int y = 0; y < m_stBitMap.bmHeight; y++ )
			{
				if( m_stEdgeDetect.pbyFilte[ iPixs ] == 255 )
				{
					if( abs( y - iDisTemp[0] ) < 20 )
					{
						pstPoints[ iId ].x = x;
						pstPoints[ iId ].y = y;
						iId++;
					}
					break;
				}
				iPixs += m_stBitMap.bmWidth;
			}
		}
		if( iId > 0 )
		{
			for( int i = 0; i < iId; i++ )
			{
				kx += pstPoints[i].x;
				ky += pstPoints[i].y;
				kx2 += pstPoints[i].x*pstPoints[i].x;
				kxy += pstPoints[i].x*pstPoints[i].y;
			}
			stEdgePara.line[0].a = ( kx*ky - iId*kxy )/( kx*kx - iId*kx2 );
			stEdgePara.line[0].b = ( ( ky - stEdgePara.line[0].a*kx )/iId ) + 3;
		}
		// ===================�±�Ե��� y =a1x + b1;
		iId = 0;
		for( int x = stEdgePara.point[2].x; x <= stEdgePara.point[3].x; x += 5 )
		{
			iPixs = ( m_stBitMap.bmHeight - 1 )*m_stBitMap.bmWidth + x;
			for( int y = m_stBitMap.bmHeight - 1; y >= 0; y-- )
			{
				if( m_stEdgeDetect.pbyFilte[ iPixs ] == 255 )
				{
					if( abs( y - iDisTemp[1] ) < 20 )
					{
						pstPoints[ iId ].x = x;
						pstPoints[ iId ].y = y;
						iId++;
					}
					break;
				}
				iPixs -= m_stBitMap.bmWidth;
			}
		}
		kx = ky = kx2 = kxy = 0;
		if( iId > 0 )
		{
			for( int i = 0; i < iId; i++ )
			{
				kx += pstPoints[i].x;
				ky += pstPoints[i].y;
				kx2 += pstPoints[i].x*pstPoints[i].x;
				kxy += pstPoints[i].x*pstPoints[i].y;
			}
			stEdgePara.line[1].a = ( kx*ky - iId*kxy )/( kx*kx - iId*kx2 );
			stEdgePara.line[1].b = ( ( ky - stEdgePara.line[1].a*kx )/iId ) - 3;
		}
		// ===================���Ե��� x = a2y + b2;
		iId = 0;
		for( int y = stEdgePara.point[0].y; y <= stEdgePara.point[2].y; y += 5 )
		{
			iPixs = y*m_stBitMap.bmWidth;
			for( int x = 0; x < m_stBitMap.bmWidth; x++ )
			{
				if( m_stEdgeDetect.pbyFilte[ iPixs ] == 255 )
				{
					if( abs( x - iDisTemp[2] ) < 20 )
					{
						pstPoints[ iId ].x = x;
						pstPoints[ iId ].y = y;
						iId++;
					}
					break;
				}
				iPixs ++;
			}
		}
		kx = ky = ky2 = kxy = 0;
		if( iId > 0 )
		{
			for( int i = 0; i < iId; i++ )
			{
				kx += pstPoints[i].x;
				ky += pstPoints[i].y;
				ky2 += pstPoints[i].y*pstPoints[i].y;
				kxy += pstPoints[i].x*pstPoints[i].y;
			}
			stEdgePara.line[2].a = ( kx*ky - iId*kxy )/( ky*ky - iId*ky2 );
			stEdgePara.line[2].b = ( ( kx - stEdgePara.line[2].a*ky )/iId ) + 3;
		}
		// ===================�ұ�Ե��� x = a3y + b3;
		iId = 0;
		for( int y = stEdgePara.point[1].y; y <= stEdgePara.point[3].y; y += 5 )
		{
			iPixs = ( y + 1 )*m_stBitMap.bmWidth - 1;
			for( int x = m_stBitMap.bmWidth - 1; x >= 0; x-- )
			{
				if( m_stEdgeDetect.pbyFilte[ iPixs ] == 255 )
				{
					if( abs( x - iDisTemp[3] ) < 20 )
					{
						pstPoints[ iId ].x = x;
						pstPoints[ iId ].y = y;
						iId++;
					}
					break;
				}
				iPixs --;
			}
		}
		kx = ky = ky2 = kxy = 0;
		if( iId > 0 )
		{
			for( int i = 0; i < iId; i++ )
			{
				kx += pstPoints[i].x;
				ky += pstPoints[i].y;
				ky2 += pstPoints[i].y*pstPoints[i].y;
				kxy += pstPoints[i].x*pstPoints[i].y;
			}
			stEdgePara.line[3].a = ( kx*ky - iId*kxy )/( ky*ky - iId*ky2 );
			stEdgePara.line[3].b = ( ( kx - stEdgePara.line[3].a*ky )/iId ) - 3;
		}
	}
	// ���¼����ĸ��ǵ�
	{
		// ���ϵ�
		stEdgePara.point[0].y = int( ( stEdgePara.line[0].a*stEdgePara.line[2].b + stEdgePara.line[0].b )/( 1 - stEdgePara.line[0].a*stEdgePara.line[2].a ) );
		stEdgePara.point[0].x = int( stEdgePara.line[2].a*stEdgePara.point[0].y + stEdgePara.line[2].b );
		// ���ϵ�
		stEdgePara.point[1].y = int( ( stEdgePara.line[0].a*stEdgePara.line[3].b + stEdgePara.line[0].b )/( 1 - stEdgePara.line[0].a*stEdgePara.line[3].a ) );
		stEdgePara.point[1].x = int( stEdgePara.line[3].a*stEdgePara.point[1].y + stEdgePara.line[3].b );
		// ���µ�
		stEdgePara.point[2].y = int( ( stEdgePara.line[1].a*stEdgePara.line[2].b + stEdgePara.line[1].b )/( 1 - stEdgePara.line[1].a*stEdgePara.line[2].a ) );
		stEdgePara.point[2].x = int( stEdgePara.line[2].a*stEdgePara.point[2].y + stEdgePara.line[2].b );
		// ���µ�
		stEdgePara.point[3].y = int( ( stEdgePara.line[1].a*stEdgePara.line[3].b + stEdgePara.line[1].b )/( 1 - stEdgePara.line[1].a*stEdgePara.line[3].a ) );
		stEdgePara.point[3].x = int( stEdgePara.line[3].a*stEdgePara.point[3].y + stEdgePara.line[3].b );
	}
	return TRUE;
}

// ��ȡ�ǵ�������
BOOL CImgProcess::ValidArea( PST_PROC_DATA pstProcData )
{
	ST_EDGE_PARA& stEdgePara = *(pstProcData->pstEdgePara);
	// У���鱾�߿��ĸ��ǵ����Ч��
	bool& bValid = stEdgePara.bValid;
	bValid = true;
	bValid = bValid && ( stEdgePara.point[0].x >= 0 );
	bValid = bValid && ( stEdgePara.point[0].y >= 0 );
	bValid = bValid && ( stEdgePara.point[1].x < m_stBitMap.bmWidth );
	bValid = bValid && ( stEdgePara.point[1].y >= 0 );
	bValid = bValid && ( stEdgePara.point[2].x >= 0 );
	bValid = bValid && ( stEdgePara.point[2].y < m_stBitMap.bmHeight );
	bValid = bValid && ( stEdgePara.point[3].x < m_stBitMap.bmWidth );
	bValid = bValid && ( stEdgePara.point[3].y < m_stBitMap.bmHeight );
	bValid = bValid && ( stEdgePara.point[0].y < stEdgePara.point[2].y );
	bValid = bValid && ( stEdgePara.point[1].y < stEdgePara.point[3].y );
	bValid = bValid && ( stEdgePara.point[0].x < stEdgePara.point[1].x );
	bValid = bValid && ( stEdgePara.point[2].x < stEdgePara.point[3].x );
	if ( !bValid )
		return FALSE;

	// ������Ͻ����������ͨ��
	for( int x = 0, y = 0; x < m_stBitMap.bmWidth; x++ )
	{
		// �ϱ�Ե
		if( x >= stEdgePara.point[0].x && x <= stEdgePara.point[1].x )
		{
			// ����y
			y = (int)( stEdgePara.line[0].a*x + stEdgePara.line[0].b );
			// �������Ҷ�128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
		// �±�Ե
		if( x >= stEdgePara.point[2].x && x <= stEdgePara.point[3].x )
		{
			// ����y
			y = (int)( stEdgePara.line[1].a*x + stEdgePara.line[1].b );
			// �������Ҷ�128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
	}
	for( int x = 0, y = 0; y < m_stBitMap.bmHeight; y++ )
	{
		// ���Ե
		if ( y >= stEdgePara.point[0].y && y <= stEdgePara.point[2].y )
		{
			// ����x
			x = (int)( stEdgePara.line[2].a*y + stEdgePara.line[2].b );
			// �������Ҷ�128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
		// �ұ�Ե
		if ( y >= stEdgePara.point[1].y && y <= stEdgePara.point[3].y )
		{
			// ����x
			x = (int)( stEdgePara.line[3].a*y + stEdgePara.line[3].b );
			// �������Ҷ�128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
	}
	// �����ͨ������ΪԴͼ����ɰ�
	int iPixs = 0, x1 = 0, x2 = 0;
	bool bLeft = true, bRight = true;
	for( int y = 0; y < m_stBitMap.bmHeight; y++ )
	{
		iPixs = y*m_stBitMap.bmWidth;
		x1 = 0, x2 = m_stBitMap.bmWidth - 1;
		bLeft = true, bRight = true;
		while( true )
		{
			// ���
			if ( bLeft )
			{
				if ( pstProcData->lpMask[ iPixs + x1 ] == 128 )
					bLeft = false;
				else
					x1 ++;
			}
			if ( bRight )
			{
				// �Ҷ�
				if( pstProcData->lpMask[ iPixs + x2 ] == 128 )
					bRight = false;
				else
					x2 --;
			}
			// ����
			if( !( bLeft || bRight ) )
			{
				// ��x1 x2֮�����Ҷ�128
				memset( pstProcData->lpMask + iPixs + x1, 128, x2 - x1 + 1 );
				break;
			}
			else if( x1 + 1 >= x2 )
				break;
		}
	}
	return TRUE;
}

// Harris�ǵ���
BOOL CImgProcess::Harris( VEC_CORNER* pvecCorner )
{
	// ����ͼ��:Cim
	{
		ZeroMemory( m_stCornerDtec.Cim, RESO_XY*sizeof(double) );
		LPRGBQUAD lpRgbSrc = NULL;
		long lPixs = 0;
		for( long i = 0; i < m_stBitMap.bmHeight; i++ )
		{
			lPixs = i*m_stBitMap.bmWidth;
			for( long j = 0; j < m_stBitMap.bmWidth; j++ )
			{
				lpRgbSrc = m_stCornerDtec.lpRGB + lPixs + j;
				m_stCornerDtec.Cim[ lPixs + j ] = (double)( lpRgbSrc->rgbReserved );
			}
		}
	}

	// ����ˮƽ����ֱ���Ӷ�ÿ�������˲����Ix��Iy :Ix2 Iy2
	{
		// ����ˮƽ���������Ӳ���Ix(��ͨ�˲���
		double dHori[9]={ -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		Convolution( m_stCornerDtec.Cim, (int)m_stBitMap.bmWidth, (int)m_stBitMap.bmHeight,
			dHori, 3, 3,
			m_stCornerDtec.Ix2 );
		// ���崹ֱ���������Ӳ���Iy����ͨ�˲���
		double dVert[9]={ -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		Convolution( m_stCornerDtec.Cim, (int)m_stBitMap.bmWidth, (int)m_stBitMap.bmHeight,
			dVert, 3, 3,
			m_stCornerDtec.Iy2 );
	}

	// ���� Ix2 Ixy Iy2
	{
		double dIx = 0, dIy = 0;
		long lPixs = 0;
		for( long i = 0; i < m_stBitMap.bmHeight; i++ )
		{
			lPixs = i*m_stBitMap.bmWidth;
			for( long j = 0; j < m_stBitMap.bmWidth; j++ )
			{
				// �ɰ洦��
				if( m_stCornerDtec.pbyFilte[ lPixs + j ] == 128 )
				{
					dIx = m_stCornerDtec.Ix2[ lPixs + j ];
					dIy = m_stCornerDtec.Iy2[ lPixs + j ];
					m_stCornerDtec.Ix2[ lPixs + j ] = dIx*dIx;
					m_stCornerDtec.Ixy[ lPixs + j ] = dIx*dIy;
					m_stCornerDtec.Iy2[ lPixs + j ] = dIy*dIy;
				}
				else
				{
					m_stCornerDtec.Ix2[ lPixs + j ] = 0;
					m_stCornerDtec.Iy2[ lPixs + j ] = 0;
				}
			}
		}
	}

	// ��Ix2 Ixy Iy2 �ֱ���и�˹�˲�
	{
		// �����˹ģ�� 5*5
		double Gus[25];
		for( int i = 0; i < 5; i++ )
		{
			for( int j = 0; j < 5; j++ )
			{
				Gus[ i*5 + j ] = exp( -( (i-2)*(i-2)+(j-2)*(j-2) )/1.28 );
			}
		}
		// ��һ����ʹģ�����֮��Ϊ1����ʵ�˲�����ʡ�ԣ�
		double total = 0;
		for( int i = 0; i < 25; i++ )
			total += Gus[i];
		for( int i = 0; i < 25; i++ )
			Gus[ i ] /= total;

		Convolution( m_stCornerDtec.Ix2, (int)m_stBitMap.bmWidth, (int)m_stBitMap.bmHeight,
			Gus, 5, 5,
			m_stCornerDtec.Cim );
		double* lpTemp = m_stCornerDtec.Ix2;
		m_stCornerDtec.Ix2 = m_stCornerDtec.Cim;
		m_stCornerDtec.Cim = lpTemp;
		Convolution( m_stCornerDtec.Ixy, (int)m_stBitMap.bmWidth, (int)m_stBitMap.bmHeight,
			Gus, 5, 5,
			m_stCornerDtec.Cim );
		lpTemp = m_stCornerDtec.Ixy;
		m_stCornerDtec.Ixy = m_stCornerDtec.Cim;
		m_stCornerDtec.Cim = lpTemp;
		Convolution( m_stCornerDtec.Iy2, (int)m_stBitMap.bmWidth, (int)m_stBitMap.bmHeight,
			Gus, 5, 5,
			m_stCornerDtec.Cim );
		lpTemp = m_stCornerDtec.Iy2;
		m_stCornerDtec.Iy2 = m_stCornerDtec.Cim;
		m_stCornerDtec.Cim = lpTemp;
	}

	// ����Cim
	{
		double dIx2 = 0, dIxy = 0, dIy2 = 0;
		long lPixs = 0;
		for( long i = 0; i < m_stBitMap.bmHeight; i++ )
		{
			lPixs = i*m_stBitMap.bmWidth;
			for( long j = 0; j < m_stBitMap.bmWidth; j++ )
			{
				dIx2 = m_stCornerDtec.Ix2[ lPixs + j ];
				dIxy = m_stCornerDtec.Ixy[ lPixs + j ];
				dIy2 = m_stCornerDtec.Iy2[ lPixs + j ];
				// ע�⣺Ҫ�ڷ�ĸ�м���һ����С���Է�ֹ����Ϊ�����
				m_stCornerDtec.Cim[ lPixs + j ] = ( dIx2*dIy2 - dIxy*dIxy )/( dIx2 + dIy2 + 0.000001 );
			}
		}
	}

	//�ֲ�����ֵ���� :Ixy
	{
		// �����С 2*size+1 = 7*7
		int size = 3;

		// Ix2 ��Ϊ�ֲ���ֵ������
		ZeroMemory( m_stCornerDtec.Ixy, RESO_XY*sizeof(double) );

		double max = 0;
		for ( long i = size; i < m_stBitMap.bmHeight - size; i++ )
		{
			for ( long j = size; j < m_stBitMap.bmWidth - size; j++ )
			{
				max = -1000000;
				for ( int k = -size; k <= size; k++ )
				{
					for ( int t = -size; t <= size; t++ )
					{
						if ( m_stCornerDtec.Cim[ ( i + k )*m_stBitMap.bmWidth + ( j + t ) ] > max )
							max = m_stCornerDtec.Cim[ ( i + k )*m_stBitMap.bmWidth + ( j + t ) ];
					}
				}
				if ( max > 1e-5 )
					m_stCornerDtec.Ixy[ i*m_stBitMap.bmWidth + j ] = max;
			}
		}
	}

	// ȷ���ǵ�
	{
		// ��սǵ�����
		pvecCorner->clear();
		long lPixs = 0;
		ST_POINT pt;
		for ( long i = 0; i < m_stBitMap.bmHeight; i++ )
		{
			lPixs = i*m_stBitMap.bmWidth;
			for ( long j = 0; j < m_stBitMap.bmWidth; j++ )
			{
				if ( ( m_stCornerDtec.Cim[ lPixs + j ] == m_stCornerDtec.Ixy[ lPixs + j ] ) &&
					m_stCornerDtec.Cim[ lPixs + j ] > 1000 )
				{
					pt.x = (short)j;
					pt.y = (short)i;
					pvecCorner->push_back( pt );
				}
			}
		}
	}

	return TRUE;
}

// �������
BOOL CImgProcess::Convolution( double* src, int lmW, int lmH, double *tp, int tpW, int tpH, double* des )
{
	// ������ڴ�
	memset( des, 0, lmW*lmH*sizeof(double) );

#define src(x,y) src[lmW*(y)+(x)]
#define tp(x,y) tp[tpW*(y)+(x)]
#define des(x,y) des[lmW*(y)+(x)]

	// ��ʼ���
	double dResult;
	for( int i = tpH/2; i < lmH - tpH/2; i++ )
	{
		for( int j = tpW/2; j < lmW - tpW/2; j++ )
		{
			dResult = 0;
			for( int m = 0; m < tpH; m++ )
			{
				for( int n = 0; n < tpW; n++ )
					dResult += src( j + n - tpW/2, i + m - tpH/2 )*tp( n, m );
			}
			des( j, i ) = dResult;
		}
	}
	return TRUE;
}

/*************************************************************************
 * �������ƣ�
 *   InteEqualize()
 * ����:
 *    BITMAP& stBitMap	- ͼ�����ݽṹ
 * ����ֵ:
 *   BOOL               - �ɹ�����TRUE�����򷵻�FALSE��
 * ˵��:
 *   �ú���������ͼ�����ֱ��ͼ���⡣
 ************************************************************************/
BOOL CImgProcess::InteEqualize( BITMAP& stBitMap )
{
	// 32λͼ��
	if( stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// ������
	long lPixNum = stBitMap.bmHeight*stBitMap.bmWidth;

	// �Ҷ�ͳ�Ʊ�
	long lCount[256] = { 0 };
	// ��������Ҷ�ֵ�ļ���
	LPRGBQUAD lpSrc = NULL;
	for ( long i = 0; i < lPixNum; i ++ )
	{
		lpSrc = ((LPRGBQUAD)stBitMap.bmBits) + i;
		lCount[ lpSrc->rgbReserved ]++;
	}

	// �Ҷ�ӳ���
	BYTE bMap[256] = { 0 };
	// ����Ҷ�ӳ���
	long lTemp = 0;
	for ( long i = 0; i < 256; i++ )
	{
		lTemp += lCount[i];
		// �����Ӧ���»Ҷ�ֵ
		bMap[i] = (BYTE)( lTemp * 255/lPixNum );
	}

	for( long i = 0; i < lPixNum; i++ )
	{
		// ָ��DIB��i�У���j�����ص�ָ��
		lpSrc = ((LPRGBQUAD)stBitMap.bmBits) + i;
		// �����µĻҶ�ֵ
		lpSrc->rgbReserved = bMap[ lpSrc->rgbReserved ];
	}
	return TRUE;
}

/*************************************************************************
 * �������ƣ�
 *   MedianFilter()
 * ����:
 *   BITMAP& stBitMap			- ͼ�����ݽṹ
 *   ST_TEMPLATE& stTemPlate	- ģ��ṹ
 * ����ֵ:
 *   BOOL						- �ɹ�����TRUE�����򷵻�FALSE��
 * ˵��:
 *   �ú����� 32λ DIBͼ�������ֵ�˲���
 ************************************************************************/
BOOL CImgProcess::MedianFilter( BITMAP& stBitMap )
{
	// ����32λ����
	if( stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// ���˲��ڴ�
	ZeroMemory( m_stEdgeDetect.pbyFilte, RESO_Y*RESO_X );

	// ��ʼ��ֵ�˲�
	LPBYTE lpSrc[9] = { NULL };
	LPBYTE lpDst = NULL;
	long lPixs = 0;
	for( long i = 1; i < stBitMap.bmHeight - 1; i++ )	// ��(��ȥ��Ե����)
	{
		lPixs = i*stBitMap.bmWidth;
		for( long j = 1; j < stBitMap.bmWidth - 1; j++ )	// ��(��ȥ��β����)
		{
			// ָ���˲�ͼ���� ����( lc, lr) ��ָ��
			lpDst = m_stEdgeDetect.pbyFilte + lPixs + j;

			// ��ȡ ��Եͼ������( j, i ) �˲�������
			lpSrc[4] = m_stEdgeDetect.pbyEgDtc + lPixs + j; lpSrc[3] = lpSrc[4] - 1; lpSrc[5] = lpSrc[4] + 1;
			lpSrc[0] = lpSrc[3] - stBitMap.bmWidth; lpSrc[1] = lpSrc[0] + 1; lpSrc[2] = lpSrc[1] + 1;
			lpSrc[6] = lpSrc[3] + stBitMap.bmWidth; lpSrc[7] = lpSrc[6] + 1; lpSrc[8] = lpSrc[7] + 1;

			// ��ȡ��ֵ
			for( BYTE num = 0, m = 0; m < 9; m++ )
			{
				if( *(lpSrc[m]) == 255 )
					num ++;
				if( m == 8 && num > 4 )
					*lpDst = 255;
				else if( m == 8 )
					*lpDst = 0;
			}
		}
	}

	return TRUE;
}

// ��ʴ
BOOL CImgProcess::Corrosion( BITMAP& stBitMap )
{
	// ����32λ����
	if( stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// ���˲��ڴ�
	ZeroMemory( m_stEdgeDetect.pbyFilte, RESO_Y*RESO_X );

	// ��ʼ��ʴ
	LPBYTE lpSrc[4] = { NULL };
	LPBYTE lpDst = NULL;
	long lPixs = 0;
	for( long i = 1; i < stBitMap.bmHeight - 1; i++ )	// ��(��ȥ��Ե����)
	{
		lPixs = i*stBitMap.bmWidth;
		for( long j = 1; j < stBitMap.bmWidth - 1; j++ )	// ��(��ȥ��β����)
		{
			// ָ��Ŀ��ͼ���� ����( lc, lr) ��ָ��
			lpDst = m_stEdgeDetect.pbyFilte + lPixs + j;

			// ��ȡ ��Եͼ������( j, i ) �˲�������
			lpSrc[1] = m_stEdgeDetect.pbyEgDtc + lPixs + j - 1;
			lpSrc[2] = lpSrc[1] + 2;
			lpSrc[3] = lpSrc[2] + stBitMap.bmWidth - 1;
			lpSrc[0] = lpSrc[2] - stBitMap.bmWidth - 1;

			// ��ʴ��⣨������ȫ���ײŰף�
			if ( lpSrc[0] && lpSrc[1] && lpSrc[2] && lpSrc[3] )
				*lpDst = 255;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// ��ȡ�����߳�
UINT CImgProcess::ThdGetDataProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// �Ե�
		Sleep( SLEEP_TIME );

		// �ȴ������¼�
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETDATA ].hStart, INFINITE );

		// ����˳��¼�
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETDATA ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// ��������¼������û�������
		if ( ::WaitForSingleObject( pImgProc->m_hHasData, MAX_WAIT_DATA_TIME ) != WAIT_OBJECT_0 )
			continue;
		ResetEvent( pImgProc->m_hHasData );

		// û�л��������
		if ( pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.empty() )
			continue;

		// �Ӷ��л�ȡ����
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// ȡ���֡����
		pstProcData = pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.back();
		// ��֡��֡�������Ƴ�
		pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );

		// ��ȡ����
		pImgProc->GetData( pstProcData );

		// ��֡ѹ�� ��Ե ������
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );
	}

	// �������˳�
	TRACE( "ThdGetDataProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_GETDATA ].hHasExit );

	// ����
	return 0;
}

// ��Ե����߳�
UINT CImgProcess::ThdGetEdgeProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// �Ե�
		Sleep( SLEEP_TIME );

		// �ȴ������¼�
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETEDGE ].hStart, INFINITE );

		// ����˳��¼�
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETEDGE ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// û�л��������
		if ( pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.empty() )
			continue;

		// �Ӷ��л�ȡ����
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// ȡ���֡����
		pstProcData = pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.back();
		// ��֡��֡�������Ƴ�
		pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );

		// ��Ե���
		{
			// ���ñ�Ե����ڴ�ṹ
			{
				pImgProc->m_stEdgeDetect.lpRGB = pstProcData->lpRGB;
				pImgProc->m_stEdgeDetect.pbyEgDtc = pstProcData->pbyBufA;
				pImgProc->m_stEdgeDetect.pbyFilte = pstProcData->lpMask;
				pImgProc->m_stEdgeDetect.pwFlag = ( LPWORD )pstProcData->pbyBufB;
				pImgProc->m_stEdgeDetect.punEq = ( PUN_EQUAL_MARK )pstProcData->pbyBufC;
				pImgProc->m_stEdgeDetect.wFlagNum = 0;
				pImgProc->m_stEdgeDetect.wEqNum = 0;
			}
			// �����Ե���
			pImgProc->HoughEdge();
			// ����
			pImgProc->Expand();
			// �����ͨ����
			pImgProc->MaxConDo();
			// �������
			pImgProc->LineFitting( *(pstProcData->pstEdgePara) );
		}

		// ��֡ѹ�� �ǵ� ������
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );
	}

	// �������˳�
	TRACE( "ThdGetEdgeProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_GETEDGE ].hHasExit );

	// ����
	return 0;
}

// �ǵ����߳�
UINT CImgProcess::ThdGetCornerProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// �Ե�
		Sleep( SLEEP_TIME );

		// �ȴ������¼�
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETCORNER ].hStart, INFINITE );

		// ����˳��¼�
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETCORNER ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// û�л��������
		if ( pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.empty() )
			continue;

		// �Ӷ��л�ȡ����
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// ȡ���֡����
		pstProcData = pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.back();
		// ��֡��֡�������Ƴ�
		pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );

		// �ǵ���
		{
			// �ǵ����ڴ�ṹ
			{
				pImgProc->m_stCornerDtec.lpRGB = pstProcData->lpRGB;
				pImgProc->m_stCornerDtec.pbyFilte = pstProcData->lpMask;
				pImgProc->m_stCornerDtec.Ix2 = (double*)pstProcData->pbyBufA;
				pImgProc->m_stCornerDtec.Iy2 = (double*)pstProcData->pbyBufB;
				pImgProc->m_stCornerDtec.Ixy = (double*)pstProcData->pbyBufC;
				pImgProc->m_stCornerDtec.Cim = (double*)pstProcData->pbyBufD;
			}
			// ��ȡ �ǵ�������
			pImgProc->ValidArea( pstProcData );

			if ( pstProcData->pstEdgePara->bValid )
				// �ǵ���
				pImgProc->Harris( pstProcData->pVecCorner );
		}

		// ��֡ѹ�� ��ͼ ������
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );
	}

	// �������˳�
	TRACE( "ThdGetCornerProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_GETCORNER ].hHasExit );

	// ����
	return 0;
}

// ��ͼ�����߳�
UINT CImgProcess::ThdUpdataViewProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// �Ե�
		Sleep( SLEEP_TIME );

		// �ȴ������¼�
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].hStart, INFINITE );

		// ����˳��¼�
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// û�л��������
		if ( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.empty() )
			continue;

		// �Ӷ��л�ȡ����
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// ȡ���֡����
		pstProcData = pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.back();
		// ��֡��֡�������Ƴ�
		pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );

		// ���½���
		// ������ڿɼ�����ˢ����ʾ
		if( pImgProc->m_pEffectWnd->IsWindowVisible() )
		{
			pImgProc->m_pEffectWnd->RefreshData( pstProcData );
		}
		// ������ɼ�������־��Ϊ������
		else if( !pImgProc->m_pMainWnd->IsIconic() )
		{
			// ��ʾ�ر�
			pImgProc->m_pMainWnd->OnTbDirectShow();
		}

		// ��֡ѹ�� ��ȡ���� ������
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );
	}

	// �������˳�
	TRACE( "ThdUpdataViewProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].hHasExit );

	// ����
	return 0;
}
//////////////////////////////////////////////////////////////////////////

// �߳� ��ʼ��
BOOL CImgProcess::Init()
{
	// ��ȡ�����¼�
	m_hHasData = CreateEvent( NULL, TRUE, FALSE, NULL );

	// �̲߳�����ʼ��
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		// ���з��� �ٽ���
		InitializeCriticalSection( &m_staThdPara[i].crtDqOp );
		// ����
		m_staThdPara[i].dqCache.clear();

		// �¼������� �Ƿ��ֶ� ��ʼ״̬ ����
		m_staThdPara[i].hStart = CreateEvent( NULL, TRUE, FALSE, NULL );
		m_staThdPara[i].hCanExit = CreateEvent( NULL, TRUE, FALSE, NULL );
		m_staThdPara[i].hHasExit = CreateEvent( NULL, TRUE, FALSE, NULL );
	}

	// �����ڴ�
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		// ԭʼͼ��
		m_staProcData[i].lpRGB = new RGBQUAD[ RESO_XY ];
		// �ɰ�ͼ��
		m_staProcData[i].lpMask = new BYTE[ RESO_XY ];
		// �м��ڴ�A
		m_staProcData[i].pbyBufA = new BYTE[ RESO_XY*sizeof(double) ];
		// �м��ڴ�B
		m_staProcData[i].pbyBufB = new BYTE[ RESO_XY*sizeof(double) ];
		// �м��ڴ�C
		m_staProcData[i].pbyBufC = new BYTE[ RESO_XY*sizeof(double) ];
		// �м��ڴ�D
		m_staProcData[i].pbyBufD = new BYTE[ RESO_XY*sizeof(double) ];

		// ��Ե�����
		m_staProcData[i].pstEdgePara = new ST_EDGE_PARA;
		ZeroMemory( m_staProcData[i].pstEdgePara, sizeof(ST_EDGE_PARA) );
		// �ǵ�����
		m_staProcData[i].pVecCorner = new VEC_CORNER;
		m_staProcData[i].pVecCorner->clear();

		// �����ȡ���ݶ���
		m_staThdPara[THREAD_GETDATA].dqCache.push_front( &m_staProcData[i] );
	}

	// �����߳�
	// ��ȡ�����̣߳����ȼ�- 0
	m_staThdPara[ THREAD_GETDATA ].pThd = AfxBeginThread( ThdGetDataProc, (LPVOID)this );
	if ( m_staThdPara[ THREAD_GETDATA ].pThd == NULL )
		return FALSE;
	// ��Ե����̣߳����ȼ�- 2
	m_staThdPara[ THREAD_GETEDGE ].pThd = AfxBeginThread( ThdGetEdgeProc, (LPVOID)this, THREAD_PRIORITY_HIGHEST );
	if ( m_staThdPara[ THREAD_GETEDGE ].pThd == NULL )
		return FALSE;
	// �ǵ����̣߳����ȼ�- 2
	m_staThdPara[ THREAD_GETCORNER ].pThd = AfxBeginThread( ThdGetCornerProc, (LPVOID)this, THREAD_PRIORITY_HIGHEST );
	if ( m_staThdPara[ THREAD_GETCORNER ].pThd == NULL )
		return FALSE;
	// ������ͼ�̣߳����ȼ�- 0
	m_staThdPara[ THREAD_UPDATAVIEW ].pThd = AfxBeginThread( ThdUpdataViewProc, (LPVOID)this );
	if ( m_staThdPara[ THREAD_UPDATAVIEW ].pThd == NULL )
		return FALSE;

	return TRUE;
}

// �߳� ����
void CImgProcess::Start()
{
	TRACE( "Start Threads.\n" );
	// ����
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
		SetEvent( m_staThdPara[i].hStart );
}

// �߳� ��ͣ
void CImgProcess::Pause()
{
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
		ResetEvent( m_staThdPara[i].hStart );

	TRACE( "Pause Threads.\n" );
}

// �߳� �˳�
void CImgProcess::Exit()
{
	// ֪ͨ�˳�
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
		SetEvent( m_staThdPara[i].hCanExit );

	// �����߳�
	Start();

	// �ȴ������߳��˳�
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		WaitForSingleObject( m_staThdPara[i].hHasExit, INFINITE );
		
		// ��ն���
		m_staThdPara[i].dqCache.clear();
		// ɾ�� ���з��� �ٽ���
		DeleteCriticalSection( &m_staThdPara[i].crtDqOp );
		// �ر��¼�
		CloseHandle( m_staThdPara[i].hStart );
		CloseHandle( m_staThdPara[i].hCanExit );
		CloseHandle( m_staThdPara[i].hHasExit );
	}
	// �ر��������¼�
	CloseHandle( m_hHasData );

	// �ͷ��ڴ�
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		// ԭʼͼ��
		ReleaseArray( m_staProcData[i].lpRGB );
		// �ɰ�ͼ��
		ReleaseArray( m_staProcData[i].lpMask );
		// �м� A
		ReleaseArray( m_staProcData[i].pbyBufA );
		// �м� B
		ReleaseArray( m_staProcData[i].pbyBufB );
		// �м� C
		ReleaseArray( m_staProcData[i].pbyBufC );
		// �м� D
		ReleaseArray( m_staProcData[i].pbyBufD );

		// ��Ե��� ���
		ReleasePoint( m_staProcData[i].pstEdgePara );
		// �ǵ�����
		ReleasePoint( m_staProcData[i].pVecCorner );
	}

	TRACE( "Exit Threads.\n" );
}
