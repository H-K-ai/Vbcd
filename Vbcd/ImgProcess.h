
#pragma once

// 声明主对话框类
class CVbcdDlg;
// 声明显示对话框类
class CEffectDlg;

// 线程数量、各标号
#define THREAD_CNT			4	// 线程数量
#define THREAD_GETDATA		0	// 获取数据线程标号
#define THREAD_GETEDGE		1	// 书本封面区域（边缘）检测线程标号
#define THREAD_GETCORNER	2	// 角点检测线程标号
#define THREAD_UPDATAVIEW	3	// 更新视图标号

// 线程歇息时间
#define SLEEP_TIME			1
// 最大等待数据到来时间
#define MAX_WAIT_DATA_TIME	1000

class CImgProcess
{
private:
	// 父窗口，用于获取父窗口状态 、报告错误等
	CVbcdDlg* m_pMainWnd;
	// 显示窗口，用于显示图像处理的结果
	CEffectDlg* m_pEffectWnd;

	// 图像参数，有主界面修改，用于图像处理中确定图像的大小等
	BITMAP m_stBitMap;

	// 边缘检测内存结构
	ST_EDGE_DETECT m_stEdgeDetect;
	// 焦点检测内存结构
	ST_CORNER_DTEC m_stCornerDtec;

	// 数据包 图像处理数据
	ST_PROC_DATA m_staProcData[THREAD_CNT];
	// 线程参数
	ST_THD_PARA m_staThdPara[THREAD_CNT];

public:
	// 有新数据事件，视频捕获回调函数中置位，获取数据函数中复位
	HANDLE m_hHasData;
	// 新数据指针，来自视频捕捉库，从视频捕获回调函数中取得
	LPVIDEOHDR m_lpVHdr;

	CImgProcess( CVbcdDlg* pMainWnd, CEffectDlg* pEffectWnd );
	virtual ~CImgProcess(void);

private:
	//-----------------获取数据-----------------
	BOOL GetData( PST_PROC_DATA pstProcData );

	//--------------书本封面区域检测-------------
	// 霍夫边缘检测
	BOOL HoughEdge();
	// 膨胀
	BOOL Expand();
	// 最大连通域
	BOOL MaxConDo();
	// 添加等价对
	BOOL AddEqualMark( WORD a, WORD b );
	// 调整等价对
	WORD AdjustEMak();
	// 标记比较
	static bool MarkCompare( UN_EQUAL_MARK& a, UN_EQUAL_MARK& b );
	// 直线拟合
	BOOL LineFitting( ST_EDGE_PARA& stEdgePara );

	//--------------书本封面角点检测-------------
	// 获取角点检测区域
	BOOL ValidArea( PST_PROC_DATA pstProcData );
	// 角点检测
	BOOL Harris( VEC_CORNER* vecCorner );
	// 卷积
	BOOL Convolution( double* src, int lmW, int lmH, double *tp, int tpW, int tpH, double* des );

	// 直方图均衡
	BOOL InteEqualize( BITMAP& stBitMap );
	// 中值滤波
	BOOL MedianFilter( BITMAP& stBitMap );
	// 腐蚀
	BOOL Corrosion( BITMAP& stBitMap );

	//------------------线程函数-----------------
	// 获取数据线程
	static UINT	ThdGetDataProc( LPVOID lpVoid );
	// 边缘检测线程
	static UINT	ThdGetEdgeProc( LPVOID lpVoid );
	// 角点检测线程
	static UINT	ThdGetCornerProc( LPVOID lpVoid );
	// 视图更新线程
	static UINT	ThdUpdataViewProc( LPVOID lpVoid );

public:
	//------------------线程操作------------------
	// 设置图像信息，用来修改图像信息如图像大小
	inline void SetBitMap( BITMAP& stBitMap ) { m_stBitMap = stBitMap; };
	// 线程 初始化
	virtual BOOL Init();
	// 线程 启动
	virtual void Start();
	// 线程 暂停
	virtual void Pause();
	// 线程 退出
	virtual void Exit();
};
