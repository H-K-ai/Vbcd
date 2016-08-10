
#pragma once

// �������Ի�����
class CVbcdDlg;
// ������ʾ�Ի�����
class CEffectDlg;

// �߳������������
#define THREAD_CNT			4	// �߳�����
#define THREAD_GETDATA		0	// ��ȡ�����̱߳��
#define THREAD_GETEDGE		1	// �鱾�������򣨱�Ե������̱߳��
#define THREAD_GETCORNER	2	// �ǵ����̱߳��
#define THREAD_UPDATAVIEW	3	// ������ͼ���

// �߳�ЪϢʱ��
#define SLEEP_TIME			1
// ���ȴ����ݵ���ʱ��
#define MAX_WAIT_DATA_TIME	1000

class CImgProcess
{
private:
	// �����ڣ����ڻ�ȡ������״̬ ����������
	CVbcdDlg* m_pMainWnd;
	// ��ʾ���ڣ�������ʾͼ����Ľ��
	CEffectDlg* m_pEffectWnd;

	// ͼ����������������޸ģ�����ͼ������ȷ��ͼ��Ĵ�С��
	BITMAP m_stBitMap;

	// ��Ե����ڴ�ṹ
	ST_EDGE_DETECT m_stEdgeDetect;
	// �������ڴ�ṹ
	ST_CORNER_DTEC m_stCornerDtec;

	// ���ݰ� ͼ��������
	ST_PROC_DATA m_staProcData[THREAD_CNT];
	// �̲߳���
	ST_THD_PARA m_staThdPara[THREAD_CNT];

public:
	// ���������¼�����Ƶ����ص���������λ����ȡ���ݺ����и�λ
	HANDLE m_hHasData;
	// ������ָ�룬������Ƶ��׽�⣬����Ƶ����ص�������ȡ��
	LPVIDEOHDR m_lpVHdr;

	CImgProcess( CVbcdDlg* pMainWnd, CEffectDlg* pEffectWnd );
	virtual ~CImgProcess(void);

private:
	//-----------------��ȡ����-----------------
	BOOL GetData( PST_PROC_DATA pstProcData );

	//--------------�鱾����������-------------
	// �����Ե���
	BOOL HoughEdge();
	// ����
	BOOL Expand();
	// �����ͨ��
	BOOL MaxConDo();
	// ��ӵȼ۶�
	BOOL AddEqualMark( WORD a, WORD b );
	// �����ȼ۶�
	WORD AdjustEMak();
	// ��ǱȽ�
	static bool MarkCompare( UN_EQUAL_MARK& a, UN_EQUAL_MARK& b );
	// ֱ�����
	BOOL LineFitting( ST_EDGE_PARA& stEdgePara );

	//--------------�鱾����ǵ���-------------
	// ��ȡ�ǵ�������
	BOOL ValidArea( PST_PROC_DATA pstProcData );
	// �ǵ���
	BOOL Harris( VEC_CORNER* vecCorner );
	// ���
	BOOL Convolution( double* src, int lmW, int lmH, double *tp, int tpW, int tpH, double* des );

	// ֱ��ͼ����
	BOOL InteEqualize( BITMAP& stBitMap );
	// ��ֵ�˲�
	BOOL MedianFilter( BITMAP& stBitMap );
	// ��ʴ
	BOOL Corrosion( BITMAP& stBitMap );

	//------------------�̺߳���-----------------
	// ��ȡ�����߳�
	static UINT	ThdGetDataProc( LPVOID lpVoid );
	// ��Ե����߳�
	static UINT	ThdGetEdgeProc( LPVOID lpVoid );
	// �ǵ����߳�
	static UINT	ThdGetCornerProc( LPVOID lpVoid );
	// ��ͼ�����߳�
	static UINT	ThdUpdataViewProc( LPVOID lpVoid );

public:
	//------------------�̲߳���------------------
	// ����ͼ����Ϣ�������޸�ͼ����Ϣ��ͼ���С
	inline void SetBitMap( BITMAP& stBitMap ) { m_stBitMap = stBitMap; };
	// �߳� ��ʼ��
	virtual BOOL Init();
	// �߳� ����
	virtual void Start();
	// �߳� ��ͣ
	virtual void Pause();
	// �߳� �˳�
	virtual void Exit();
};
