#include <windows.h>
#include <iostream>
#include <chrono>
using namespace std::chrono;

HHOOK hHook;
bool isCtrlDown = false;  // 改为检测Ctrl键
auto lastPressTime = steady_clock::now();
bool toggleState = false;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (p->vkCode == VK_CONTROL || p->vkCode == VK_LCONTROL || p->vkCode == VK_RCONTROL) {
                isCtrlDown = true;
            }

            // 拦截引号键 (VK_OEM_7对应单引号/双引号键)
            if (p->vkCode == VK_OEM_7 && isCtrlDown) {
                auto now = steady_clock::now();
                auto duration = duration_cast<milliseconds>(now - lastPressTime).count();
                lastPressTime = now;

                // 模拟输入
                INPUT input = {0};
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = 0;
                input.ki.wScan = 0;
                input.ki.dwFlags = KEYEVENTF_UNICODE;
                input.ki.wVk = 0;

                if (toggleState) {
                    input.ki.wScan = L'」';  // 第二次按下输出右中文引号
                }
                else {
                    input.ki.wScan = L'「';  // 第一次按下输出左中文引号
                }

                // 切换状态，下次输出另一个引号
                toggleState = !toggleState;

                SendInput(1, &input, sizeof(INPUT));
                return 1; // 不传递原始按键
            }
        }

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (p->vkCode == VK_CONTROL || p->vkCode == VK_LCONTROL || p->vkCode == VK_RCONTROL) {
                isCtrlDown = false;
            }
        }
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int main() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

    if (!hHook) {
        MessageBoxW(NULL, L"无法安装钩子", L"错误", MB_ICONERROR);
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
