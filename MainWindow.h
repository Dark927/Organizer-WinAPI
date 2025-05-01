// MainWindow.h
#pragma once
#include <windows.h>
#include "EventManager.h"
#include "Alarm.h"

class MainWindow {
public:
    MainWindow(HINSTANCE hInstance);
    ~MainWindow();

    // Window message handling
    BOOL Init(int nCmdShow);
    void Run();

    // Event handling functions
    void AddEvent();
    void DeleteEvent();
    void UpdateEvent();
    void DisplayEvents();

    HWND GetWindowHandle() const;

private:
    HINSTANCE hInst;
    HWND hWnd;
    HWND hDateTimePicker;  // Handle for the calendar (SysDateTimePick32)
    EventManager eventManager;
    Alarm alarm;

    // Window procedure
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    void OnCreate();
    void OnCommand(WPARAM wParam);
    void OnDestroy();
};
