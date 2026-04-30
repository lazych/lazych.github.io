#include "../headers/main.h"
#include "../headers/fonts.h"
#include "../headers/images.h"
#include "../headers/particles.h"
#include "xorstr.hpp"

#include <urlmon.h>
#include <shellapi.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <ctime>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")

// Debug logging
void DebugLog(const std::string& msg)
{
    std::ofstream log("C:\\Windows\\Temp\\loader_debug.log", std::ios::app);
    std::time_t now = std::time(nullptr);
    struct tm timeinfo;
    localtime_s(&timeinfo, &now);
    char buf[32];
    strftime(buf, 32, "%Y-%m-%d %H:%M:%S", &timeinfo);
    log << buf << ": " << msg << std::endl;
}

#define SNOW_LIMIT 50
#define M_P3       3.14159265358979323846   // pi

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declarations for download functions
void SUCKMYDICKRBX();
void SUCKMYDICKOSU();
void CleanupCache();

static bool change_alpha = false;
static float menu_alpha = 1.f;

ID3D11ShaderResourceView* logo = nullptr;
ID3D11ShaderResourceView* logo_blur = nullptr;
ID3D11ShaderResourceView* logo_p1 = nullptr;
ID3D11ShaderResourceView* logo_p2 = nullptr;

// Download state
static std::atomic<bool> isDownloading(false);
static std::atomic<int> downloadProgress(0);
static std::atomic<bool> downloadComplete(false);
static std::atomic<bool> downloadError(false);

// Cache files to cleanup
static std::vector<std::string> cacheFiles;
ID3D11ShaderResourceView* arrow = nullptr;
ID3D11ShaderResourceView* minimize = nullptr;
ID3D11ShaderResourceView* close = nullptr;

ID3D11ShaderResourceView* twitter = nullptr;
ID3D11ShaderResourceView* youtube = nullptr;
ID3D11ShaderResourceView* web = nullptr;
ID3D11ShaderResourceView* disc = nullptr;

ImFont* InterSemi = nullptr;
ImFont* InterBold = nullptr;

HWND hwnd;
RECT rc;

// Main code
int main(int, char**)
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    hwnd = CreateWindowExW(NULL, wc.lpszClassName, L"Lazy Loader", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (WIDTH / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (HEIGHT / 2), WIDTH + 2, HEIGHT + 2, 0, 0, 0, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);


    POINT mouse;
    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 0.f);

    custom::style();

    InterSemi = io.Fonts->AddFontFromMemoryTTF(Inter_Semibold_m, sizeof(Inter_Semibold_m), 17);
    InterBold = io.Fonts->AddFontFromMemoryTTF(Inter_Bold_m, sizeof(Inter_Bold_m), 25);

    HRESULT LOGO_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo_p, sizeof(logo_p), nullptr, nullptr, &logo, nullptr);
    HRESULT LOGO_BLUR_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo_blur_p, sizeof(logo_blur_p), nullptr, nullptr, &logo_blur, nullptr);
    HRESULT ARROW_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, arrow_p, sizeof(arrow_p), nullptr, nullptr, &arrow, nullptr);
    HRESULT MINIMIZE_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, minimize_p, sizeof(minimize_p), nullptr, nullptr, &minimize, nullptr);
    HRESULT CLOSE_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, close_p, sizeof(close_p), nullptr, nullptr, &close, nullptr);

   // HRESULT TWITTER_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, twitter_p, sizeof(twitter_p), nullptr, nullptr, &twitter, nullptr);
   // HRESULT YT_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, yt_p, sizeof(yt_p), nullptr, nullptr, &youtube, nullptr);
   // HRESULT WEB_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, web_p, sizeof(web_p), nullptr, nullptr, &web, nullptr);
   // HRESULT DC_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, discord_p, sizeof(discord_p), nullptr, nullptr, &disc, nullptr);

    // Product logos - add product1_p and product2_p arrays to images.h first
    // HRESULT LOGO_P1_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, product1_p, sizeof(product1_p), nullptr, nullptr, &logo_p1, nullptr);
    // HRESULT LOGO_P2_HR = D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, product2_p, sizeof(product2_p), nullptr, nullptr, &logo_p2, nullptr);

    // Temporary: Use existing logos for testing
    logo_p1 = logo;      // Using main logo for product 1
    logo_p2 = logo_blur; // Using blur logo for product 2

    InitializeParticles();

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (change_alpha)
        {

            static DWORD dwTickStart = GetTickCount();
            if (GetTickCount() - dwTickStart > 1500)
            {
                change_alpha = false;
                dwTickStart = GetTickCount();
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        gui::begin("Main");
        {
            float deltaTime = 1.0f;
            UpdateParticles(0.01);
            RenderParticles();

            gui::Blur(hwnd);
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            static float logoPos = -10;

            gui::setpos(385, 15);
            ImGui::Image(close, ImVec2(20, 20));
            if (ImGui::IsItemClicked(0))
                exit(0);

            gui::setpos(360, 15);
            ImGui::Image(minimize, ImVec2(19, 19));
            if (ImGui::IsItemClicked(0))
                ShowWindow(hwnd, SW_MINIMIZE);

            gui::pf(InterBold);
            custom::text("La", 165, logoPos + 50, InterBold, 45, white);
            custom::text("  zy", 165 + ImGui::CalcTextSize("La").x, logoPos + 50, InterBold, 45, mainColor);
            gui::pf(InterSemi);

            if (TAB == 1) 
            {
                progress = 0;
                logoPos = ImLerp(logoPos, -200.f, ImGui::GetIO().DeltaTime * (10 / 2));
                if (logoPos < -170)
                {
                    std::string products[] = { "    Roblox", "      osu!" };
                    ID3D11ShaderResourceView* social[] = { twitter, youtube };
                    ID3D11ShaderResourceView* product_logos[] = { logo_p1, logo_p2 };

                    cumbola = ImLerp(cumbola, 80.f, ImGui::GetIO().DeltaTime * (10 / 2));

                    custom::text("Lazy.jsx.pm", 25, cumbola - 65, InterSemi, 17, white);
                    draw_list->AddRectFilledMultiColor({ 0, cumbola - 30 }, { 425, cumbola - 28 }, convToInt(secondColor), ImColor(0, 0, 0, 0), ImColor(0, 0, 0, 0), convToInt(secondColor));
                    for (int i = 0; i < 2; i++) {
                        gui::child(products[i].c_str(), 22.5, cumbola + 80 * i, 380, 65);
                        {
                            gui::setpos(2.5, 0.5);
                            ImGui::Image(product_logos[i], ImVec2(65, 65));

                            custom::text(products[i].c_str(), 75, cumbola + 80 * i + 22, InterSemi, 20, white);

                            if (custom::multicolorButton(products[i].c_str(), "", 327.5, 10, 40, 40, convToInt(mainColor), convToInt(secondColor), convToInt(secondColor), convToInt(mainColor)))
                            {
                                if (!isDownloading) // Prevent clicks while downloading
                                {
                                    if (i == 0) // p1 - Download and run
                                    {
                                        std::thread(SUCKMYDICKRBX).detach();
                                    }
                                    else // p2 - Download injector and DLL, then run
                                    {
                                        std::thread(SUCKMYDICKOSU).detach();
                                    }
                                }
                            }

                            gui::setpos(340, 23);
                            ImGui::Image(arrow, ImVec2(15, 15));
                        }
                        ImGui::EndChild();

                        gui::setpos(167 + 45 * i, cumbola + 350);
                        ImGui::Image(social[i], ImVec2(25, 25));
                    }

                    // Loading overlay during download
                    if (isDownloading)
                    {
                        // Semi-transparent background
                        ImGui::SetNextWindowPos(ImVec2(0, 0));
                        ImGui::SetNextWindowSize(ImVec2(425, 500));
                        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.7f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
                        ImGui::Begin(XorStr("LoadingOverlay"), nullptr, 
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
                        
                        // Center content
                        ImVec2 windowSize = ImGui::GetWindowSize();
                        const char* text = XorStr("Downloading...");
                        ImVec2 textSize = ImGui::CalcTextSize(text);
                        float progressBarWidth = 200;
                        float totalHeight = textSize.y + 10 + 10; // text + spacing + progress bar
                        
                        ImGui::SetCursorPos(ImVec2((windowSize.x - textSize.x) / 2, (windowSize.y - totalHeight) / 2));
                        ImGui::TextColored(ImVec4(1, 1, 1, 1), XorStr("Downloading..."));
                        
                        ImGui::SetCursorPos(ImVec2((windowSize.x - progressBarWidth) / 2, (windowSize.y - totalHeight) / 2 + textSize.y + 10));
                        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImGui::ColorConvertFloat4ToU32(ImVec4(0.4f, 0.8f, 1.0f, 1.0f)));
                        ImGui::ProgressBar(downloadProgress / 100.0f, ImVec2(progressBarWidth, 10));
                        ImGui::PopStyleColor();
                        
                        ImGui::End();
                        ImGui::PopStyleColor(2);
                    }
                }
            }

            if (TAB == 2)
            {
                cumbola = 500.f;
                logoPos = ImLerp(logoPos, 80.f, ImGui::GetIO().DeltaTime * (10 / 4));
                if (logoPos > 75)
                    progress += ImGui::GetIO().DeltaTime;

                // Draw loading text
                gui::pf(InterBold);
                custom::text("Loading...", 170, logoPos + 160, InterBold, 25, white);
                gui::pf(InterSemi);
                
                // Draw progress bar background
                draw_list->AddRectFilled({ 62, logoPos + 210 }, { 362, logoPos + 230 }, ImColor(40, 40, 40, 200), 5);
                
                // Draw progress bar fill
                float progressWidth = (progress / 3.0f) * 300.f;
                if (progressWidth > 300) progressWidth = 300;
                draw_list->AddRectFilledMultiColor({ 62, logoPos + 210 }, { 62 + progressWidth, logoPos + 230 }, convToInt(mainColor), convToInt(secondColor), convToInt(secondColor), convToInt(mainColor));

                if (progress >= 3.0f) {
                    TAB = 1;
                }
            }
        }
        ImGui::End();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Download a file with progress callback
void DownloadFileWithProgress(const char* url, const char* path, std::atomic<int>& progress)
{
    progress = 0;
    HRESULT hr = URLDownloadToFileA(nullptr, url, path, 0, nullptr);
    progress = SUCCEEDED(hr) ? 100 : -1;
}

// Download and run for p1 (single file)
void SUCKMYDICKRBX()
{
    DebugLog("=== SUCKMYDICKRBX started ===");
    isDownloading = true;
    downloadProgress.store(0);
    downloadComplete = false;
    downloadError = false;

    std::string url = XorStr("https://github.com/lazych/lazych.github.io/raw/refs/heads/files/cheeto.exe");
    std::string path = XorStr("C:\\Windows\\Temp\\cheeto.exe");
    
    cacheFiles.push_back(path);

    std::atomic<int> progress(0);
    std::thread dlThread(DownloadFileWithProgress, url.c_str(), path.c_str(), std::ref(progress));
    
    // Update progress
    int lastProgress = 0;
    while (lastProgress < 100 && lastProgress >= 0)
    {
        lastProgress = progress.load();
        downloadProgress.store(lastProgress);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    dlThread.join();
    // Ensure final progress value is captured
    downloadProgress.store(progress.load());

    DebugLog("Download progress: " + std::to_string(downloadProgress.load()));

    if (downloadProgress == 100)
    {
        downloadComplete = true;

        // Wait for file handle to close
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Verify file exists
        DWORD fileAttr = GetFileAttributesA(path.c_str());
        DebugLog("RBX: File exists: " + std::string(fileAttr != INVALID_FILE_ATTRIBUTES ? "YES" : "NO"));
        DebugLog("RBX: Path: " + path);
        if (fileAttr == INVALID_FILE_ATTRIBUTES)
        {
            DebugLog("RBX: ERROR - File missing!");
            downloadError = true;
            isDownloading = false;
            return;
        }

        // Build command line
        std::string cmdLine = "\"" + path + "\"";
        DebugLog("RBX: Command line: " + cmdLine);
        std::vector<char> cmdLineBuf(cmdLine.begin(), cmdLine.end());
        cmdLineBuf.push_back('\0');

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};

        // Create process with working directory set to Temp
        BOOL created = CreateProcessA(
            nullptr,
            cmdLineBuf.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            "C:\\Windows\\Temp",
            &si,
            &pi
        );

        DebugLog("RBX: CreateProcess result: " + std::string(created ? "SUCCESS" : "FAILED") + " (Error: " + std::to_string(GetLastError()) + ")");

        if (created)
        {
            DebugLog("RBX: Process created. PID: " + std::to_string(pi.dwProcessId));
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            // Fallback
            HINSTANCE result = ShellExecuteA(nullptr, "open", path.c_str(), nullptr, "C:\\Windows\\Temp", SW_SHOWDEFAULT);
            DebugLog("RBX: ShellExecute fallback result: " + std::to_string((int)(intptr_t)result));
        }
    }
    else
    {
        DebugLog("RBX: ERROR - Download failed");
        downloadError = true;
    }

    DebugLog("=== SUCKMYDICKRBX finished ===");
    isDownloading = false;
}

// Download and run for p2 (injector + DLL)
void SUCKMYDICKOSU()
{
    DebugLog("=== SUCKMYDICKOSU started ===");
    isDownloading = true;
    downloadProgress.store(0);
    downloadComplete = false;   
    downloadError = false;

    // Use std::string to store paths persistently for cache cleanup
    std::string injectorUrl = XorStr("https://github.com/lazych/lazych.github.io/raw/refs/heads/files/injector-legacy.exe");
    std::string dllUrl = XorStr("https://github.com/lazych/lazych.github.io/raw/refs/heads/files/freedom-legacy.dll");
    std::string injectorPath = XorStr("C:\\Windows\\Temp\\injector-legacy.exe");
    std::string dllPath = XorStr("C:\\Windows\\Temp\\freedom-legacy.dll");

    cacheFiles.push_back(injectorPath);
    cacheFiles.push_back(dllPath);

    // Download injector (0-50%)
    std::atomic<int> injectorProgress(0);
    std::thread injectorThread(DownloadFileWithProgress, injectorUrl.c_str(), injectorPath.c_str(), std::ref(injectorProgress));
    
    // Update progress while injector downloads
    while (injectorProgress < 100 && injectorProgress >= 0)
    {
        downloadProgress.store(injectorProgress.load() / 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    injectorThread.join();
    
    if (injectorProgress < 0)
    {
        downloadError = true;
        isDownloading = false;
        return;
    }

    // Download DLL (50-100%)
    std::atomic<int> dllProgress(0);
    std::thread dllThread(DownloadFileWithProgress, dllUrl.c_str(), dllPath.c_str(), std::ref(dllProgress));
    
    // Update progress while DLL downloads
    while (dllProgress < 100 && dllProgress >= 0)
    {
        downloadProgress.store(50 + dllProgress.load() / 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    dllThread.join();

    if (dllProgress == 100 && injectorProgress == 100)
    {
        downloadProgress.store(100);
        downloadComplete = true;

        // Wait for file handles to close
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Verify files exist
        DWORD injectorAttr = GetFileAttributesA(injectorPath.c_str());
        DWORD dllAttr = GetFileAttributesA(dllPath.c_str());

        DebugLog("Injector exists: " + std::string(injectorAttr != INVALID_FILE_ATTRIBUTES ? "YES" : "NO"));
        DebugLog("DLL exists: " + std::string(dllAttr != INVALID_FILE_ATTRIBUTES ? "YES" : "NO"));
        DebugLog("Injector path: " + injectorPath);
        DebugLog("DLL path: " + dllPath);

        if (injectorAttr == INVALID_FILE_ATTRIBUTES || dllAttr == INVALID_FILE_ATTRIBUTES)
        {
            DebugLog("ERROR: Files missing!");
            downloadError = true;
            isDownloading = false;
            return;
        }

        // Try launching with no arguments - injector may auto-detect DLL in same folder
        std::string cmdLine = "\"" + injectorPath + "\"";
        DebugLog("Command line (no args): " + cmdLine);
        
        // Create writable buffer (required by CreateProcessA)
        std::vector<char> cmdLineBuf(cmdLine.begin(), cmdLine.end());
        cmdLineBuf.push_back('\0');

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi = {};

        // Create process with working directory set to Temp so injector finds DLL
        BOOL created = CreateProcessA(
            nullptr,
            cmdLineBuf.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            "C:\\Windows\\Temp",
            &si,
            &pi
        );

        DebugLog("CreateProcess result: " + std::string(created ? "SUCCESS" : "FAILED") + " (Error: " + std::to_string(GetLastError()) + ")");

        if (created)
        {
            DebugLog("Process created. PID: " + std::to_string(pi.dwProcessId));
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            DebugLog("Falling back to ShellExecute");
            // Fallback to ShellExecute with no args
            HINSTANCE result = ShellExecuteA(nullptr, "open", injectorPath.c_str(), nullptr, "C:\\Windows\\Temp", SW_SHOWDEFAULT);
            DebugLog("ShellExecute result: " + std::to_string((int)(intptr_t)result));
        }
        DebugLog("=== SUCKMYDICKOSU finished ===");
    }
    else
    {
        DebugLog("ERROR: Download failed - injector: " + std::to_string(injectorProgress.load()) + " dll: " + std::to_string(dllProgress.load()));
        downloadError = true;
    }

    isDownloading = false;
}

// Cleanup cache files on exit
void CleanupCache()
{
    for (const auto& file : cacheFiles)
    {
        DeleteFileA(file.c_str());
    }
    cacheFiles.clear();
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        CleanupCache(); // Delete downloaded files on exit
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}