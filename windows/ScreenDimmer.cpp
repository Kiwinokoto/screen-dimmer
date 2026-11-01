#include "DimmerLogic.hpp"

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>

#include <initializer_list>
#include <string>
#include <vector>

namespace {

constexpr wchar_t kMainClass[] = L"KiwinokotoScreenDimmerMain";
constexpr wchar_t kOverlayClass[] = L"KiwinokotoScreenDimmerOverlay";
constexpr wchar_t kMutexName[] = L"Local\\KiwinokotoScreenDimmer";
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT kControlMessage = WM_APP + 2;
constexpr UINT kTrayId = 1;
constexpr int kSwitchId = 101;
constexpr int kSliderId = 102;
constexpr int kValueId = 103;
constexpr int kMenuToggle = 201;
constexpr int kMenuSettings = 202;
constexpr int kMenuExit = 203;

enum class Command : WPARAM { Show = 1, Toggle, Normal, Dim, Startup, Exit };

struct Overlay {
    HWND window = nullptr;
    RECT bounds{};
};

HINSTANCE g_instance = nullptr;
HWND g_main = nullptr;
HWND g_switch = nullptr;
HWND g_slider = nullptr;
HWND g_value = nullptr;
std::vector<Overlay> g_overlays;
screen_dimmer::DimmerState g_state;
std::wstring g_configPath;
UINT g_taskbarCreated = 0;
bool g_trayAdded = false;

void ShowSettings();
void RefreshOverlays();
void UpdateControls();

std::wstring ConfigPath() {
    PWSTR local = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &local))) {
        return L"";
    }
    std::wstring dir(local);
    CoTaskMemFree(local);
    dir += L"\\ScreenDimmer";
    if (!CreateDirectoryW(dir.c_str(), nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return L"";
    }
    return dir + L"\\settings.ini";
}

void SaveState() {
    if (g_configPath.empty()) return;
    const std::wstring level = std::to_wstring(g_state.brightness);
    WritePrivateProfileStringW(L"dimmer", L"brightness", level.c_str(), g_configPath.c_str());
    WritePrivateProfileStringW(L"dimmer", L"mode", g_state.enabled ? L"dimmed" : L"normal",
                               g_configPath.c_str());
}

void LoadPreference() {
    g_configPath = ConfigPath();
    if (!g_configPath.empty()) {
        g_state.brightness = screen_dimmer::ClampBrightness(
            GetPrivateProfileIntW(L"dimmer", L"brightness", 55, g_configPath.c_str()));
    }
    // A new process always starts in a safe, undimmed state.
    g_state.enabled = false;
}

LRESULT CALLBACK OverlayProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT paint{};
            HDC dc = BeginPaint(window, &paint);
            FillRect(dc, &paint.rcPaint, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
            EndPaint(window, &paint);
            return 0;
        }
        default:
            return DefWindowProcW(window, message, wParam, lParam);
    }
}

BOOL CALLBACK AddMonitor(HMONITOR monitor, HDC, LPRECT, LPARAM) {
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) return TRUE;

    const RECT& bounds = info.rcMonitor;
    HWND overlay = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        kOverlayClass, L"", WS_POPUP, bounds.left, bounds.top,
        bounds.right - bounds.left, bounds.bottom - bounds.top,
        nullptr, nullptr, g_instance, nullptr);
    if (overlay) {
        SetLayeredWindowAttributes(overlay, 0, 0, LWA_ALPHA);
        g_overlays.push_back({overlay, bounds});
    }
    return TRUE;
}

void ApplyOverlays() {
    const unsigned char alpha = screen_dimmer::OverlayAlpha(g_state.brightness);
    const bool show = g_state.enabled && alpha != 0;
    for (const auto& overlay : g_overlays) {
        if (!show) {
            ShowWindow(overlay.window, SW_HIDE);
            continue;
        }
        SetLayeredWindowAttributes(overlay.window, 0, alpha, LWA_ALPHA);
        SetWindowPos(overlay.window, HWND_TOPMOST, overlay.bounds.left, overlay.bounds.top,
                     overlay.bounds.right - overlay.bounds.left,
                     overlay.bounds.bottom - overlay.bounds.top,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    // Keep the settings window usable and readable above the filter.
    if (g_main && IsWindowVisible(g_main)) {
        SetWindowPos(g_main, HWND_TOPMOST, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void RefreshOverlays() {
    for (const auto& overlay : g_overlays) DestroyWindow(overlay.window);
    g_overlays.clear();
    EnumDisplayMonitors(nullptr, nullptr, AddMonitor, 0);
    ApplyOverlays();
}

void SetEnabled(bool enabled) {
    g_state.enabled = enabled;
    ApplyOverlays();
    UpdateControls();
    SaveState();
}

void SetBrightness(int percent) {
    g_state.SetBrightness(percent);
    ApplyOverlays();
    UpdateControls();
    SaveState();
}

void UpdateControls() {
    if (g_switch) InvalidateRect(g_switch, nullptr, TRUE);
    if (g_slider) SendMessageW(g_slider, TBM_SETPOS, TRUE, g_state.brightness);
    if (g_value) {
        std::wstring label = std::to_wstring(g_state.brightness) + L" %";
        SetWindowTextW(g_value, label.c_str());
    }
}

void AddTrayIcon() {
    NOTIFYICONDATAW icon{};
    icon.cbSize = sizeof(icon);
    icon.hWnd = g_main;
    icon.uID = kTrayId;
    icon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    icon.uCallbackMessage = kTrayMessage;
    icon.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    lstrcpynW(icon.szTip, L"Screen Dimmer", ARRAYSIZE(icon.szTip));
    g_trayAdded = Shell_NotifyIconW(NIM_ADD, &icon) != FALSE;
    if (g_trayAdded) {
        icon.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &icon);
    }
}

void RemoveTrayIcon() {
    if (!g_trayAdded) return;
    NOTIFYICONDATAW icon{};
    icon.cbSize = sizeof(icon);
    icon.hWnd = g_main;
    icon.uID = kTrayId;
    Shell_NotifyIconW(NIM_DELETE, &icon);
    g_trayAdded = false;
}

void ShowTrayMenu() {
    POINT point{};
    GetCursorPos(&point);
    HMENU menu = CreatePopupMenu();
    if (!menu) return;
    AppendMenuW(menu, MF_STRING, kMenuToggle,
                g_state.enabled ? L"Désactiver le filtre" : L"Activer le filtre");
    AppendMenuW(menu, MF_STRING, kMenuSettings, L"Régler la luminosité...");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kMenuExit, L"Quitter");
    SetForegroundWindow(g_main);
    const UINT selection = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
                                           point.x, point.y, 0, g_main, nullptr);
    DestroyMenu(menu);
    if (selection == kMenuToggle) SetEnabled(!g_state.enabled);
    if (selection == kMenuSettings) ShowSettings();
    if (selection == kMenuExit) DestroyWindow(g_main);
    PostMessageW(g_main, WM_NULL, 0, 0);
}

void ShowSettings() {
    ShowWindow(g_main, SW_SHOWNORMAL);
    SetWindowPos(g_main, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    SetForegroundWindow(g_main);
}

void DrawSwitch(const DRAWITEMSTRUCT& draw) {
    HDC dc = draw.hDC;
    const bool enabled = g_state.enabled;
    RECT background = draw.rcItem;
    FillRect(dc, &background, reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1));
    const RECT pill{4, 4, 58, 30};
    HBRUSH fill = CreateSolidBrush(enabled ? RGB(40, 143, 92) : RGB(130, 135, 142));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, fill));
    HPEN pen = CreatePen(PS_NULL, 0, 0);
    HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
    RoundRect(dc, pill.left, pill.top, pill.right, pill.bottom, 28, 28);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(fill);
    DeleteObject(pen);

    HBRUSH thumb = CreateSolidBrush(RGB(255, 255, 255));
    oldBrush = static_cast<HBRUSH>(SelectObject(dc, thumb));
    Ellipse(dc, enabled ? 34 : 8, 8, enabled ? 54 : 28, 28);
    SelectObject(dc, oldBrush);
    DeleteObject(thumb);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, GetSysColor(COLOR_BTNTEXT));
    RECT textRect{72, 4, background.right, 30};
    DrawTextW(dc, enabled ? L"Activé" : L"Désactivé", -1, &textRect,
              DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    if (draw.itemState & ODS_FOCUS) DrawFocusRect(dc, &background);
}

void Dispatch(Command command) {
    switch (command) {
        case Command::Show: ShowSettings(); break;
        case Command::Toggle: SetEnabled(!g_state.enabled); break;
        case Command::Normal:
        case Command::Startup: SetEnabled(false); break;
        case Command::Dim: SetEnabled(true); break;
        case Command::Exit: DestroyWindow(g_main); break;
    }
}

LRESULT CALLBACK MainProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == g_taskbarCreated) {
        g_trayAdded = false;
        AddTrayIcon();
        return 0;
    }
    switch (message) {
        case WM_CREATE: {
            g_main = window;
            HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            HWND title = CreateWindowW(L"STATIC", L"Screen Dimmer", WS_CHILD | WS_VISIBLE,
                                        20, 16, 305, 26, window, nullptr, g_instance, nullptr);
            g_switch = CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                                      20, 52, 290, 34, window,
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSwitchId)), g_instance, nullptr);
            HWND label = CreateWindowW(L"STATIC", L"Luminosité apparente", WS_CHILD | WS_VISIBLE,
                                        20, 103, 230, 22, window, nullptr, g_instance, nullptr);
            g_value = CreateWindowW(L"STATIC", L"55 %", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                      264, 103, 60, 22, window,
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(kValueId)), g_instance, nullptr);
            g_slider = CreateWindowW(TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                                      TBS_HORZ | TBS_NOTICKS, 16, 130, 310, 32, window,
                                      reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSliderId)), g_instance, nullptr);
            HWND hint = CreateWindowW(L"STATIC", L"Fermer cette fenêtre laisse le filtre actif.",
                                       WS_CHILD | WS_VISIBLE, 20, 174, 310, 22,
                                       window, nullptr, g_instance, nullptr);
            for (HWND item : {title, g_switch, label, g_value, hint}) {
                SendMessageW(item, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            }
            SendMessageW(g_slider, TBM_SETRANGE, TRUE, MAKELPARAM(10, 100));
            UpdateControls();
            AddTrayIcon();
            RefreshOverlays();
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == kSwitchId && HIWORD(wParam) == BN_CLICKED) {
                SetEnabled(!g_state.enabled);
                return 0;
            }
            break;
        case WM_DRAWITEM:
            if (wParam == kSwitchId && lParam) {
                DrawSwitch(*reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
                return TRUE;
            }
            break;
        case WM_HSCROLL:
            if (reinterpret_cast<HWND>(lParam) == g_slider) {
                const int value = static_cast<int>(SendMessageW(g_slider, TBM_GETPOS, 0, 0));
                if (value != g_state.brightness) SetBrightness(value);
                return 0;
            }
            break;
        case kControlMessage:
            Dispatch(static_cast<Command>(wParam));
            return 0;
        case kTrayMessage:
            if (LOWORD(lParam) == WM_LBUTTONUP || LOWORD(lParam) == NIN_SELECT) ShowSettings();
            if (LOWORD(lParam) == WM_LBUTTONDBLCLK) SetEnabled(!g_state.enabled);
            if (LOWORD(lParam) == WM_RBUTTONUP || LOWORD(lParam) == WM_CONTEXTMENU) ShowTrayMenu();
            return 0;
        case WM_DISPLAYCHANGE:
        case WM_DEVICECHANGE:
            RefreshOverlays();
            return 0;
        case WM_CLOSE:
            ShowWindow(window, SW_HIDE);
            return 0;
        case WM_DESTROY:
            RemoveTrayIcon();
            for (const auto& overlay : g_overlays) DestroyWindow(overlay.window);
            g_overlays.clear();
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

Command ParseCommand(const wchar_t* line) {
    if (line && *line) {
        std::wstring command(line);
        while (!command.empty() && (command.front() == L' ' || command.front() == L'"')) {
            command.erase(command.begin());
        }
        while (!command.empty() && (command.back() == L' ' || command.back() == L'"')) {
            command.pop_back();
        }
        if (command == L"toggle" || command == L"--toggle") return Command::Toggle;
        if (command == L"normal" || command == L"--normal") return Command::Normal;
        if (command == L"dim" || command == L"--dim") return Command::Dim;
        if (command == L"startup" || command == L"--startup") return Command::Startup;
        if (command == L"quit" || command == L"--quit") return Command::Exit;
    }
    return Command::Show;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
    g_instance = instance;
    const Command command = ParseCommand(commandLine);
    HANDLE mutex = CreateMutexW(nullptr, TRUE, kMutexName);
    if (!mutex) return 1;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // A second invocation forwards the desktop-shortcut command to the tray process.
        for (int attempt = 0; attempt < 20; ++attempt) {
            HWND existing = FindWindowW(kMainClass, nullptr);
            if (existing) {
                DWORD_PTR ignored = 0;
                SendMessageTimeoutW(existing, kControlMessage, static_cast<WPARAM>(command), 0,
                                    SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, &ignored);
                CloseHandle(mutex);
                return 0;
            }
            Sleep(50);
        }
        CloseHandle(mutex);
        return 1;
    }

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    INITCOMMONCONTROLSEX common{};
    common.dwSize = sizeof(common);
    common.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&common);
    LoadPreference();
    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");

    WNDCLASSW overlayClass{};
    overlayClass.hInstance = instance;
    overlayClass.lpfnWndProc = OverlayProc;
    overlayClass.lpszClassName = kOverlayClass;
    overlayClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    WNDCLASSW mainClass{};
    mainClass.hInstance = instance;
    mainClass.lpfnWndProc = MainProc;
    mainClass.lpszClassName = kMainClass;
    mainClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    mainClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    mainClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    if (!RegisterClassW(&overlayClass) || !RegisterClassW(&mainClass)) {
        CloseHandle(mutex);
        return 1;
    }

    const int left = (GetSystemMetrics(SM_CXSCREEN) - 360) / 2;
    const int top = (GetSystemMetrics(SM_CYSCREEN) - 250) / 2;
    HWND window = CreateWindowExW(0, kMainClass, L"Screen Dimmer — Windows",
                                  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                  left, top, 360, 250, nullptr, nullptr, instance, nullptr);
    if (!window) {
        CloseHandle(mutex);
        return 1;
    }
    // Startup explicitly drops any previous dim state, but retains the slider value.
    SaveState();
    if (command == Command::Dim || command == Command::Toggle) Dispatch(command);
    if (command == Command::Show) ShowSettings();
    if (command == Command::Exit) DestroyWindow(window);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    CloseHandle(mutex);
    return static_cast<int>(message.wParam);
}
