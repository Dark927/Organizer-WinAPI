//// MainWindow.cpp
//#include "MainWindow.h"
//#include "Utils.h"
//
//MainWindow::MainWindow(HINSTANCE hInstance)
//    : hInst(hInstance), hWnd(NULL), hDateTimePicker(NULL) {
//}
//
//MainWindow::~MainWindow() {
//    // Cleanup resources if needed
//}
//
//BOOL MainWindow::Init(int nCmdShow) {
//    WNDCLASSEX wcex;
//    wcex.cbSize = sizeof(WNDCLASSEX);
//    wcex.style = CS_HREDRAW | CS_VREDRAW;
//    wcex.lpfnWndProc = (WNDPROC)WndProc;
//    wcex.cbClsExtra = 0;
//    wcex.cbWndExtra = 0;
//    wcex.hInstance = hInst;
//    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
//    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
//    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    wcex.lpszMenuName = NULL;
//    wcex.lpszClassName = "MainWindowClass";
//    wcex.hIconSm = NULL;
//
//    if (!RegisterClassEx(&wcex)) {
//        return FALSE;
//    }
//
//    hWnd = CreateWindow("MainWindowClass", "Event Scheduler", WS_OVERLAPPEDWINDOW,
//        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
//        NULL, NULL, hInst, this); // Pass this to associate with the window object
//
//    if (!hWnd) {
//        return FALSE;
//    }
//
//    ShowWindow(hWnd, nCmdShow);
//    UpdateWindow(hWnd);
//    return TRUE;
//}
//
//void MainWindow::Run() {
//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0)) {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);
//    }
//}
//
//LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
//    MainWindow* pThis;
//
//    // Associate the window with the MainWindow object
//    if (message == WM_CREATE) {
//        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
//        pThis = (MainWindow*)pCreate->lpCreateParams;
//        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
//    }
//    else {
//        pThis = (MainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
//    }
//
//    if (pThis) {
//        switch (message) {
//        case WM_COMMAND:
//            pThis->OnCommand(wParam);
//            break;
//        case WM_DESTROY:
//            pThis->OnDestroy();
//            break;
//        case WM_CREATE:
//            pThis->OnCreate();
//            break;
//        case WM_NOTIFY:
//            if (lParam) {
//                // Check if the calendar (SysDateTimePick32) control has changed its value
//                LPNMHDR pnmhdr = (LPNMHDR)lParam;
//                if (pnmhdr->code == DTN_DATETIMECHANGE) {
//                    pThis->OnDateChanged();
//                }
//            }
//            break;
//        default:
//            return DefWindowProc(hWnd, message, wParam, lParam);
//        }
//    }
//    return 0;
//}
//
//void MainWindow::OnCreate() {
//    // Create the DateTime Picker control (calendar)
//    hDateTimePicker = CreateWindowEx(0, DATETIMEPICK_CLASS, NULL,
//        WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT,
//        50, 50, 200, 25, hWnd, (HMENU)1, hInst, NULL);
//    if (hDateTimePicker == NULL) {
//        MessageBox(hWnd, "Failed to create DateTime Picker control.", "Error", MB_OK);
//    }
//
//    // Example: Create an "Add Event" button
//    CreateWindow("BUTTON", "Add Event", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
//        50, 100, 100, 30, hWnd, (HMENU)2, hInst, NULL);
//}
//
//void MainWindow::OnCommand(WPARAM wParam) {
//    int wmId = LOWORD(wParam);
//    switch (wmId) {
//    case 2: // Add Event Button
//        AddEvent();
//        break;
//    default:
//        break;
//    }
//}
//
//void MainWindow::OnDestroy() {
//    PostQuitMessage(0);
//}
//
//void MainWindow::AddEvent() {
//    // Show the date selected from the calendar control
//    SYSTEMTIME st;
//    GetSystemTime(&st); // Default to current date if nothing is selected yet
//    if (hDateTimePicker) {
//        SendMessage(hDateTimePicker, DTM_GETSYSTEMTIME, 0, (LPARAM)&st);
//    }
//
//    // Create a new event with the selected date
//    Event newEvent("Sample Event", "This is a sample event.", "", Utils::getCurrentTime(), st, false, 0);
//    eventManager.addEvent(newEvent);
//}
//
//void MainWindow::OnDateChanged() {
//    SYSTEMTIME selectedDate;
//    SendMessage(hDateTimePicker, DTM_GETSYSTEMTIME, 0, (LPARAM)&selectedDate);
//
//    // Now you can use the selected date (selectedDate) for further processing
//    printf("Date selected: %02d/%02d/%d\n", selectedDate.wDay, selectedDate.wMonth, selectedDate.wYear);
//}
//
//HWND MainWindow::GetWindowHandle() const {
//    return hWnd;
//}
