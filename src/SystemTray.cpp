// #define UNICODE
// #define _UNICODE
#include "../lib/SystemTray.h"

// 初始化静态实例指针
SystemTray* SystemTray::s_instance = nullptr;

SystemTray::SystemTray() : m_hWnd(NULL), m_autoStartEnabled(false) {
    ZeroMemory(&m_nid, sizeof(NOTIFYICONDATA));
    s_instance = this;
}

SystemTray::~SystemTray() {
    if (m_hWnd) {
        Shell_NotifyIcon(NIM_DELETE, &m_nid);
        DestroyWindow(m_hWnd);
    }
    s_instance = nullptr;
}

bool SystemTray::Initialize(HINSTANCE hInstance) {
    // 检查是否已设置开机自启动
    m_autoStartEnabled = IsAutoStartEnabled();

    // 检查管理员权限
    bool isAdmin = IsRunAsAdmin();
    if (!isAdmin) {
        // 仅显示通知，不强制要求管理员权限
        ShowMessage(L"注意：某些功能可能需要管理员权限才能正常工作", L"提示");
    }

    // 创建用于处理托盘消息的隐藏窗口
    m_hWnd = CreateHiddenWindow(hInstance);
    if (!m_hWnd) {
        ShowMessage(L"创建窗口失败", L"错误", MB_ICONERROR);
        return false;
    }

    // 创建系统托盘图标
    CreateTrayIcon();

    return true;
}

bool SystemTray::IsRunAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(adminGroup);
    }
    return isAdmin;
}

bool SystemTray::SetAutoStart(bool enable) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_ALL_ACCESS, &hKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    if (enable) {
        wchar_t path[MAX_PATH];
        GetModuleFileName(NULL, path, MAX_PATH);
        RegSetValueEx(hKey, L"KeyboardReplace", 0, REG_SZ, (BYTE*)path,
            (wcslen(path) + 1) * sizeof(wchar_t));
    }
    else {
        RegDeleteValue(hKey, L"KeyboardReplace");
    }

    RegCloseKey(hKey);
    return true;
}

bool SystemTray::IsAutoStartEnabled() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    wchar_t path[MAX_PATH] = {0};
    DWORD size = sizeof(path);
    DWORD type = REG_SZ;

    result = RegQueryValueEx(hKey, L"KeyboardReplace", 0, &type, (LPBYTE)path, &size);
    RegCloseKey(hKey);

    return (result == ERROR_SUCCESS && size > 0);
}

void SystemTray::HideConsoleWindow() {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
}

void SystemTray::ShowMessage(const wchar_t* message, const wchar_t* title, UINT type) {
    MessageBox(NULL, message, title, type);
}

HWND SystemTray::CreateHiddenWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"KeyboardReplaceClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        L"KeyboardReplaceClass", L"KeyboardReplace",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    return hwnd;
}

void SystemTray::CreateTrayIcon() {
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hWnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION); // 使用默认图标，也可以加载自定义图标
    lstrcpy(m_nid.szTip, L"键盘替换工具"); // 鼠标悬停提示文本

    Shell_NotifyIcon(NIM_ADD, &m_nid);
}

LRESULT CALLBACK SystemTray::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // 确保实例存在
    if (s_instance == nullptr) {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch (uMsg) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();
            if (hMenu) {
                // 添加自启动选项
                UINT autoStartFlags = MF_STRING | (s_instance->m_autoStartEnabled ? MF_CHECKED : MF_UNCHECKED);
                InsertMenu(hMenu, -1, autoStartFlags, ID_TRAY_AUTOSTART, L"开机自启动");

                // 添加分隔线和退出选项
                InsertMenu(hMenu, -1, MF_SEPARATOR, 0, NULL);
                InsertMenu(hMenu, -1, MF_STRING, ID_TRAY_EXIT, L"退出");

                // 窗口必须是前台窗口才能显示菜单
                SetForegroundWindow(hwnd);

                // 显示菜单
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
        }
        else if (lParam == WM_LBUTTONDBLCLK) {
            // 可以在这里添加双击托
            ShowMessage(L"键盘替换工具正在运行", L"提示");
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_EXIT:
            // 发送自定义消息通知主程序退出
            PostQuitMessage(0);
            return 0;

        case ID_TRAY_AUTOSTART:
            // 切换自启动状态
            s_instance->m_autoStartEnabled = !s_instance->m_autoStartEnabled;
            if (!SetAutoStart(s_instance->m_autoStartEnabled)) {
                ShowMessage(L"设置自启动失败，可能需要管理员权限",
                    L"错误", MB_ICONERROR);
                s_instance->m_autoStartEnabled = !s_instance->m_autoStartEnabled; // 还原状态
            }
            return 0;
        }
        break;

    case WM_DESTROY:
        // 删除托盘图标并退出
        Shell_NotifyIcon(NIM_DELETE, &s_instance->m_nid);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}