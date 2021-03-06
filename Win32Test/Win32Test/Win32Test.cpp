// Win32Test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Test.h"

using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace winrt::Windows::UI::Composition::Desktop;
using namespace winrt::Windows::UI::Composition::Interactions;
using namespace winrt::Windows::UI::Input;
using namespace winrt::Windows::System;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND g_hwnd;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


void InitComposition();
void PrepareVisuals();
void AddVisual(VisualCollection const& visuals, float x, float y);
void ConfigureTracker();

DispatcherQueueController g_controller(nullptr);
Compositor g_compositor(nullptr);
DesktopWindowTarget g_target(nullptr);
SpriteVisual g_viewportVisual(nullptr);
ContainerVisual g_contentVisual(nullptr);
InteractionTracker g_tracker(nullptr);
VisualInteractionSource vis(nullptr);

void InitComposition()
{
	DispatcherQueueOptions dqOptions = { 0 };
	dqOptions.dwSize = sizeof(dqOptions);
	dqOptions.apartmentType = DQTAT_COM_ASTA;
	dqOptions.threadType = DQTYPE_THREAD_CURRENT;
	
	DispatcherQueueController controller(nullptr);

	::CreateDispatcherQueueController(dqOptions, reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(winrt::put_abi(g_controller)));
	
	g_compositor = Compositor();

	auto interop = g_compositor.as<ABI::Windows::UI::Composition::Desktop::ICompositorDesktopInterop>();

	winrt::check_hresult(interop->CreateDesktopWindowTarget(g_hwnd, FALSE, reinterpret_cast<ABI::Windows::UI::Composition::Desktop::IDesktopWindowTarget**>(winrt::put_abi(g_target))));
}

void PrepareVisuals()

{
	auto root = g_compositor.CreateSpriteVisual();
	root.RelativeSizeAdjustment({ 1000, 1000});
	root.Brush(g_compositor.CreateColorBrush({ 0xFF, 0xEF, 0xE4 , 0xB0 }));
	g_target.Root(root);
	g_viewportVisual = g_compositor.CreateSpriteVisual();
	g_viewportVisual.RelativeSizeAdjustment({ 1.0f, 1.0f });
	g_viewportVisual.Brush(g_compositor.CreateColorBrush({ 0xAA, 0xAA, 0xAA, 0xAA }));
	g_contentVisual = g_compositor.CreateContainerVisual();
	g_contentVisual.Size({ 5000, 5000 });
	
	auto visuals = root.Children();
	visuals.InsertAtTop(g_contentVisual);
	visuals.InsertAtTop(g_viewportVisual);
	visuals = g_contentVisual.Children();

	AddVisual(visuals, 100.0f, 100.0f);
	AddVisual(visuals, 220.0f, 100.0f);
	AddVisual(visuals, 100.0f, 220.0f);
	AddVisual(visuals, 220.0f, 220.0f);

}


void AddVisual(VisualCollection const& visuals, float x, float y)
{
	auto visual = g_compositor.CreateSpriteVisual();
	static Color colors[] =
	{
		{ 0xDC, 0x5B, 0x9B, 0xD5 },
		{ 0xDC, 0xFF, 0xC0, 0x00 },
		{ 0xDC, 0xED, 0x7D, 0x31 },
		{ 0xDC, 0x70, 0xAD, 0x47 },
	};

	static unsigned last = 0;
	unsigned const next = ++last % _countof(colors);
	visual.Brush(g_compositor.CreateColorBrush(colors[next]));
	visual.Size(
		{
			100.0f,
			100.0f
		});

	visual.Offset(
		{
			x,
			y,
			0.0f,
		});
	visuals.InsertAtTop(visual);
}
void ConfigureTracker() 
{
	g_tracker = InteractionTracker::Create(g_compositor);

	vis = VisualInteractionSource::Create(g_viewportVisual);
	vis.PositionXSourceMode(InteractionSourceMode::EnabledWithInertia);
	vis.PositionYSourceMode(InteractionSourceMode::EnabledWithInertia);
	vis.ScaleSourceMode(InteractionSourceMode::EnabledWithInertia);
	vis.ManipulationRedirectionMode(VisualInteractionSourceRedirectionMode::CapableTouchpadAndPointerWheel);

	g_tracker.MaxPosition({ (g_contentVisual.Size().x - g_viewportVisual.Size().x), (g_contentVisual.Size().y - g_viewportVisual.Size().y), 0.0f });
	g_tracker.MinScale(0.5f);
	g_tracker.MaxScale(4.0f);

	g_tracker.InteractionSources().Add(vis);

	auto positionExpression = g_compositor.CreateExpressionAnimation(L"-tracker.Position");
	positionExpression.SetReferenceParameter(L"tracker", g_tracker);
	g_contentVisual.StartAnimation(L"Offset", positionExpression);

	auto scaleExpression = g_compositor.CreateExpressionAnimation(L"Vector3(tracker.Scale, tracker.Scale, 1.0)");
	scaleExpression.SetReferenceParameter(L"tracker", g_tracker);
	g_contentVisual.StartAnimation(L"Scale", scaleExpression);

	//g_tracker.TryUpdatePosition({ 800, 800, 0 });
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	winrt::init_apartment(winrt::apartment_type::single_threaded);


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32TEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	InitComposition();
	PrepareVisuals();
	ConfigureTracker();

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32TEST));

    MSG msg;

    // Main message loop:
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32TEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WIN32TEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);


	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32TEST));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32TEST);
	wcex.lpszClassName = L"Parent dude";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));




    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hwndParent = CreateWindowW(L"Parent dude", szTitle, WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   ShowWindow(hwndParent, nCmdShow);
   UpdateWindow(hwndParent);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_CHILD,
      0, 0, 800, 800, hwndParent, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hwnd = hWnd;
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	case WM_LBUTTONDOWN:
	{
		g_tracker.TryUpdatePositionBy({ -10, -10, 0 });
		break;
	}


	case WM_POINTERDOWN:
	{
		PointerPoint pp = PointerPoint::GetCurrentPoint(GET_POINTERID_WPARAM(wParam));
		vis.TryRedirectForManipulation(pp);
		break;
	}

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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
