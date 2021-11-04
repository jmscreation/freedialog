#include "freedialog.h"

#include <iostream>
#include <functional>
#include <cstring>

#define ID_BUTTON_OKAY 0x8F00
#define ID_BUTTON_CANCEL 0x8F01
#define ID_LABEL_TEXT 0x8F02
#define ID_INPUT_FIELD_1 0x8F03
#define ID_INPUT_FIELD_2 0x8F04

namespace freedialog {

    const HCURSOR Form::DefaultCursor = LoadCursor(NULL, IDC_ARROW); // default cursor

    Form::Form(): CLASS_NAME("_INPUTWINDOW"), hInstance(GetModuleHandle(NULL)),
                winClass({}), hWnd(NULL), wincolorbrush(NULL), allowNoValue(false), userAbort(true) {
        // Register the window class.

        winClass.lpfnWndProc   = winHandle;
        winClass.hInstance     = hInstance;
        winClass.lpszClassName = CLASS_NAME;

        textcolor = RGB(0,0,0);
        wincolor = RGB(230,230,230);
        fontcolor = RGB(0,0,0);

        RegisterClass(&winClass);
    }

    Form::~Form() {
        if(wincolorbrush != NULL){
            DeleteObject(wincolorbrush);
            wincolorbrush = NULL;
        }
    }

    void Form::SetWindowColor(COLORREF col) {
        wincolor = col;
    }
    void Form::SetLabelColor(COLORREF col) {
        textcolor = col;
    }
    void Form::SetFontColor(COLORREF col) {
        fontcolor = col;
    }

    void Form::FormCreate(const char* label, const char* title, const char* defvalue, int width, int height, bool titlebar, char pwd, bool cancelbtn, bool isIP) {
        if(hWnd != NULL) return;

        allowNoValue = titlebar || cancelbtn;
        userAbort = true;
        
        int btnWidth = 96, btnHeight = 32, btnMargin = 32,
            labelMargin = 8,
            txHeight = 20, txMargin = 32;


        results.resize(2, "");
        wincolorbrush = CreateSolidBrush(wincolor);

        CreateDlg(title, width, height, titlebar);

        RECT frame;
        AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
        int tbHeight = std::abs(frame.top);

        if(cancelbtn){
            AddButton(ID_BUTTON_OKAY, "OK", width / 4 - btnWidth / 2, height - 32 - tbHeight - btnHeight, btnWidth, btnHeight, true); // okay button
            AddButton(ID_BUTTON_CANCEL, "CANCEL", 3 * width / 4 - btnWidth / 2, height - 32 - tbHeight - btnHeight, btnWidth, btnHeight); // cancel button
        } else {
            AddButton(ID_BUTTON_OKAY, "OK",
                width / 2 - btnWidth / 2, height - btnMargin - tbHeight - btnHeight,
                btnWidth, btnHeight, true); // okay button
        }

        int labelHeight = height - labelMargin - txMargin - txHeight - btnMargin - tbHeight - btnHeight,
            top = labelHeight + labelMargin,
            bottom = height - btnMargin - tbHeight - btnHeight,
        
            center = top + (bottom - top) / 2 - txHeight / 2;

        AddTextLabel(ID_LABEL_TEXT, label, 8, labelMargin, width - 16, labelHeight); // input label

        if(!isIP) {
            AddTextField(ID_INPUT_FIELD_1, defvalue, 16, center, width - 32, txHeight, pwd); // first input field
        } else {
            int ipFieldWidth = std::min(width - 8, 120);
            AddIPField(ID_INPUT_FIELD_1, defvalue, width / 2 - ipFieldWidth / 2, center, ipFieldWidth, txHeight);
        }
        SetFocus(GetDlgItem(hWnd, ID_INPUT_FIELD_1)); // first input field is default focused
        SendMessage(GetDlgItem(hWnd, ID_INPUT_FIELD_1), EM_SETSEL, 0, -1);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this); // _this reference is passed to window procedure
        HandleMessages();
    }

    void Form::HandleMessages() {
        MSG msg {};
        while(GetMessage(&msg, NULL, 0, 0) > 0){

            if(msg.message == WM_KEYDOWN) {
                KeyDown(msg.wParam);
            }
            if(msg.message == WM_KEYUP) {
                KeyUp(msg.wParam);
            }

            if (!IsWindow(hWnd) || !IsDialogMessage(hWnd, &msg)){
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    void Form::CreateDlg(const char* title, int width, int height, bool hasbar){

        RECT rect;
        GetClientRect(GetDesktopWindow(), &rect);
        rect.left = (rect.right/2) - (width/2);
        rect.top = (rect.bottom/2) - (height/2);

        DWORD flags = WS_VISIBLE | WS_DLGFRAME;

        if(hasbar) flags |= WS_SYSMENU;

        hWnd = CreateWindowEx( WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                            CLASS_NAME, title,
                            flags,
                            rect.left, rect.top, width, height,
                            NULL, NULL,
                            hInstance, (LPVOID)this );
    }

    void Form::AddButton(ULONG64 id, const char* text, int x, int y, int width, int height, bool defaultbtn) {
        CreateWindowEx(0, WC_BUTTON, text,
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RIGHTBUTTON | (defaultbtn ? BS_PUSHLIKE : 0),
                    x, y, width, height,
                    hWnd, (HMENU)id,
                    hInstance, NULL );
    }

    void Form::AddTextField(ULONG64 id, const char* value, int x, int y, int width, int height, char pwd) {
        HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, value,
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                    x, y, width, height,
                    hWnd, (HMENU)id,
                    hInstance, NULL );
        SendMessage(hwnd, EM_SETPASSWORDCHAR, (WPARAM)pwd, 0);
        
    }

    void Form::AddIPField(ULONG64 id, const char* ip, int x, int y, int width, int height) {
        int octet = 0, i = 0, pos = 0, bytes[4] {0, 0, 0, 0};
        {   // calculate ip address from const char*
            char c, buf[4];
            do {
            c = ip[i++];
            if(c == '.' || c == ' ' || c == '-' || c == ',' || c == 0 || pos == 3){
                buf[pos] = 0;
                try {
                    bytes[octet++] = std::stoi(buf);
                } catch( std::invalid_argument& e){
                    break;
                }
                pos = 0;
                continue;
            }
            buf[pos++] = c;
        } while(octet < 4 && c != 0);
        }
        HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_IPADDRESS, "",
                    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                    x, y, width, height,
                    hWnd, (HMENU)id,
                    hInstance, NULL );
        if(octet > 0){
            SendMessage(hwnd, IPM_SETADDRESS, 0, MAKEIPADDRESS(bytes[0], bytes[1], bytes[2], bytes[3]));
        }
    }

    void Form::AddTextLabel(ULONG64 id, const char* text, int x, int y, int width, int height) {
        CreateWindowEx(0,
                    WC_STATIC, text,
                    WS_CHILD | WS_VISIBLE | SS_CENTER,
                    x, y, width, height,
                    hWnd, (HMENU) id,
                    hInstance, NULL);
    }

    void Form::Submit() {
        size_t len = GetWindowTextLength(GetDlgItem(hWnd, ID_INPUT_FIELD_1)) + 1;

        char* buf = new char[len];
        GetDlgItemText(hWnd, ID_INPUT_FIELD_1, buf, len);
        results[0].assign(buf, len);
        delete[] buf;

        userAbort = false;
        CloseWindow();
    }

    void Form::KeyDown(WPARAM key){
        switch(key){
            case VK_RETURN: {
                SendMessage(GetDlgItem(hWnd, ID_BUTTON_OKAY), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
                break;
            }
            case VK_ESCAPE:{
                if(allowNoValue){
                    HWND btn = GetDlgItem(hWnd, ID_BUTTON_CANCEL);
                    if(btn == NULL){
                        CloseWindow();
                    } else {
                        SendMessage(btn, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
                    }
                } else {
                    SendMessage(GetDlgItem(hWnd, ID_BUTTON_OKAY), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(0, 0));
                }
                break;
            }
        }
    }

    void Form::KeyUp(WPARAM key){
        switch(key){
            case VK_RETURN: {
                SendMessage(GetDlgItem(hWnd, ID_BUTTON_OKAY), WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
                break;
            }
            case VK_ESCAPE: {
                if(allowNoValue){
                    HWND btn = GetDlgItem(hWnd, ID_BUTTON_CANCEL);
                    if(btn != NULL){
                        SendMessage(btn, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
                    }
                } else {
                    SendMessage(GetDlgItem(hWnd, ID_BUTTON_OKAY), WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(0, 0));
                }
            }
        }
    }

    void Form::ButtonClick(UINT id, HWND button){

        switch(id){
            case ID_BUTTON_OKAY:{
                Submit();
                break;
            }
            case ID_BUTTON_CANCEL:{
                CloseWindow();
                break;
            }
        }
    }

    void Form::CloseWindow() {
        DestroyWindow(hWnd);
        PostQuitMessage(0);
        hWnd = NULL;
    }

    std::string Form::GetResult(size_t index) {
        if(index > results.size()) return "";
        return results[index];
    }

    Form* Form::getThis(HWND hwnd){
        return (Form*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    LRESULT CALLBACK Form::winHandle(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch(uMsg){
            case WM_CREATE:{
                return true;
            }
            case WM_INITDIALOG:{
                SetForegroundWindow(hwnd);
                return true;
            }
            case WM_SETCURSOR:{
                SetCursor(DefaultCursor);
                break;
            }
            case WM_SETFOCUS:{
                SetFocus(GetDlgItem(hwnd, ID_INPUT_FIELD_1));
                break;
            }
            case WM_CLOSE:{
                Form* _this = getThis(hwnd);
                if(_this == nullptr) break;

                _this->CloseWindow();
                break;
            }
            case WM_PAINT:{
                Form* _this = getThis(hwnd);
                if(_this == nullptr) break;

                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                FillRect(hdc, &ps.rcPaint, _this->wincolorbrush);
                EndPaint(hwnd, &ps);

                return false;
            }
            case WM_CTLCOLOREDIT:{
                Form* _this = getThis(hwnd);
                if(_this == nullptr) break;

                HDC hdc = (HDC) wParam;

                SetTextColor(hdc, _this->fontcolor);

                return (INT_PTR)_this->wincolorbrush;
            }
            case WM_CTLCOLORSTATIC:{
                Form* _this = getThis(hwnd);
                if(_this == nullptr) break;

                HDC hdc = (HDC) wParam;
                
                SetTextColor(hdc, _this->textcolor);
                SetBkColor(hdc, _this->wincolor);

                return (INT_PTR)_this->wincolorbrush;
            }
            case WM_COMMAND:{
                Form* _this = getThis(hwnd);
                if(_this == nullptr) break;

                UINT id = LOWORD(wParam);
                int code = HIWORD(wParam);
                HWND btn = (HWND)lParam;
                
                if(code == BN_CLICKED){
                    _this->ButtonClick(id, btn);
                }

                return true;
            }
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }


    Form Dialogue;

    void setWindowColor(COLORREF col) {
        Dialogue.SetWindowColor(col);
    }

    void setTextColor(COLORREF col) {
        Dialogue.SetLabelColor(col);
    }

    void setFontColor(COLORREF col) {
        Dialogue.SetFontColor(col);
    }

    bool getInput(std::string& rvalue, const std::string& title, const std::string& caption, const std::string& def, int width, int height, char passChar, bool cancelbtn) {
        Dialogue.FormCreate(title.data(), caption.data(), def.data(), width, height, true, passChar, cancelbtn);
        if(Dialogue.GetUserAbort()) return false;
        rvalue = Dialogue.GetResult();

        return true;
    }

    bool getInputRequired(std::string& rvalue, const std::string& title, const std::string& def, int width, int height, char passChar, bool cancelbtn) {
        Dialogue.FormCreate(title.data(), "", def.data(), width, height, false, passChar, cancelbtn);
        if(Dialogue.GetUserAbort()) return false;
        rvalue = Dialogue.GetResult();
        
        return true;
    }

    bool getIPAddress(std::string& rvalue, const std::string& title, const std::string& caption, const std::string& def, int width, int height, bool required, bool cancelbtn) {
        Dialogue.FormCreate(title.data(), caption.data(), def.data(), width, height, true, 0, cancelbtn, true);
        if(Dialogue.GetUserAbort()) return false;
        rvalue = Dialogue.GetResult();

        return true;
    }

    std::string getBasicInput(const std::string& title, const std::string& caption, const std::string& def) {
        Dialogue.FormCreate(title.data(), caption.data(), def.data(), 400, 200 + title.size() / 30);

        return Dialogue.GetResult();
    }

    std::string getBasicPassword(const std::string& title, const std::string& caption, const std::string& def, char passChar) {
        Dialogue.FormCreate(title.data(), caption.data(), def.data(), 400, 200 + title.size() / 30, true, passChar);

        return Dialogue.GetResult();
    }
}