
#pragma once

#include <winbase.h>
#include <vector>
#include <deque>

using namespace std;

// ͼ������С
#define RESO_XY			307200	// ͼ�����������
#define RESO_X			640		// ͼ�������
#define RESO_Y			480		// ͼ�����߶�

// �ͷ��ڴ��
#define ReleasePoint( p ) { if( p ){ delete p; p = NULL; } }
#define ReleaseArray( p ) { if( p ){ delete [] p; p = NULL; } }


// �ȼ۱�ǶԽṹ
typedef union _UN_EQUAL_MARK
{
	struct _ST_EQUAL_MARK
	{
		WORD b;
		WORD a;	// a Ҫ�ڸߵ�ַ��
	}stEqualMark;
	DWORD dwMark;
}UN_EQUAL_MARK, *PUN_EQUAL_MARK;
#define wMarkA stEqualMark.a
#define wMarkB stEqualMark.b

// ��Ե������ڴ�ṹ
typedef struct _ST_EDGE_DETECT
{
	LPRGBQUAD lpRGB;		// ԭʼͼ��

	LPBYTE pbyEgDtc;		// ��Ե����ֵ�����
	LPBYTE pbyFilte;		// ��Ե��� ���ڴ��� ���

	WORD wFlagNum;			// ��Ǹ���
	LPWORD pwFlag;			// ��ǽ��

	WORD wEqNum;			// �ȼ۶���Ŀ
	PUN_EQUAL_MARK punEq;	// �ȼ۶�

}ST_EDGE_DETECT, *PST_EDGE_DETECT;


// �ǵ������ڴ�ṹ
typedef struct _ST_CORNER_DTEC
{
	LPRGBQUAD lpRGB;		// ԭʼͼ��
	LPBYTE pbyFilte;		// �ɰ�ͼ��

	double* Ix2;			// ���ͼ��
	double* Ixy;			// ���ͼ��
	double* Iy2;			// ���ͼ��

	double* Cim;			// ����ͼ��
}ST_CORNER_DTEC, *PST_CORNER_DTEC;

// ������
typedef struct _ST_POINT
{
	short x;
	short y;
}ST_POINT, *PST_POINT;

// ��Ե����
typedef struct _ST_EDGE_PARA
{
	// ��Ե�����Ƿ���Ч
	bool bValid;
	// �ĸ��ǵ�
	ST_POINT point[4];
	// �ĸ���Ե��
	struct _LINE
	{
		double a;
		double b;
	}line[4];
}ST_EDGE_PARA, *PST_EDGE_PARA;

// �ǵ�ϵ�нṹ
typedef vector<ST_POINT> VEC_CORNER;

// ͼ�������ݽṹ(���ݰ�)
typedef struct _ST_PROC_DATA
{
	LPRGBQUAD lpRGB;		// ԭʼͼ������
	LPBYTE	lpMask;			// �ɰ�ͼ��

	LPBYTE	pbyBufA;		// ������A
	LPBYTE	pbyBufB;		// ������B
	LPBYTE	pbyBufC;		// ������C
	LPBYTE	pbyBufD;		// ������D

	PST_EDGE_PARA pstEdgePara;	// ��Ե�����
	VEC_CORNER*	pVecCorner;		// �ǵ�����

}ST_PROC_DATA, *PST_PROC_DATA;

// �̲߳���
typedef struct _ST_THD_PARA
{
	CWinThread*	pThd;		// �߳�ָ��

	HANDLE hStart;			// �����¼�
	HANDLE hCanExit;		// �����˳��¼�
	HANDLE hHasExit;		// �Ѿ��˳��¼�

	CRITICAL_SECTION crtDqOp;		// ������в��� �ٽ���
	deque< PST_PROC_DATA > dqCache;	// �������

}ST_THD_PARA, *PST_THD_PARA;
