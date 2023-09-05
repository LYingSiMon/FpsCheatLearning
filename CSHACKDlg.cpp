
// CSHACKDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "CSHACK.h"
#include "CSHACKDlg.h"
#include "afxdialogex.h"

#include "spdlog/spdlog.h"
#include "Common.h"
#include "Cheat.h"
#include "Draw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCSHACKDlg 对话框



CCSHACKDlg::CCSHACKDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CSHACK_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCSHACKDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PLAYER, m_ListCtrl_Player);
	DDX_Control(pDX, IDC_LIST_MATRIX, m_ListCtrl_Matrix);
}

BEGIN_MESSAGE_MAP(CCSHACKDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CCSHACKDlg::OnBnClickedButtonStart)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CCSHACKDlg 消息处理程序

BOOL CCSHACKDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//
	// 初始化玩家列表
	//

	DWORD style = m_ListCtrl_Player.GetExtendedStyle();
	style |= LVS_EX_GRIDLINES;
	m_ListCtrl_Player.SetExtendedStyle(style);
	m_ListCtrl_Player.InsertColumn(0, _T("编号"), LVCFMT_CENTER, 50);
	m_ListCtrl_Player.InsertColumn(1, _T("名字"), LVCFMT_CENTER, 50);
	m_ListCtrl_Player.InsertColumn(2, _T("阵营"), LVCFMT_CENTER, 50);
	m_ListCtrl_Player.InsertColumn(3, _T("血量"), LVCFMT_CENTER, 50);
	m_ListCtrl_Player.InsertColumn(4, _T("X"), LVCFMT_CENTER, 50);
	m_ListCtrl_Player.InsertColumn(5, _T("Y"), LVCFMT_CENTER, 50);
	m_ListCtrl_Player.InsertColumn(6, _T("Z"), LVCFMT_CENTER, 50);
	m_ListCtrl_Player.InsertColumn(7, _T("ScreenXY"), LVCFMT_CENTER, 50);

	//
	// 初始化矩阵列表
	//

	style = m_ListCtrl_Matrix.GetExtendedStyle();
	style |= LVS_EX_GRIDLINES;
	m_ListCtrl_Matrix.SetExtendedStyle(style);
	m_ListCtrl_Matrix.InsertColumn(0, _T("矩阵index"), LVCFMT_CENTER, 50);
	m_ListCtrl_Matrix.InsertColumn(1, _T("值"), LVCFMT_CENTER, 50);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCSHACKDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCSHACKDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCSHACKDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void MonitorPlayersInfo()
{
	
	GetGameMatrix();
	GetPlayerCount();
	for (UINT i = 0; i < g_PlayerCount; ++i)
	{
		FLOAT ScreenXYZ[3] = { 0 };

		GetPlayerXYZ(i);
		GetPlayerCamp(i);
		GetPlayerIdAndName(i);
		GetPlayerHealth(i);
		GameXYZToScreenX1Y1Y2(g_PlayerInfo[i].X, g_PlayerInfo[i].Y, g_PlayerInfo[i].Z, ScreenXYZ, i);
	}
}

void CCSHACKDlg::DisPlayPlayersInfo()
{
	// 清空列表
	m_ListCtrl_Player.DeleteAllItems();
	m_ListCtrl_Matrix.DeleteAllItems();

	// 矩阵信息
	for (UINT i = 0; i < 4; ++i)
	{
		for (UINT j = 0; j < 4; ++j)
		{
			CString StrIndex;
			StrIndex.Format("%d", i * 4 + j);
			m_ListCtrl_Matrix.InsertItem(i * 4 + j, StrIndex);

			CString StrMatrix, StrMatrixIndex;
			StrMatrixIndex.Format("[%d][%d]", i, j);
			StrMatrix.Format("%f", g_Matrix[i][j]);
			m_ListCtrl_Matrix.SetItemText(i * 4 + j, 0, StrMatrixIndex);
			m_ListCtrl_Matrix.SetItemText(i * 4 + j, 1, StrMatrix);
		}
	}

	// 玩家信息
	for (UINT i = 0; i < g_PlayerCount; ++i)
	{
		CString StrX, StrY, StrZ;
		CString StrCamp;
		CString StrId;
		CString StrHealth;
		CString StrIndex;

		StrX.Format("%f", g_PlayerInfo[i].X);
		StrY.Format("%f", g_PlayerInfo[i].Y);
		StrZ.Format("%f", g_PlayerInfo[i].Z);
		if (g_PlayerInfo[i].Camp == 0xb6)
			StrCamp.Format("ct");
		else if (g_PlayerInfo[i].Camp == 0xb7)
			StrCamp.Format("t");
		else
			StrCamp.Format("error");
		StrId.Format("%d", g_PlayerInfo[i].Id);
		StrHealth.Format("%d", g_PlayerInfo[i].Health);

		StrIndex.Format("%d", i);
		m_ListCtrl_Player.InsertItem(i, StrIndex);
		m_ListCtrl_Player.SetItemText(i, 0, StrId);
		m_ListCtrl_Player.SetItemText(i, 1, g_PlayerInfo[i].Name);
		m_ListCtrl_Player.SetItemText(i, 2, StrCamp);
		m_ListCtrl_Player.SetItemText(i, 3, StrHealth);
		m_ListCtrl_Player.SetItemText(i, 4, StrX);
		m_ListCtrl_Player.SetItemText(i, 5, StrY);
		m_ListCtrl_Player.SetItemText(i, 6, StrZ);

		CString StrScreenX1Y1Y2;
		StrScreenX1Y1Y2.Format("%f,%f,%f", g_PlayerInfo[i].ScreenX1, g_PlayerInfo[i].ScreenY1, g_PlayerInfo[i].ScreenY2);
		m_ListCtrl_Player.SetItemText(i, 7, StrScreenX1Y1Y2);
	}
}

void CCSHACKDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	switch (nIDEvent)
	{
	case TimerId_MonitorPlayersInfo:
		MonitorPlayersInfo();
	case TimerId_DisPlayPlayersInfo:
		DisPlayPlayersInfo();
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CCSHACKDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码

	// 显示控制台
	AllocConsole();

	// 获取游戏进程信息
	if (!GetGameWnd() ||
		!GetGamePid() ||
		!GetGameProcHandle() ||
		!GetGameModuleBase())
	{
		return;
	}
	
	// 创建定时器
	SetTimer(TimerId_MonitorPlayersInfo, 10, NULL);
	SetTimer(TimerId_DisPlayPlayersInfo, 100, NULL);

	// 绘制
	StartImguiDraw();
}
