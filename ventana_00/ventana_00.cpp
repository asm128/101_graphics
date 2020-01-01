#include <Windows.h>

// Callback for window events (required to handle window events such as click or closing the window)
LRESULT WINAPI						WndProc					(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_DESTROY:	// Catch the window's DESTROY message which is posted to the queue when the window is closed.
		PostQuitMessage(0);	// Signal the WM_QUIT message.
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int		WINAPI						WinMain
	( _In_		HINSTANCE					hInstance
	, _In_opt_	HINSTANCE					//hPrevInstance
	, _In_		LPSTR						//lpCmdLine
	, _In_		int							nShowCmd
	) {
	// Specify window class parameters.
	WNDCLASSEX								wndClass				= {sizeof(WNDCLASSEX)};
	const char								className	[]			= "ventana_main";
	wndClass.lpfnWndProc				= WndProc;
	wndClass.lpszClassName				= className;
	wndClass.hInstance					= hInstance;
	RegisterClassEx(&wndClass);	// Registers the class

	// Create the window
	HWND									newWindow				= CreateWindowExA(0, className, "Ventana 0", WS_OVERLAPPEDWINDOW, 100, 100, 640, 480, 0, 0, hInstance, 0);

	ShowWindow(newWindow, nShowCmd);	// The window is hidden by default. Show the window with the SW_SHOW parameter.

	bool									running					= true;	// This variable will signal when we need to exit the main loop.
	while(running) {	// Loop until the window is closed
		MSG										msg						= {};	// Store in this variable the message picked up from the queue.
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {	// Retrieve the first message from the event queue
			TranslateMessage(&msg);
			DispatchMessage(&msg);	// This function causes our WndProc() function to be called with the message as parameter
			if(msg.message == WM_QUIT)	// This message is posted in the message queue by PostQuitMessage(0) called in the window event procedure (WndProc)
				running								= false;
		}
	}

	UnregisterClass(className, hInstance);
	return EXIT_SUCCESS;
}
