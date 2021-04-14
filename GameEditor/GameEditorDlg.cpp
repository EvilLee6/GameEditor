
// GameEditorDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "GameEditor.h"
#include "GameEditorDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <Windows.h>
#include <stdlib.h>
#include <TlHelp32.h>
#include <Locale.h>
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

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


// CGameEditorDlg 对话框



CGameEditorDlg::CGameEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GAMEEDITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGameEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, mProcessListBox);
	DDX_Control(pDX, IDC_EDIT1, showEdit);
	DDX_Control(pDX, IDC_LIST2, mAddrListBox);
	DDX_Control(pDX, IDC_EDIT2, mEditData);
	DDX_Control(pDX, IDC_EDIT3, showAddr);
	DDX_Control(pDX, IDC_EDIT4, setData);
}

BEGIN_MESSAGE_MAP(CGameEditorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REFRESH, &CGameEditorDlg::OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_OPEN, &CGameEditorDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_SEARCH, &CGameEditorDlg::OnBnClickedSearch)
	ON_BN_CLICKED(IDC_SEARCH_AGAIN, &CGameEditorDlg::OnBnClickedSearchAgain)
	ON_BN_CLICKED(IDC_SELECT, &CGameEditorDlg::OnBnClickedSelect)
	ON_BN_CLICKED(IDC_MODIFY, &CGameEditorDlg::OnBnClickedModify)
END_MESSAGE_MAP()


// CGameEditorDlg 消息处理程序

BOOL CGameEditorDlg::OnInitDialog()
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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGameEditorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGameEditorDlg::OnPaint()
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
HCURSOR CGameEditorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


const DWORD KONEK = 1024;
const DWORD KPAGE = 4 * KONEK;
const DWORD KONEG = KONEK * KONEK * KONEK;
list<DWORD>m_listAddr;


//刷新按钮
void CGameEditorDlg::OnBnClickedRefresh()
{
	// TODO: 在此添加控件通知处理程序代码
	mProcessListBox.ResetContent();
	PROCESSENTRY32 pc;
	pc.dwSize = sizeof(pc);
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	BOOL bMore = Process32First(hProcessSnap, &pc);
	while (bMore)
	{
		CString str;
		str.Format(TEXT("%6d:%s"), pc.th32ParentProcessID, pc.szExeFile);
		int index = mProcessListBox.AddString(str);
		mProcessListBox.SetItemData(index, pc.th32ProcessID);
		bMore = Process32Next(hProcessSnap, &pc);
	}
	CloseHandle(hProcessSnap);
}


//打开按钮
void CGameEditorDlg::OnBnClickedOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	showEdit.Clear();

	DWORD dwId = 0;

	//获取进程号
	int selectIndex = mProcessListBox.GetCurSel();
	dwId = (DWORD)mProcessListBox.GetItemData(selectIndex);

	//创建快照
	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnapshot == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(TEXT("Error while create snapshot"));
		return;
	}

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	//检查第一进程
	if (FALSE == Process32First(hProcessSnapshot, &pe))
	{
		AfxMessageBox(TEXT("Error while create procfirst!"));
		return;
	}
	//循环遍历
	while (Process32Next(hProcessSnapshot, &pe))
	{
		if (pe.th32ProcessID == dwId)
		{
			CString str;
			str.Format(TEXT("%6d:%s"), pe.th32ParentProcessID, pe.szExeFile);
			//显示到Edit
			showEdit.SetWindowTextW(str);
			break;
		}
	}
	CloseHandle(hProcessSnapshot);
}


//查找按钮
void CGameEditorDlg::OnBnClickedSearch()
{
	// TODO: 在此添加控件通知处理程序代码

	mAddrListBox.ResetContent();

	CString str;
	DWORD dwValue = 0;
	DWORD dwPID = 0;
	DWORD BaseAddr = 64 * KONEK;
	DWORD* Buffer;

	mEditData.GetWindowText(str);
	//转进制
	dwValue = _tcstoul(str, NULL, 10);
	
	//鼠标选中 对应一个索引值
	int selectIndex = mProcessListBox.GetCurSel();
	//根据索引找到ID
	dwPID = (DWORD)mProcessListBox.GetItemData(selectIndex);
	//打开进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (hProcess == NULL)
	{
		AfxMessageBox(TEXT("查找时！打开进程失败！"));
	}

	for (; BaseAddr < 2 * KONEG - 64 * KONEK; BaseAddr += KPAGE)
	{
		//读一页数据
		BYTE PageBytes[KPAGE] = { 0 };

		if (FALSE == ReadProcessMemory(hProcess, (LPVOID)BaseAddr, (LPVOID)PageBytes, KPAGE, NULL))
		{
			continue;
		}
		for (int i = 0; i < 4 * 1024 - 3; i++)
		{
			Buffer = (DWORD*)&PageBytes[i];
			if (Buffer[0] == dwValue)
			{
				m_listAddr.push_back((DWORD)BaseAddr + i);
				CString s;
				s.Format(TEXT("%#x"), (DWORD)BaseAddr + i);
				mAddrListBox.AddString(s);
			}
		}
	}
	CloseHandle(hProcess);
}


//再次查找按钮
void CGameEditorDlg::OnBnClickedSearchAgain()
{
	// TODO: 在此添加控件通知处理程序代码
	mAddrListBox.ResetContent();
	
	DWORD dwPID = 0;
	CString str;
	DWORD dwValue = 0;

	mEditData.GetWindowText(str);	
	dwValue = _tcstoul(str, NULL, 10);

	//鼠标选中的索引
	int selectIndex = mProcessListBox.GetCurSel();
	//根据索引查找ID
	dwPID = (DWORD)mProcessListBox.GetItemData(selectIndex);
	//打开进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (hProcess == NULL)
	{
		AfxMessageBox(TEXT("打开进程失败！"));
	}

	//只需要查找listAddr里面的值
	DWORD dwRead, dwCurValue = 0;
	list<DWORD>::iterator iterTemp, iter = m_listAddr.begin();

	for (; iter != m_listAddr.end(); )
	{
		if (FALSE == ReadProcessMemory(hProcess, (LPVOID)*iter, &dwCurValue, sizeof(dwCurValue), &dwRead))
		{
			continue;
		}
		iterTemp = iter++;
		if (dwCurValue != dwValue)
		{
			m_listAddr.erase(iterTemp);
		}
	}
	iter = m_listAddr.begin();
	for (; iter != m_listAddr.end(); iter++)
	{
		CString s;
		s.Format(TEXT("%#x"), *iter);
		int iListAddr = mAddrListBox.AddString(s);
		mAddrListBox.SetItemData(iListAddr, *iter);
		
	}
	CloseHandle(hProcess);
}


//选中地址按钮
void CGameEditorDlg::OnBnClickedSelect()
{
	// TODO: 在此添加控件通知处理程序代码
	showAddr.Clear();
	int iAddrIndex = mAddrListBox.GetCurSel();
	DWORD dwAddr = (DWORD)mAddrListBox.GetItemData(iAddrIndex);
	CString str;
	str.Format(TEXT("%#x"), dwAddr);
	showAddr.SetWindowTextW(str);
}


//修改按钮
void CGameEditorDlg::OnBnClickedModify()
{
	// TODO: 在此添加控件通知处理程序代码
	int iDataAddrIndex = mAddrListBox.GetCurSel();
	DWORD dwAddr = (DWORD)mAddrListBox.GetItemData(iDataAddrIndex);

	CString str;
	setData.GetWindowTextW(str);
	//获取用户要修改的值
	DWORD dwWalue = _tcstoul(str, NULL, 10);
	int iSeletIndex = mProcessListBox.GetCurSel();
	DWORD dwProcessID = (DWORD)mProcessListBox.GetItemData(iSeletIndex);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
	if (hProcess == NULL)
	{
		AfxMessageBox(TEXT("修改时，打开进程失败！"));
		return;
	}
	DWORD dw01dProtect;
	DWORD dwWriteBytes;
	//修改虚拟地址的读写权限
	VirtualProtectEx(hProcess, (LPVOID)dwAddr, sizeof(dwAddr), PAGE_READWRITE, &dw01dProtect);
	//写入数据
	BOOL nRes = WriteProcessMemory(hProcess, (LPVOID)dwAddr, &dwWalue, sizeof(dwWalue), &dwWriteBytes);
	if (nRes == FALSE)
	{
		int i = GetLastError();
		AfxMessageBox(TEXT("写入失败！"));
		return;
	}
	else
	{
		//成功
		AfxMessageBox(TEXT("修改成功！"));

	}
	//改回原来的地址保护方式
	VirtualProtectEx(hProcess, (LPVOID)dwAddr, sizeof(dwAddr), dw01dProtect, NULL);
	CloseHandle(hProcess);
}
