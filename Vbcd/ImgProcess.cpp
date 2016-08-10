
#include "StdAfx.h"

#include <math.h>
#include <algorithm>

#include "Vbcd.h"
#include "VbcdDlg.h"
#include "EffectDlg.h"
#include "ImgProcess.h"

#define EQ_SIZE	65535	// 等价对数组大小

CImgProcess::CImgProcess( CVbcdDlg* pMainWnd, CEffectDlg* pEffectWnd )
	: m_pMainWnd( pMainWnd )		// 主窗口
	, m_pEffectWnd( pEffectWnd )	// 显示窗口
{
	// 清图像信息
	ZeroMemory( &m_stBitMap, sizeof( BITMAP ) );
	// 边缘检测内存结构
	ZeroMemory( &m_stEdgeDetect, sizeof( m_stEdgeDetect ) );
	// 角点检测内存结构
	ZeroMemory( &m_stCornerDtec, sizeof( m_stCornerDtec ) );
}

CImgProcess::~CImgProcess(void)
{
}

// 获取数据
BOOL CImgProcess::GetData( PST_PROC_DATA pstProcData )
{
	// 图像的宽高
	long lbmW = m_stBitMap.bmWidth;
	long lbmH = m_stBitMap.bmHeight;
	// 像素数据
	LPRGBQUAD pstRgb = pstProcData->lpRGB;
	LPBYTE pbyPixel = m_lpVHdr->lpData;

	int iY = 0;
	int iU = 0;
	int iV = 0;
	float fT = 0.0f;
	// 获取数据，把捕获的图像转换为RGB图像，同时保留一份灰度图像
	for( long lr = 0; lr < lbmH; lr++ )
	{
		for( long lc = 0; lc < lbmW; lc ++ )
		{
			// 对YUY2格式图像取YUV分量
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

			// YUV->RGB，将YUY2格式图像转换为RGB格式图像
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

			// TUV->GRAY，保留一份灰度图像
			pstRgb->rgbReserved = *pbyPixel;

			// 偏移指针
			pbyPixel += 2;
			pstRgb ++;
		}
	}
	return TRUE;
}

// 霍夫边缘检测
BOOL CImgProcess::HoughEdge()
{
	// 不是32位返回
	if( m_stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// 清 边缘检测 内存
	ZeroMemory( m_stEdgeDetect.pbyEgDtc, RESO_XY );

	// 边缘检测
	{
		// 阈值
		long lThre = 100;
		// 空出第1行
		LPRGBQUAD lpSrc[9] = { NULL };

		LPBYTE pbyMem = m_stEdgeDetect.pbyEgDtc + m_stBitMap.bmWidth;
		short sDTemp = 0;
		for( long i = 1; i < m_stBitMap.bmHeight - 1; i++ )		// 每行
		{
			// 空出第1列
			pbyMem ++;
			for( long j = 1; j < m_stBitMap.bmWidth - 1; j++ )	// 每列
			{
				// 指向DIB第i行，第j个象素的指针
				lpSrc[4] = m_stEdgeDetect.lpRGB + i*m_stBitMap.bmWidth + j; lpSrc[3] = lpSrc[4] - 1; lpSrc[5] = lpSrc[4] + 1;
				lpSrc[0] = lpSrc[3] - m_stBitMap.bmWidth; lpSrc[1] = lpSrc[0] + 1; lpSrc[2] = lpSrc[1] + 1;
				lpSrc[6] = lpSrc[3] + m_stBitMap.bmWidth; lpSrc[7] = lpSrc[6] + 1; lpSrc[8] = lpSrc[7] + 1;

				// Hough差分检测
				sDTemp = abs( lpSrc[2]->rgbReserved - lpSrc[0]->rgbReserved + lpSrc[5]->rgbReserved - lpSrc[3]->rgbReserved + lpSrc[8]->rgbReserved - lpSrc[6]->rgbReserved ) +
					abs( lpSrc[5]->rgbReserved - lpSrc[1]->rgbReserved + lpSrc[8]->rgbReserved - lpSrc[0]->rgbReserved + lpSrc[7]->rgbReserved - lpSrc[3]->rgbReserved ) +
					abs( lpSrc[6]->rgbReserved - lpSrc[0]->rgbReserved + lpSrc[7]->rgbReserved - lpSrc[1]->rgbReserved + lpSrc[8]->rgbReserved - lpSrc[2]->rgbReserved ) +
					abs( lpSrc[3]->rgbReserved - lpSrc[1]->rgbReserved + lpSrc[6]->rgbReserved - lpSrc[2]->rgbReserved + lpSrc[7]->rgbReserved - lpSrc[5]->rgbReserved );

				// 二值化
				if( sDTemp < lThre )
					*pbyMem = 0;
				else
					*pbyMem = 255;

				pbyMem ++;
			}
			// 空出最后1列
			pbyMem ++;
		}
		// 空出最后一行
	}

	return TRUE;
}

// 膨胀
BOOL CImgProcess::Expand()
{
	// 不是32位返回
	if( m_stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// 清 蒙版 内存
	ZeroMemory( m_stEdgeDetect.pbyFilte, RESO_XY );

	// 开始膨胀
	LPBYTE lpSrc[4] = { NULL };
	LPBYTE lpDst = NULL;
	long lPixs = 0;
	for( long i = 1; i < m_stBitMap.bmHeight - 1; i++ )	// 行(除去边缘几行)
	{
		lPixs = i*m_stBitMap.bmWidth;
		for( long j = 1; j < m_stBitMap.bmWidth - 1; j++ )	// 列(除去首尾几列)
		{
			// 指向目的图像中 象素( lc, lr) 的指针
			lpDst = m_stEdgeDetect.pbyFilte + lPixs + j;

			// 获取 边缘图像象素( j, i ) 滤波器数组
			lpSrc[1] = m_stEdgeDetect.pbyEgDtc + lPixs + j - 1;
			lpSrc[2] = lpSrc[1] + 2;
			lpSrc[3] = lpSrc[2] + m_stBitMap.bmWidth - 1;
			lpSrc[0] = lpSrc[2] - m_stBitMap.bmWidth - 1;

			// 膨胀检测（四邻域有一个白就白）
			if ( *lpSrc[0] || *lpSrc[1] || *lpSrc[2] || *lpSrc[3] )
				*lpDst = 255;
		}
	}

	return TRUE;
}

// 最大连通域检测
BOOL CImgProcess::MaxConDo()
{
	// 不是32位返回
	if( m_stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// 能够标记的像素 的 值
	BYTE byMaking = 255;

	m_stEdgeDetect.wFlagNum = 0;
	m_stEdgeDetect.wEqNum = 0;
	ZeroMemory( m_stEdgeDetect.pwFlag, RESO_XY*sizeof(WORD) );
	//==========================第一次标记开始==========================
	{
		// 标记图象的第一行、第一列的象素(只有这一个象素)
		if ( m_stEdgeDetect.pbyFilte[0] == byMaking )
			m_stEdgeDetect.pwFlag[0] = ++m_stEdgeDetect.wFlagNum;

		// 标记图象的第一行，此时不会出现等价的情况
		for ( long i = 1; i < m_stBitMap.bmWidth; i++ )
		{
			// 需要标记的情况
			if ( m_stEdgeDetect.pbyFilte[ i ] == byMaking )
			{
				// 前面没有被标记过，则开始一个新的标记
				if ( m_stEdgeDetect.pwFlag[ i - 1 ] == 0 )
					m_stEdgeDetect.pwFlag[ i ] = ++m_stEdgeDetect.wFlagNum;
				// 前面被标记过，则跟随前一个标记
				else
					m_stEdgeDetect.pwFlag[ i ] = m_stEdgeDetect.pwFlag[ i - 1 ];
			}
		}

		// 除第一行之外的标记，此时会出现等价的关系
		for ( long j = 1; j < m_stBitMap.bmHeight; j++ )
		{
			// 第j行前面的像素数
			long lPixs = j*m_stBitMap.bmWidth;

			// 先对该行的第一个点做处理，只需要检视上，右上两个点，这两个可能存在等价对
			if ( m_stEdgeDetect.pbyFilte[ lPixs ] == byMaking )
			{
				// <上>位置被标记过
				if ( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ] != 0 )
				{
					// 跟随<上>标记
					m_stEdgeDetect.pwFlag[ lPixs ] = m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ];
					// 如果<上> != <右上>，且<右上>已标记，存在等价对
					if ( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ] != m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] &&
						m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] != 0 )
						// <上><右上>等价标记
						AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth ], m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] );
				}
				// <上>没有标记，此时一定不会存在等价关系
				else
				{
					if ( m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] != 0 )
						m_stEdgeDetect.pwFlag[ lPixs ] = m_stEdgeDetect.pwFlag[ lPixs - m_stBitMap.bmWidth + 1 ] != 0;   //跟随<右上>标记
					// <上>、<右上>都没有标记，则开始新的标记
					else
						m_stEdgeDetect.pwFlag[ lPixs ] = ++m_stEdgeDetect.wFlagNum;
				}
			}

			// 对每行的中间点做标记处理，此时存在<左>、<左上>、<上>、<右上> 4种情况
			for ( long i = 1; i < m_stBitMap.bmWidth - 1; i++ )
			{
				long lSself = lPixs + i;	// 当前点
				long lLeft = lSself - 1;	// <左>
				long lLfTop = lLeft - m_stBitMap.bmWidth;	// <左上>
				long lTop = lLfTop + 1;		// <上>
				long lRtTop = lTop + 1;		// <右上>
				// 需要标记
				if ( m_stEdgeDetect.pbyFilte[ lSself ] == byMaking )
				{
					// <左>被标记过
					if ( m_stEdgeDetect.pwFlag[ lLeft ] != 0 )
					{
						m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lLeft ];	//跟随<左>

						if ( m_stEdgeDetect.pwFlag[ lLfTop ] != 0 && m_stEdgeDetect.pwFlag[ lLeft ] != m_stEdgeDetect.pwFlag[ lLfTop ] )
							// 标记<左>、<左上>等价
							AddEqualMark( m_stEdgeDetect.pwFlag[ lLeft ], m_stEdgeDetect.pwFlag[ lLfTop ] );

						if ( m_stEdgeDetect.pwFlag[ lTop ] != 0 && m_stEdgeDetect.pwFlag[ lLeft ] != m_stEdgeDetect.pwFlag[ lTop ] )
							//标记<左>、<上>等价
							AddEqualMark( m_stEdgeDetect.pwFlag[ lLeft], m_stEdgeDetect.pwFlag[ lTop ] );

						if ( m_stEdgeDetect.pwFlag[ lRtTop ] != 0 && m_stEdgeDetect.pwFlag[ lLeft ] != m_stEdgeDetect.pwFlag[ lRtTop ] )
							//标记<左>、<右上>等价
							AddEqualMark( m_stEdgeDetect.pwFlag[ lLeft ], m_stEdgeDetect.pwFlag[ lRtTop ] );
					}
					// <左>未被标记过
					else
					{
						// <左上>被标记过
						if ( m_stEdgeDetect.pwFlag[ lLfTop ] != 0 )
						{
							m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lLfTop ];	// 跟随<左上>

							if ( m_stEdgeDetect.pwFlag[ lTop ] != 0 && m_stEdgeDetect.pwFlag[ lLfTop ] != m_stEdgeDetect.pwFlag[ lTop ] )
								//标记<左上>、<上>等价
								AddEqualMark( m_stEdgeDetect.pwFlag[ lLfTop ], m_stEdgeDetect.pwFlag[ lTop ] );

							if ( m_stEdgeDetect.pwFlag[ lRtTop ] != 0 && m_stEdgeDetect.pwFlag[ lLfTop ] != m_stEdgeDetect.pwFlag[ lRtTop ] )
								//标记<左上>、<右上>等价
								AddEqualMark( m_stEdgeDetect.pwFlag[ lLfTop ], m_stEdgeDetect.pwFlag[ lRtTop ] );
						}
						// <左>、<左上>未标记过
						else
						{
							if ( m_stEdgeDetect.pwFlag[ lTop ] !=0 )
							{
								m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lTop ];	//跟随<上>标记

								if ( m_stEdgeDetect.pwFlag[ lRtTop ] != 0 && m_stEdgeDetect.pwFlag[ lTop ] != m_stEdgeDetect.pwFlag[ lRtTop ] )
									// 标记<上>和<右上>等价
									AddEqualMark( m_stEdgeDetect.pwFlag[ lTop ], m_stEdgeDetect.pwFlag[ lRtTop ] );
							}
							// <左>、<左上>、<上>未标记过，此时不存在等价关系
							else
							{
								if ( m_stEdgeDetect.pwFlag[ lRtTop ] !=0 )
									m_stEdgeDetect.pwFlag[ lSself ] = m_stEdgeDetect.pwFlag[ lRtTop ];	//跟随<右上>标记
								// <左>、<左上>、<上>、<右上>未标记过，则开始新的标记值
								else
									m_stEdgeDetect.pwFlag[ lSself ] = ++m_stEdgeDetect.wFlagNum;
							}
							// <左>、<左上>、<上>未标记过结束
						}
						// <左>、<左上>未标记过结束
					}
					// <左>未被标记过结束
				}
				// else 不需要标记
			}
			// 中间点处理的结束

			// 最后对该行的最后一个点做处理，此时存在<左>、<左上>、<上> 3种情况
			// 需要标记
			if ( m_stEdgeDetect.pbyFilte[ lPixs + m_stBitMap.bmWidth - 1 ] == byMaking )
			{
				// <左>被标记过
				if ( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ] != 0 )
				{
					m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ];	// 跟随<左>

					if ( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ] != m_stEdgeDetect.pwFlag[ lPixs - 2 ] &&
						m_stEdgeDetect.pwFlag[ lPixs - 2 ] != 0 )
						// 标记<左>、<左上>等价
						AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ], m_stEdgeDetect.pwFlag[ lPixs - 2 ] );

					if ( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ] != m_stEdgeDetect.pwFlag[ lPixs - 1 ] &&
						m_stEdgeDetect.pwFlag[ lPixs - 1 ] != 0 )
						// 标记<左>、<上>等价
						AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 2 ], m_stEdgeDetect.pwFlag[ lPixs - 1 ] );
				}
				// <左>未被标记过
				else
				{
					if ( m_stEdgeDetect.pwFlag[ lPixs - 2 ] != 0 )
					{
						m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = m_stEdgeDetect.pwFlag[ lPixs - 2 ];	//跟随<左上>

						if ( m_stEdgeDetect.pwFlag[ lPixs - 2 ] != m_stEdgeDetect.pwFlag[ lPixs - 1 ] && m_stEdgeDetect.pwFlag[ lPixs - 1 ] != 0 )
							// 标记<左上>、<上>等价
							AddEqualMark( m_stEdgeDetect.pwFlag[ lPixs  - 2 ], m_stEdgeDetect.pwFlag[ lPixs - 1 ] );
					}
					// <左>、<左上>未标记过，此时不存在等价对
					else
					{
						if ( m_stEdgeDetect.pwFlag[ lPixs - 1 ] != 0 )
							m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = m_stEdgeDetect.pwFlag[ lPixs - 1 ];	//跟随<上>标记
						//<左>、<左上>、<上>未标记过,则开始新的标记值
						else
							m_stEdgeDetect.pwFlag[ lPixs + m_stBitMap.bmWidth - 1 ] = ++m_stEdgeDetect.wFlagNum;
					}
				}
			}
			// 对每行的最后一个点 处理 结束
		}
		// "除第一行之外的标记"的结束
	}
	if ( m_stEdgeDetect.wFlagNum == NULL )
		return FALSE;
	//==========================第一次标记结束==========================
	// 图像像素数
	long lPixNum = m_stBitMap.bmHeight*m_stBitMap.bmWidth;
	if ( m_stEdgeDetect.wEqNum )
	{
		// 整理标记
		m_stEdgeDetect.wFlagNum = AdjustEMak();
		if ( m_stEdgeDetect.wFlagNum == NULL )
			return FALSE;

		// 第二次标记
		for( long i = 0; i < lPixNum; ++i )
		{
			WORD& wFlag = m_stEdgeDetect.pwFlag[i];
			if ( wFlag )
				wFlag = m_stEdgeDetect.punEq[ wFlag - 1 ].wMarkB;
		}
	}
	// 新标记统计面积,求出最大面积的区域，并将其他区域消掉
	{
		// 逐一统计标记面积，存放在m_stEdgeDetect.punEq中
		ZeroMemory( m_stEdgeDetect.punEq, 65535*sizeof(UN_EQUAL_MARK) );
		LPDWORD lpdwAera = (LPDWORD)m_stEdgeDetect.punEq;
		for( long j = 0; j < lPixNum; ++j )
		{
			WORD& wFlag = m_stEdgeDetect.pwFlag[j];
			if ( wFlag )
				lpdwAera[ wFlag - 1 ] += 1;
		}
		// 需找最大面积的标记
		DWORD dwAeraMax = 0;		// 最大区域面积
		WORD wMaxIndex[2] = { 0 };	// 最大面积的标记
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
		// 保留最大面积区域
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

// 添加等价对
BOOL CImgProcess::AddEqualMark( WORD a, WORD b )
{
	// 数目超限
	if( m_stEdgeDetect.wEqNum == 65535 )
	{
		AfxMessageBox( _T("等价对数目超过支持范围。\n") );
		return FALSE;
	}

	// 是否与上一个等价对相同
	if ( m_stEdgeDetect.wEqNum &&
		( m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkA == a &&
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkB == b ) ||
		( m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkA ==  b &&
		m_stEdgeDetect.punEq[ m_stEdgeDetect.wEqNum - 1 ].wMarkB == a ) )
		return TRUE;

	// 加入新的等价对，小的为A，大的为B
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

// 调整等价对
WORD CImgProcess::AdjustEMak()
{
	// 等价对排序
	std::sort( m_stEdgeDetect.punEq, m_stEdgeDetect.punEq + m_stEdgeDetect.wEqNum, &CImgProcess::MarkCompare );
	// 去除中间标记
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
	// 等价对再次排序
	std::sort( m_stEdgeDetect.punEq, m_stEdgeDetect.punEq + m_stEdgeDetect.wEqNum, &CImgProcess::MarkCompare );
	// 去除重复等价对
	WORD wIndex = 0;
	for( WORD i = 1; i < m_stEdgeDetect.wEqNum; i++ )
	{
		if( m_stEdgeDetect.punEq[i].dwMark != m_stEdgeDetect.punEq[wIndex].dwMark )
			m_stEdgeDetect.punEq[++wIndex].dwMark = m_stEdgeDetect.punEq[i].dwMark;
	}
	m_stEdgeDetect.wEqNum = wIndex + 1;
	// 建立第二次标记
	wIndex = 0;
	for( WORD wA = 0, wNum = m_stEdgeDetect.wEqNum, i = 0; i < wNum; i++ )
	{
		if( wA < m_stEdgeDetect.punEq[i].wMarkA )
		{
			wIndex ++;
			wA = m_stEdgeDetect.punEq[i].wMarkA;
			// 加入标记
			m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkA = wIndex;
			m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkB = wA;

			m_stEdgeDetect.wEqNum ++;
		}
		if( wA == m_stEdgeDetect.punEq[i].wMarkA )
			m_stEdgeDetect.punEq[i].wMarkA = wIndex;
	}
	// 添加不是等价对的标记
	for ( WORD wNum = m_stEdgeDetect.wEqNum, i = 1; i <= m_stEdgeDetect.wFlagNum; i++ )
	{
		for( WORD j = 0; j < wNum; j++ )
		{
			// 是等价标记，跳过
			if ( m_stEdgeDetect.punEq[j].wMarkB == i )
				break;
			// 没找到，不是等价标记
			if( j == wNum - 1 )
			{
				// 加入新标记
				wIndex ++;
				m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkA = wIndex;
				m_stEdgeDetect.punEq[m_stEdgeDetect.wEqNum].wMarkB = i;

				m_stEdgeDetect.wEqNum ++;
				break;
			}
		}
	}
	// 交换A、B，之后A是老标记，B是新标记
	for ( WORD wT = 0, i = 0; i < m_stEdgeDetect.wEqNum; i++ )
	{
		wT = m_stEdgeDetect.punEq[i].wMarkA;
		m_stEdgeDetect.punEq[i].wMarkA = m_stEdgeDetect.punEq[i].wMarkB;
		m_stEdgeDetect.punEq[i].wMarkB = wT;
	}
	// 根据老标记排序，使老标记从1开始，逐一下去，以方便用数组下标访问
	std::sort( m_stEdgeDetect.punEq, m_stEdgeDetect.punEq + m_stEdgeDetect.wEqNum, &CImgProcess::MarkCompare );

	// 返回新标记数
	return wIndex;
}

// 标记比较
bool CImgProcess::MarkCompare( UN_EQUAL_MARK& a, UN_EQUAL_MARK& b )
{
	return ( a.dwMark < b.dwMark );
}

// 直线拟合
BOOL CImgProcess::LineFitting( ST_EDGE_PARA& stEdgePara )
{
	// 寻找4个角点
	{
		int iCorner[4] = {0}, iDistance = 0;
		int iDisTemp[4] = {0};
		// 初始化为最远的距离
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
					// 0 书框 左上点，离图像 左上点 最近的点
					iDistance = iDisTemp[0] + iDisTemp[1];
					if( iCorner[0] > iDistance )
					{
						iCorner[0] = iDistance;
						stEdgePara.point[0].x = j;
						stEdgePara.point[0].y = i;
					}
					// 1 右上点
					iDistance = iDisTemp[0] + iDisTemp[3];
					if( iCorner[1] > iDistance )
					{
						iCorner[1] = iDistance;
						stEdgePara.point[1].x = j;
						stEdgePara.point[1].y = i;
					}
					// 2 左下点
					iDistance = iDisTemp[2] + iDisTemp[1];
					if( iCorner[2] > iDistance )
					{
						iCorner[2] = iDistance;
						stEdgePara.point[2].x = j;
						stEdgePara.point[2].y = i;
					}
					// 3 右下点
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
	// 拟合四个边缘
	{
		// 边缘点集
		PST_POINT pstPoints = (PST_POINT)m_stEdgeDetect.pwFlag;
		// 边缘点数组游标
		int iId = 0;
		// 拟合中间变量
		int iDisTemp[4] = { ( stEdgePara.point[0].y + stEdgePara.point[1].y )/2,
			( stEdgePara.point[2].y + stEdgePara.point[3].y )/2,
			( stEdgePara.point[0].x + stEdgePara.point[2].x )/2,
			( stEdgePara.point[1].x + stEdgePara.point[1].x )/2 };
		double kx = 0, ky = 0, kx2 = 0, ky2 = 0, kxy = 0;
		// 遍历变量
		int iPixs = 0;
		// ===================上边缘拟合 y = a0x + b0;
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
		// ===================下边缘拟合 y =a1x + b1;
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
		// ===================左边缘拟合 x = a2y + b2;
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
		// ===================右边缘拟合 x = a3y + b3;
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
	// 重新计算四个角点
	{
		// 左上点
		stEdgePara.point[0].y = int( ( stEdgePara.line[0].a*stEdgePara.line[2].b + stEdgePara.line[0].b )/( 1 - stEdgePara.line[0].a*stEdgePara.line[2].a ) );
		stEdgePara.point[0].x = int( stEdgePara.line[2].a*stEdgePara.point[0].y + stEdgePara.line[2].b );
		// 右上点
		stEdgePara.point[1].y = int( ( stEdgePara.line[0].a*stEdgePara.line[3].b + stEdgePara.line[0].b )/( 1 - stEdgePara.line[0].a*stEdgePara.line[3].a ) );
		stEdgePara.point[1].x = int( stEdgePara.line[3].a*stEdgePara.point[1].y + stEdgePara.line[3].b );
		// 左下点
		stEdgePara.point[2].y = int( ( stEdgePara.line[1].a*stEdgePara.line[2].b + stEdgePara.line[1].b )/( 1 - stEdgePara.line[1].a*stEdgePara.line[2].a ) );
		stEdgePara.point[2].x = int( stEdgePara.line[2].a*stEdgePara.point[2].y + stEdgePara.line[2].b );
		// 右下点
		stEdgePara.point[3].y = int( ( stEdgePara.line[1].a*stEdgePara.line[3].b + stEdgePara.line[1].b )/( 1 - stEdgePara.line[1].a*stEdgePara.line[3].a ) );
		stEdgePara.point[3].x = int( stEdgePara.line[3].a*stEdgePara.point[3].y + stEdgePara.line[3].b );
	}
	return TRUE;
}

// 提取角点检测区域
BOOL CImgProcess::ValidArea( PST_PROC_DATA pstProcData )
{
	ST_EDGE_PARA& stEdgePara = *(pstProcData->pstEdgePara);
	// 校验书本边框四个角点的有效性
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

	// 利用拟合结果，重塑连通域
	for( int x = 0, y = 0; x < m_stBitMap.bmWidth; x++ )
	{
		// 上边缘
		if( x >= stEdgePara.point[0].x && x <= stEdgePara.point[1].x )
		{
			// 计算y
			y = (int)( stEdgePara.line[0].a*x + stEdgePara.line[0].b );
			// 将点填充灰度128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
		// 下边缘
		if( x >= stEdgePara.point[2].x && x <= stEdgePara.point[3].x )
		{
			// 计算y
			y = (int)( stEdgePara.line[1].a*x + stEdgePara.line[1].b );
			// 将点填充灰度128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
	}
	for( int x = 0, y = 0; y < m_stBitMap.bmHeight; y++ )
	{
		// 左边缘
		if ( y >= stEdgePara.point[0].y && y <= stEdgePara.point[2].y )
		{
			// 计算x
			x = (int)( stEdgePara.line[2].a*y + stEdgePara.line[2].b );
			// 将点填充灰度128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
		// 右边缘
		if ( y >= stEdgePara.point[1].y && y <= stEdgePara.point[3].y )
		{
			// 计算x
			x = (int)( stEdgePara.line[3].a*y + stEdgePara.line[3].b );
			// 将点填充灰度128
			pstProcData->lpMask[ y*m_stBitMap.bmWidth + x ] = 128;
		}
	}
	// 填充连通区域作为源图像的蒙版
	int iPixs = 0, x1 = 0, x2 = 0;
	bool bLeft = true, bRight = true;
	for( int y = 0; y < m_stBitMap.bmHeight; y++ )
	{
		iPixs = y*m_stBitMap.bmWidth;
		x1 = 0, x2 = m_stBitMap.bmWidth - 1;
		bLeft = true, bRight = true;
		while( true )
		{
			// 左端
			if ( bLeft )
			{
				if ( pstProcData->lpMask[ iPixs + x1 ] == 128 )
					bLeft = false;
				else
					x1 ++;
			}
			if ( bRight )
			{
				// 右端
				if( pstProcData->lpMask[ iPixs + x2 ] == 128 )
					bRight = false;
				else
					x2 --;
			}
			// 处理
			if( !( bLeft || bRight ) )
			{
				// 将x1 x2之间填充灰度128
				memset( pstProcData->lpMask + iPixs + x1, 128, x2 - x1 + 1 );
				break;
			}
			else if( x1 + 1 >= x2 )
				break;
		}
	}
	return TRUE;
}

// Harris角点检测
BOOL CImgProcess::Harris( VEC_CORNER* pvecCorner )
{
	// 拷贝图像:Cim
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

	// 利用水平、垂直算子对每个像素滤波求得Ix，Iy :Ix2 Iy2
	{
		// 定义水平方向差分算子并求Ix(低通滤波）
		double dHori[9]={ -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		Convolution( m_stCornerDtec.Cim, (int)m_stBitMap.bmWidth, (int)m_stBitMap.bmHeight,
			dHori, 3, 3,
			m_stCornerDtec.Ix2 );
		// 定义垂直方向差分算子并求Iy（低通滤波）
		double dVert[9]={ -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		Convolution( m_stCornerDtec.Cim, (int)m_stBitMap.bmWidth, (int)m_stBitMap.bmHeight,
			dVert, 3, 3,
			m_stCornerDtec.Iy2 );
	}

	// 计算 Ix2 Ixy Iy2
	{
		double dIx = 0, dIy = 0;
		long lPixs = 0;
		for( long i = 0; i < m_stBitMap.bmHeight; i++ )
		{
			lPixs = i*m_stBitMap.bmWidth;
			for( long j = 0; j < m_stBitMap.bmWidth; j++ )
			{
				// 蒙版处理
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

	// 对Ix2 Ixy Iy2 分别进行高斯滤波
	{
		// 计算高斯模板 5*5
		double Gus[25];
		for( int i = 0; i < 5; i++ )
		{
			for( int j = 0; j < 5; j++ )
			{
				Gus[ i*5 + j ] = exp( -( (i-2)*(i-2)+(j-2)*(j-2) )/1.28 );
			}
		}
		// 归一化：使模板参数之和为1（其实此步可以省略）
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

	// 计算Cim
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
				// 注意：要在分母中加入一个极小量以防止除数为零溢出
				m_stCornerDtec.Cim[ lPixs + j ] = ( dIx2*dIy2 - dIxy*dIxy )/( dIx2 + dIy2 + 0.000001 );
			}
		}
	}

	//局部极大值抑制 :Ixy
	{
		// 区域大小 2*size+1 = 7*7
		int size = 3;

		// Ix2 作为局部极值的数组
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

	// 确定角点
	{
		// 清空角点向量
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

// 卷积函数
BOOL CImgProcess::Convolution( double* src, int lmW, int lmH, double *tp, int tpW, int tpH, double* des )
{
	// 清输出内存
	memset( des, 0, lmW*lmH*sizeof(double) );

#define src(x,y) src[lmW*(y)+(x)]
#define tp(x,y) tp[tpW*(y)+(x)]
#define des(x,y) des[lmW*(y)+(x)]

	// 开始卷积
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
 * 函数名称：
 *   InteEqualize()
 * 参数:
 *    BITMAP& stBitMap	- 图像数据结构
 * 返回值:
 *   BOOL               - 成功返回TRUE，否则返回FALSE。
 * 说明:
 *   该函数用来对图像进行直方图均衡。
 ************************************************************************/
BOOL CImgProcess::InteEqualize( BITMAP& stBitMap )
{
	// 32位图像
	if( stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// 像素数
	long lPixNum = stBitMap.bmHeight*stBitMap.bmWidth;

	// 灰度统计表
	long lCount[256] = { 0 };
	// 计算各个灰度值的计数
	LPRGBQUAD lpSrc = NULL;
	for ( long i = 0; i < lPixNum; i ++ )
	{
		lpSrc = ((LPRGBQUAD)stBitMap.bmBits) + i;
		lCount[ lpSrc->rgbReserved ]++;
	}

	// 灰度映射表
	BYTE bMap[256] = { 0 };
	// 计算灰度映射表
	long lTemp = 0;
	for ( long i = 0; i < 256; i++ )
	{
		lTemp += lCount[i];
		// 计算对应的新灰度值
		bMap[i] = (BYTE)( lTemp * 255/lPixNum );
	}

	for( long i = 0; i < lPixNum; i++ )
	{
		// 指向DIB第i行，第j个象素的指针
		lpSrc = ((LPRGBQUAD)stBitMap.bmBits) + i;
		// 计算新的灰度值
		lpSrc->rgbReserved = bMap[ lpSrc->rgbReserved ];
	}
	return TRUE;
}

/*************************************************************************
 * 函数名称：
 *   MedianFilter()
 * 参数:
 *   BITMAP& stBitMap			- 图像数据结构
 *   ST_TEMPLATE& stTemPlate	- 模板结构
 * 返回值:
 *   BOOL						- 成功返回TRUE，否则返回FALSE。
 * 说明:
 *   该函数对 32位 DIB图像进行中值滤波。
 ************************************************************************/
BOOL CImgProcess::MedianFilter( BITMAP& stBitMap )
{
	// 不是32位返回
	if( stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// 清滤波内存
	ZeroMemory( m_stEdgeDetect.pbyFilte, RESO_Y*RESO_X );

	// 开始中值滤波
	LPBYTE lpSrc[9] = { NULL };
	LPBYTE lpDst = NULL;
	long lPixs = 0;
	for( long i = 1; i < stBitMap.bmHeight - 1; i++ )	// 行(除去边缘几行)
	{
		lPixs = i*stBitMap.bmWidth;
		for( long j = 1; j < stBitMap.bmWidth - 1; j++ )	// 列(除去首尾几列)
		{
			// 指向滤波图像中 象素( lc, lr) 的指针
			lpDst = m_stEdgeDetect.pbyFilte + lPixs + j;

			// 获取 边缘图像象素( j, i ) 滤波器数组
			lpSrc[4] = m_stEdgeDetect.pbyEgDtc + lPixs + j; lpSrc[3] = lpSrc[4] - 1; lpSrc[5] = lpSrc[4] + 1;
			lpSrc[0] = lpSrc[3] - stBitMap.bmWidth; lpSrc[1] = lpSrc[0] + 1; lpSrc[2] = lpSrc[1] + 1;
			lpSrc[6] = lpSrc[3] + stBitMap.bmWidth; lpSrc[7] = lpSrc[6] + 1; lpSrc[8] = lpSrc[7] + 1;

			// 获取中值
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

// 腐蚀
BOOL CImgProcess::Corrosion( BITMAP& stBitMap )
{
	// 不是32位返回
	if( stBitMap.bmBitsPixel != 32 )
		return FALSE;

	// 清滤波内存
	ZeroMemory( m_stEdgeDetect.pbyFilte, RESO_Y*RESO_X );

	// 开始腐蚀
	LPBYTE lpSrc[4] = { NULL };
	LPBYTE lpDst = NULL;
	long lPixs = 0;
	for( long i = 1; i < stBitMap.bmHeight - 1; i++ )	// 行(除去边缘几行)
	{
		lPixs = i*stBitMap.bmWidth;
		for( long j = 1; j < stBitMap.bmWidth - 1; j++ )	// 列(除去首尾几列)
		{
			// 指向目的图像中 象素( lc, lr) 的指针
			lpDst = m_stEdgeDetect.pbyFilte + lPixs + j;

			// 获取 边缘图像象素( j, i ) 滤波器数组
			lpSrc[1] = m_stEdgeDetect.pbyEgDtc + lPixs + j - 1;
			lpSrc[2] = lpSrc[1] + 2;
			lpSrc[3] = lpSrc[2] + stBitMap.bmWidth - 1;
			lpSrc[0] = lpSrc[2] - stBitMap.bmWidth - 1;

			// 腐蚀检测（四邻域全部白才白）
			if ( lpSrc[0] && lpSrc[1] && lpSrc[2] && lpSrc[3] )
				*lpDst = 255;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// 获取数据线程
UINT CImgProcess::ThdGetDataProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// 稍等
		Sleep( SLEEP_TIME );

		// 等待启动事件
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETDATA ].hStart, INFINITE );

		// 检查退出事件
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETDATA ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// 检查数据事件，如果没有则继续
		if ( ::WaitForSingleObject( pImgProc->m_hHasData, MAX_WAIT_DATA_TIME ) != WAIT_OBJECT_0 )
			continue;
		ResetEvent( pImgProc->m_hHasData );

		// 没有缓存则继续
		if ( pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.empty() )
			continue;

		// 从队列获取缓存
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// 取最后帧数据
		pstProcData = pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.back();
		// 该帧从帧队列中移除
		pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );

		// 获取数据
		pImgProc->GetData( pstProcData );

		// 该帧压入 边缘 队列中
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );
	}

	// 表明已退出
	TRACE( "ThdGetDataProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_GETDATA ].hHasExit );

	// 结束
	return 0;
}

// 边缘检测线程
UINT CImgProcess::ThdGetEdgeProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// 稍等
		Sleep( SLEEP_TIME );

		// 等待启动事件
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETEDGE ].hStart, INFINITE );

		// 检查退出事件
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETEDGE ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// 没有缓存则继续
		if ( pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.empty() )
			continue;

		// 从队列获取缓存
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// 取最后帧数据
		pstProcData = pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.back();
		// 该帧从帧队列中移除
		pImgProc->m_staThdPara[ THREAD_GETEDGE ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETEDGE ].crtDqOp );

		// 边缘检测
		{
			// 设置边缘检测内存结构
			{
				pImgProc->m_stEdgeDetect.lpRGB = pstProcData->lpRGB;
				pImgProc->m_stEdgeDetect.pbyEgDtc = pstProcData->pbyBufA;
				pImgProc->m_stEdgeDetect.pbyFilte = pstProcData->lpMask;
				pImgProc->m_stEdgeDetect.pwFlag = ( LPWORD )pstProcData->pbyBufB;
				pImgProc->m_stEdgeDetect.punEq = ( PUN_EQUAL_MARK )pstProcData->pbyBufC;
				pImgProc->m_stEdgeDetect.wFlagNum = 0;
				pImgProc->m_stEdgeDetect.wEqNum = 0;
			}
			// 霍夫边缘检测
			pImgProc->HoughEdge();
			// 膨胀
			pImgProc->Expand();
			// 最大连通域检测
			pImgProc->MaxConDo();
			// 线性拟合
			pImgProc->LineFitting( *(pstProcData->pstEdgePara) );
		}

		// 该帧压入 角点 队列中
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );
	}

	// 表明已退出
	TRACE( "ThdGetEdgeProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_GETEDGE ].hHasExit );

	// 结束
	return 0;
}

// 角点检测线程
UINT CImgProcess::ThdGetCornerProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// 稍等
		Sleep( SLEEP_TIME );

		// 等待启动事件
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETCORNER ].hStart, INFINITE );

		// 检查退出事件
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_GETCORNER ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// 没有缓存则继续
		if ( pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.empty() )
			continue;

		// 从队列获取缓存
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// 取最后帧数据
		pstProcData = pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.back();
		// 该帧从帧队列中移除
		pImgProc->m_staThdPara[ THREAD_GETCORNER ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETCORNER ].crtDqOp );

		// 角点检测
		{
			// 角点检测内存结构
			{
				pImgProc->m_stCornerDtec.lpRGB = pstProcData->lpRGB;
				pImgProc->m_stCornerDtec.pbyFilte = pstProcData->lpMask;
				pImgProc->m_stCornerDtec.Ix2 = (double*)pstProcData->pbyBufA;
				pImgProc->m_stCornerDtec.Iy2 = (double*)pstProcData->pbyBufB;
				pImgProc->m_stCornerDtec.Ixy = (double*)pstProcData->pbyBufC;
				pImgProc->m_stCornerDtec.Cim = (double*)pstProcData->pbyBufD;
			}
			// 获取 角点检测区域
			pImgProc->ValidArea( pstProcData );

			if ( pstProcData->pstEdgePara->bValid )
				// 角点检测
				pImgProc->Harris( pstProcData->pVecCorner );
		}

		// 该帧压入 视图 队列中
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );
	}

	// 表明已退出
	TRACE( "ThdGetCornerProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_GETCORNER ].hHasExit );

	// 结束
	return 0;
}

// 视图更新线程
UINT CImgProcess::ThdUpdataViewProc( LPVOID lpVoid )
{
	CImgProcess* pImgProc = ( CImgProcess* )lpVoid;
	while( true )
	{
		// 稍等
		Sleep( SLEEP_TIME );

		// 等待启动事件
		WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].hStart, INFINITE );

		// 检查退出事件
		if ( ::WaitForSingleObject( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].hCanExit, 0 ) == WAIT_OBJECT_0 )
			break;

		// 没有缓存则继续
		if ( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.empty() )
			continue;

		// 从队列获取缓存
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );
		PST_PROC_DATA pstProcData = NULL;
		// 取最后帧数据
		pstProcData = pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.back();
		// 该帧从帧队列中移除
		pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].dqCache.pop_back();
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].crtDqOp );

		// 更新界面
		// 如果窗口可见，就刷新显示
		if( pImgProc->m_pEffectWnd->IsWindowVisible() )
		{
			pImgProc->m_pEffectWnd->RefreshData( pstProcData );
		}
		// 如果不可见，将标志设为不捕获
		else if( !pImgProc->m_pMainWnd->IsIconic() )
		{
			// 显示关闭
			pImgProc->m_pMainWnd->OnTbDirectShow();
		}

		// 该帧压入 获取数据 队列中
		EnterCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );
		pImgProc->m_staThdPara[ THREAD_GETDATA ].dqCache.push_front( pstProcData );
		LeaveCriticalSection( &pImgProc->m_staThdPara[ THREAD_GETDATA ].crtDqOp );
	}

	// 表明已退出
	TRACE( "ThdUpdataViewProc has exited.\n" );
	SetEvent( pImgProc->m_staThdPara[ THREAD_UPDATAVIEW ].hHasExit );

	// 结束
	return 0;
}
//////////////////////////////////////////////////////////////////////////

// 线程 初始化
BOOL CImgProcess::Init()
{
	// 获取数据事件
	m_hHasData = CreateEvent( NULL, TRUE, FALSE, NULL );

	// 线程参数初始化
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		// 队列访问 临界区
		InitializeCriticalSection( &m_staThdPara[i].crtDqOp );
		// 队列
		m_staThdPara[i].dqCache.clear();

		// 事件：属性 是否手动 初始状态 名称
		m_staThdPara[i].hStart = CreateEvent( NULL, TRUE, FALSE, NULL );
		m_staThdPara[i].hCanExit = CreateEvent( NULL, TRUE, FALSE, NULL );
		m_staThdPara[i].hHasExit = CreateEvent( NULL, TRUE, FALSE, NULL );
	}

	// 分配内存
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		// 原始图像
		m_staProcData[i].lpRGB = new RGBQUAD[ RESO_XY ];
		// 蒙版图像
		m_staProcData[i].lpMask = new BYTE[ RESO_XY ];
		// 中间内存A
		m_staProcData[i].pbyBufA = new BYTE[ RESO_XY*sizeof(double) ];
		// 中间内存B
		m_staProcData[i].pbyBufB = new BYTE[ RESO_XY*sizeof(double) ];
		// 中间内存C
		m_staProcData[i].pbyBufC = new BYTE[ RESO_XY*sizeof(double) ];
		// 中间内存D
		m_staProcData[i].pbyBufD = new BYTE[ RESO_XY*sizeof(double) ];

		// 边缘检测结果
		m_staProcData[i].pstEdgePara = new ST_EDGE_PARA;
		ZeroMemory( m_staProcData[i].pstEdgePara, sizeof(ST_EDGE_PARA) );
		// 角点检测结果
		m_staProcData[i].pVecCorner = new VEC_CORNER;
		m_staProcData[i].pVecCorner->clear();

		// 加入获取数据队列
		m_staThdPara[THREAD_GETDATA].dqCache.push_front( &m_staProcData[i] );
	}

	// 启动线程
	// 获取数据线程：优先级- 0
	m_staThdPara[ THREAD_GETDATA ].pThd = AfxBeginThread( ThdGetDataProc, (LPVOID)this );
	if ( m_staThdPara[ THREAD_GETDATA ].pThd == NULL )
		return FALSE;
	// 边缘检测线程：优先级- 2
	m_staThdPara[ THREAD_GETEDGE ].pThd = AfxBeginThread( ThdGetEdgeProc, (LPVOID)this, THREAD_PRIORITY_HIGHEST );
	if ( m_staThdPara[ THREAD_GETEDGE ].pThd == NULL )
		return FALSE;
	// 角点检测线程：优先级- 2
	m_staThdPara[ THREAD_GETCORNER ].pThd = AfxBeginThread( ThdGetCornerProc, (LPVOID)this, THREAD_PRIORITY_HIGHEST );
	if ( m_staThdPara[ THREAD_GETCORNER ].pThd == NULL )
		return FALSE;
	// 更新视图线程：优先级- 0
	m_staThdPara[ THREAD_UPDATAVIEW ].pThd = AfxBeginThread( ThdUpdataViewProc, (LPVOID)this );
	if ( m_staThdPara[ THREAD_UPDATAVIEW ].pThd == NULL )
		return FALSE;

	return TRUE;
}

// 线程 启动
void CImgProcess::Start()
{
	TRACE( "Start Threads.\n" );
	// 启动
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
		SetEvent( m_staThdPara[i].hStart );
}

// 线程 暂停
void CImgProcess::Pause()
{
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
		ResetEvent( m_staThdPara[i].hStart );

	TRACE( "Pause Threads.\n" );
}

// 线程 退出
void CImgProcess::Exit()
{
	// 通知退出
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
		SetEvent( m_staThdPara[i].hCanExit );

	// 启动线程
	Start();

	// 等待所有线程退出
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		WaitForSingleObject( m_staThdPara[i].hHasExit, INFINITE );
		
		// 清空队列
		m_staThdPara[i].dqCache.clear();
		// 删除 队列访问 临界区
		DeleteCriticalSection( &m_staThdPara[i].crtDqOp );
		// 关闭事件
		CloseHandle( m_staThdPara[i].hStart );
		CloseHandle( m_staThdPara[i].hCanExit );
		CloseHandle( m_staThdPara[i].hHasExit );
	}
	// 关闭新数据事件
	CloseHandle( m_hHasData );

	// 释放内存
	for ( BYTE i = 0; i < THREAD_CNT; i++ )
	{
		// 原始图像
		ReleaseArray( m_staProcData[i].lpRGB );
		// 蒙版图像
		ReleaseArray( m_staProcData[i].lpMask );
		// 中间 A
		ReleaseArray( m_staProcData[i].pbyBufA );
		// 中间 B
		ReleaseArray( m_staProcData[i].pbyBufB );
		// 中间 C
		ReleaseArray( m_staProcData[i].pbyBufC );
		// 中间 D
		ReleaseArray( m_staProcData[i].pbyBufD );

		// 边缘检测 结果
		ReleasePoint( m_staProcData[i].pstEdgePara );
		// 角点检测结果
		ReleasePoint( m_staProcData[i].pVecCorner );
	}

	TRACE( "Exit Threads.\n" );
}
