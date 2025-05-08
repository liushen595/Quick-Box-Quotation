#pragma once

#include <windows.h>
#include <shellapi.h>
#include <string>

// 定义托盘消息ID
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_AUTOSTART 1002

class SystemTray {
public:
    SystemTray();
    ~SystemTray();

    // 初始化系统托盘
    bool Initialize(HINSTANCE hInstance);

    // 检查当前是否以管理员权限运行
    static bool IsRunAsAdmin();

    // 添加或删除开机自启动
    static bool SetAutoStart(bool enable);

    // 检查是否已设置开机自启动
    static bool IsAutoStartEnabled();

    // 隐藏控制台窗口
    static void HideConsoleWindow();

    // 显示消息框
    static void ShowMessage(const wchar_t* message, const wchar_t* title, UINT type = MB_ICONINFORMATION);

private:
    // 创建隐藏窗口
    HWND CreateHiddenWindow(HINSTANCE hInstance);

    // 创建系统托盘图标
    void CreateTrayIcon();

    // 窗口过程函数（静态）
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;                 // 窗口句柄
    NOTIFYICONDATA m_nid;        // 托盘图标数据
    bool m_autoStartEnabled;     // 自启动状态
    static SystemTray* s_instance; // 静态实例指针用于窗口回调
};