
#undef UNICODE // VS Code keeps complaining about UNICODE being defined even though it's not


#include "freedialog.h"

#include <iostream>

namespace dlg = freedialog;

int main(){

    std::cout << "launch\n";
    do {
        dlg::setTextColor(RGB(0, 120, 0));
        dlg::setFontColor(RGB(0, 10, 200));

        
        std::string val = dlg::getBasicInput("Enter text into the test text-field. This is a test. "
                                        "You don't need to enter any information because this is just a test.",
            "Input");

        std::string val2;
        dlg::getInputRequired(val2, "Enter Data:");

        std::string val3;
        dlg::getInputRequired(val3, "Enter Password:", "", 400, 200, '*');

        std::string ip;
        if(dlg::getIPAddress(ip, "Enter IP Address:", "")){
            std::cout << "IP: " << ip << "\n";
        }

        std::cout << "User entered: " << val << "\n";
        std::cout << "User entered: " << val2 << "\n";
        std::cout << "User entered: " << val3 << "\n";
        
        std::string lpath, spath;
        {
            dlg::LoadDialog load(lpath, "");
            dlg::SaveDialog save(spath, "");
            std::cout << "User load: " << lpath << "\n";
            std::cout << "User save: " << spath << "\n";
        }

        dlg::MessageDialog msg("Message Box", "Title", dlg::Information);
        
        {
            dlg::QuestionDialog question("Question And Selection", "Title", dlg::Warning | dlg::YesNoCancel);
            switch(question.result()){
                case dlg::Yes:
                    std::cout << "user seleted yes\n";
                    break;
                case dlg::No:
                    std::cout << "user selected no\n";
                    break;
                case dlg::Cancel:
                    std::cout << "user canceled\n";
                    break;
                default:
                    std::cout << "unknown selection!\n";
                    break;
            }
        }

        {
            dlg::QuestionDialog retry("Would you like to run the test again?", "Retry", dlg::Question | dlg::YesNo);
            if(retry.result() == dlg::No) break; // break from application loop
        }


    } while(1);

    return 0;
}