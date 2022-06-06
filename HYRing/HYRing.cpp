// HYRing.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "HYRing.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
WCHAR szWelcomeText[MAX_LOADSTRING];            // 欢迎使用
WCHAR szMainText[MAX_LOADSTRING];               // 主字符串
HFONT winFont;                                  // 窗口字符字体
BOOL  winTopped;                                // 窗口置顶选择
BOOL  winTransparent;                           // 窗口半透明选择
HANDLE hMonitorThread;
HANDLE hWinTopThread;
WCHAR szReadBlock[2048] = { 0 } ;
HWND hMainText;
WORD ringLoopEnd = 0;
BOOL stopRingFlag = 0;
OPENFILENAME ofn;
WCHAR szLoadCfgFileName[MAX_PATH];

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
VOID CDECL          TimeMonitorProc(LPVOID);
VOID CDECL          WinTopperProc(LPVOID);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_WELCOME, szWelcomeText, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_MAINTEXT, szMainText, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HYRING, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HYRING));

    MSG msg;


    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HYRING));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HYRING);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG winStyle;
    switch (message)
    {
    case WM_CREATE:
        //创建控件
        CreateWindow(_T("STATIC"), szWelcomeText, WS_CHILD | WS_VISIBLE, 10, 10, 350, 20, hWnd, (HMENU)IDM_WELCOME, GetModuleHandle(NULL), NULL);
        hMainText = CreateWindow(_T("STATIC"), szMainText, WS_CHILD | WS_VISIBLE, 10, 35, 350, 40, hWnd, (HMENU)IDM_MAINTEXT, GetModuleHandle(NULL), NULL);
        //设置字体
        //winFont = CreateFont(20, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Microsoft YaHei UI"));
        winFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SendDlgItemMessage(hWnd, IDM_WELCOME, WM_SETFONT, (WPARAM)winFont, MAKELPARAM(TRUE, 0));
        SendDlgItemMessage(hWnd, IDM_MAINTEXT, WM_SETFONT, (WPARAM)winFont, MAKELPARAM(TRUE, 0));
        //启动线程
        hMonitorThread = (HANDLE)_beginthread(TimeMonitorProc, 0, NULL);
        //hWinTopThread = (HANDLE)_beginthread(WinTopperProc, 0, (LPVOID)hWnd);
        winTopped = 1;
        winTransparent = 1;
        //窗口及风格设置
        SetWindowPos((HWND)hWnd, HWND_TOPMOST, 0, 0, 400, 180, 0);
        winStyle = GetWindowLong(hWnd, GWL_STYLE);
        winStyle &= ~(WS_MAXIMIZEBOX) & ~WS_THICKFRAME;
        SetWindowLong(hWnd, GWL_STYLE, winStyle);
        winStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        winStyle ^= WS_EX_LAYERED;
        SetWindowLong(hWnd, GWL_EXSTYLE, winStyle);
        SetLayeredWindowAttributes(hWnd, 0, 200, LWA_ALPHA);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_TOP:
                CheckMenuItem(GetSubMenu(GetMenu(hWnd), 0), 0, MF_BYPOSITION | (winTopped ? MF_UNCHECKED : MF_CHECKED));
                winTopped = !winTopped;
                if (winTopped) {
                    SetWindowPos((HWND)hWnd, HWND_TOPMOST, 0, 0, 400, 180, 0);
                }
                else {
                    SetWindowPos((HWND)hWnd, HWND_NOTOPMOST, 0, 0, 400, 180, 0);
                }
                break;
            case IDM_LOADCFG: 
                memset(&ofn, 0, sizeof(OPENFILENAME));
                memset(szLoadCfgFileName, 0, sizeof(WCHAR) * MAX_PATH);
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.lpstrFilter = L"HYRing Config (.cfg)\0*.cfg";
                ofn.lpstrFile = szLoadCfgFileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST;
                if (GetOpenFileName(&ofn)) {
                    //MessageBox(NULL, szLoadCfgFileName, L"即将打开配置文件...", NULL);
                    ringLoopEnd = 1;
                }
                else {
                    break;
                }
                Sleep(1000);
                ringLoopEnd = 2;
                Sleep(500);
                hMonitorThread = (HANDLE)_beginthread(TimeMonitorProc, 0, NULL);
                break;
            case IDM_STOPRING:
                stopRingFlag = 1;
                break;
            case IDM_TOGGLE_TRANSPARENCY:
                CheckMenuItem(GetSubMenu(GetMenu(hWnd), 1), 2, MF_BYPOSITION | (winTransparent ? MF_UNCHECKED : MF_CHECKED));
                winTransparent = !winTransparent;
                if (winTransparent) {
                    SetLayeredWindowAttributes(hWnd, 0, 200, LWA_ALPHA);
                }
                else {
                    SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
                }
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CLOSE:
        if (MessageBox(hWnd, _T("确认要退出HYRing？"), _T("HYRing - Close"), MB_OKCANCEL) == IDOK)
            SendMessage(hWnd, WM_DESTROY, 0, 0);
        break;
    case WM_DESTROY:
        //CloseHandle(hMonitorThread);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool PlayMP3(LPWSTR szFileName) //UNUSED
{
    MCI_OPEN_PARMS m_mciOpen;  //打开参数  
    MCI_PLAY_PARMS m_mciPlay;  //播放参数  
    m_mciOpen.lpstrDeviceType = L"mpegvideo"; //要操作的文件类型  
    m_mciOpen.lpstrElementName = szFileName; //要操作的文件路径  
    MCIERROR mcierror = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&m_mciOpen); //打开文件命令  
    if (mcierror)
    {
        return false;
    }
    else
    {
        m_mciPlay.dwCallback = (DWORD)NULL;
        m_mciPlay.dwFrom = (DWORD)0; //播放起始位置ms为单位  
        mciSendCommand(m_mciOpen.wDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_FROM, (DWORD)(LPVOID)&m_mciPlay);
    }
    return true;
}

VOID CDECL TimeMonitorProc(LPVOID lpParam) {
    /*
    DWORD dwNumberOfBytesTransffered = 0;
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    if (ReadFile(m_hReadHandle, szReadBlock, sizeof(szReadBlock),
        &dwNumberOfBytesTransffered, &overlapped)) {
        ;
    }
    */
    time_t rawtime;
    struct tm timeinfo;
    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);

    WCHAR szCfgFileName[260] = {0};

    if (ringLoopEnd == 2) {
        wsprintf(szCfgFileName, L"%s", szLoadCfgFileName);
    }
    else {
        wsprintf(szCfgFileName, L"./rings/%d.cfg", timeinfo.tm_wday);
    }
    
    WCHAR szMp3PlayCommand[330] = { 0 };
    FILE* pCfgFile = NULL;
    _wfopen_s(&pCfgFile, szCfgFileName, L"r");
    if (pCfgFile == 0) {
        MessageBox(NULL, L"File Open Error", L"HYRing", 0);
        PostQuitMessage(0);
        _endthread();
    }
    int iHrs[50] = {0}, iMin[50] = {0}, iPtr = 0, iSFlag/*是否指定显示内容*/;
    WCHAR szRingName[260][50] = {0};
    int iRingCount = 0;
    WCHAR szMainTextCopy[300] = {0};

    for (int i = 0; i < 50; i++) {
        fwscanf_s(pCfgFile, L"%d:%d %s\n", &(iHrs[i]), &(iMin[i]), szRingName[i], 260);
        if (iHrs[i] == -1 || iMin[i] == -1) {
            iRingCount = i;
            break;
        }
    }

    while (1) {
        if (ringLoopEnd == 1) {
            //MessageBox(NULL, L"THREAD EXIT", L"", 0);
            //OutputDebugString(L"Thread Exit");
            _fcloseall();
            _endthread();
            return;
        }
        if (stopRingFlag) {
            stopRingFlag = 0;
            wsprintf(szMp3PlayCommand, L"stop ring%d", iPtr - 1);
            mciSendString(szMp3PlayCommand, NULL, 0, NULL);
            MessageBox(NULL, L"铃声已停止！", L"HYRing", 0);
        }
        
        iSFlag = 0;
        time(&rawtime);
        localtime_s(&timeinfo, &rawtime);
        for (; iPtr < iRingCount; iPtr++) {
            if (iHrs[iPtr] > timeinfo.tm_hour) {
                break;
            }
            if (iHrs[iPtr] == timeinfo.tm_hour) {
                if (iMin[iPtr] >= timeinfo.tm_min) {
                    break;
                }
            }
            if (iPtr >= iRingCount - 1) {
                iSFlag = 1;
                wsprintf(szMainTextCopy, L"今日无更多闹铃");
                break;
            }
        }
        if (iPtr > iRingCount - 1) {
            iSFlag = 1;
            wsprintf(szMainTextCopy, L"今日无更多闹铃");
        }
        if (iHrs[iPtr] == timeinfo.tm_hour && iMin[iPtr] == timeinfo.tm_min) {
            //Ring!
            iSFlag = 1;
            wsprintf(szMainTextCopy, L"正在闹铃: %s", szRingName[iPtr]);
            //PlaySound(szRingName[iPtr], NULL, SND_FILENAME | SND_ASYNC);
            wsprintf(szMp3PlayCommand, L"open \"./rings/%s.mp3\" alias ring%d", szRingName[iPtr], iPtr);
            //"open \"./rings/%s.mp3\" type mpegvideo alias ring%d"
            mciSendString(szMp3PlayCommand, NULL, 0, NULL);
            //MessageBox(NULL, szMp3PlayCommand, L"HYRing", 0);
            wsprintf(szMp3PlayCommand, L"play ring%d", iPtr);
            mciSendString(szMp3PlayCommand, NULL, 0, NULL);
            //MessageBox(NULL, szMp3PlayCommand, L"HYRing", 0);
            iPtr++;
        }
        if (!iSFlag) {
            wsprintf(szMainTextCopy, szMainText, timeinfo.tm_wday, iRingCount, iHrs[iPtr], iMin[iPtr], iPtr + 1, szRingName[iPtr]);
        }
        SetWindowText(hMainText, szMainTextCopy);
        Sleep(500);
    }
    _fcloseall();
    _endthread();
}

VOID CDECL WinTopperProc(LPVOID lpParam) {
    while (1) {
        if (winTopped) {
            SetWindowPos((HWND)lpParam, HWND_TOPMOST, 0, 0, 200, 50, 0);
        } else {
            _endthread();
        }
    }
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
