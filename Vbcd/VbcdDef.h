
#pragma once

#include <winbase.h>
#include <vector>
#include <deque>

using namespace std;

// 图像最大大小
#define RESO_XY			307200	// 图像最大像素数
#define RESO_X			640		// 图像最大宽度
#define RESO_Y			480		// 图像最大高度

// 释放内存宏
#define ReleasePoint( p ) { if( p ){ delete p; p = NULL; } }
#define ReleaseArray( p ) { if( p ){ delete [] p; p = NULL; } }


// 等价标记对结构
typedef union _UN_EQUAL_MARK
{
	struct _ST_EQUAL_MARK
	{
		WORD b;
		WORD a;	// a 要在高地址上
	}stEqualMark;
	DWORD dwMark;
}UN_EQUAL_MARK, *PUN_EQUAL_MARK;
#define wMarkA stEqualMark.a
#define wMarkB stEqualMark.b

// 边缘检测用内存结构
typedef struct _ST_EDGE_DETECT
{
	LPRGBQUAD lpRGB;		// 原始图像

	LPBYTE pbyEgDtc;		// 边缘检测二值化结果
	LPBYTE pbyFilte;		// 边缘检测 后期处理 结果

	WORD wFlagNum;			// 标记个数
	LPWORD pwFlag;			// 标记结果

	WORD wEqNum;			// 等价对数目
	PUN_EQUAL_MARK punEq;	// 等价对

}ST_EDGE_DETECT, *PST_EDGE_DETECT;


// 角点检测用内存结构
typedef struct _ST_CORNER_DTEC
{
	LPRGBQUAD lpRGB;		// 原始图像
	LPBYTE pbyFilte;		// 蒙版图像

	double* Ix2;			// 差分图像
	double* Ixy;			// 差分图像
	double* Iy2;			// 差分图像

	double* Cim;			// 哈瑞图像
}ST_CORNER_DTEC, *PST_CORNER_DTEC;

// 点坐标
typedef struct _ST_POINT
{
	short x;
	short y;
}ST_POINT, *PST_POINT;

// 边缘参数
typedef struct _ST_EDGE_PARA
{
	// 边缘参数是否有效
	bool bValid;
	// 四个角点
	ST_POINT point[4];
	// 四个边缘线
	struct _LINE
	{
		double a;
		double b;
	}line[4];
}ST_EDGE_PARA, *PST_EDGE_PARA;

// 角点系列结构
typedef vector<ST_POINT> VEC_CORNER;

// 图像处理数据结构(数据包)
typedef struct _ST_PROC_DATA
{
	LPRGBQUAD lpRGB;		// 原始图像数据
	LPBYTE	lpMask;			// 蒙版图像

	LPBYTE	pbyBufA;		// 处理缓存A
	LPBYTE	pbyBufB;		// 处理缓存B
	LPBYTE	pbyBufC;		// 处理缓存C
	LPBYTE	pbyBufD;		// 处理缓存D

	PST_EDGE_PARA pstEdgePara;	// 边缘检测结果
	VEC_CORNER*	pVecCorner;		// 角点检测结果

}ST_PROC_DATA, *PST_PROC_DATA;

// 线程参数
typedef struct _ST_THD_PARA
{
	CWinThread*	pThd;		// 线程指针

	HANDLE hStart;			// 启动事件
	HANDLE hCanExit;		// 允许退出事件
	HANDLE hHasExit;		// 已经退出事件

	CRITICAL_SECTION crtDqOp;		// 缓存队列操作 临界区
	deque< PST_PROC_DATA > dqCache;	// 缓存队列

}ST_THD_PARA, *PST_THD_PARA;
