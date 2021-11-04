
#undef UNICODE // VS Code keeps complaining about UNICODE being defined even though it's not


#include "freedialog.h"

#include <iostream>

namespace dlg = freedialog;

int main(){

    std::cout << "launch\n";
    dlg::setTextColor(RGB(0, 120, 0));
    dlg::setFontColor(RGB(0, 10, 200));

    
    std::string val = dlg::getBasicInput("Enter text into the test text-field. This is a test. "
                                    "You don't need to enter any information because this is just a test.",
        "Input");

    std::string val2;
    dlg::getInputRequired(val2, "Enter Data:");

    std::string ip;
    if(dlg::getIPAddress(ip, "Enter IP Address:", "")){
        std::cout << "IP: " << ip << "\n";
    }

    std::cout << "User entered: " << val << "\n";
    std::cout << "User entered: " << val2 << "\n";


    return 0;
}