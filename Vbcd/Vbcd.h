// Vbcd.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CVbcdApp:
// �йش����ʵ�֣������ Vbcd.cpp
//

class CVbcdApp : public CWinApp
{
public:
	CVbcdApp();

// ��д
	public:
		virtual BOOL InitInstance();
		virtual int ExitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CVbcdApp theApp;