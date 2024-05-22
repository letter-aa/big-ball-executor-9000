// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <map>
#include <string>
#include <tchar.h>

#define printStructureArgs int, const char*

template<typename T, typename...a> using function = T(*)(a...);
typedef void(__fastcall* printStructure) (printStructureArgs);
typedef int(__cdecl* any)();

typedef struct charExist {
    bool char1 = false;
    bool char2 = false;
};

//uintptr_t base = 0x0;
uintptr_t printAddress = 0x0; // address here!

//long long info = 1i64;
long long notification = 4i64;

printStructure printType = (printStructure)printAddress;

#define print(string) printType(0, string)
#define info(string) printType(1, string)
#define warn(string) printType(2, string)
#define error(string) printType(3, string)

std::map<std::string, function<void, any>> functions = {
    { "print", [](any arg) { print((const char*)arg); } },
    { "info", [](any arg) { info((const char*)arg); } },
    { "warn", [](any arg) { warn((const char*)arg); } },
    { "error", [](any arg) { error((const char*)arg); } },
    { "close", [](any arg) { FreeConsole(); } } // RESERVED
};

bool isnum(std::string string) {
    return string.find_first_not_of("0123456789") == std::string::npos;
}

charExist areCharsAtEnds(std::string input, char char1, char char2) {
    charExist exists;

    if (input.size() > 0) {
        exists.char1 = input.at(0) == char1;
        exists.char2 = input.at(input.size() - 1) == char2;
    }

    return exists;
}

charExist areCharsAtEnds(std::string input, char char1) {
    charExist exists;

    if (input.size() > 0) {
        exists.char1 = input.at(0) == char1;
        exists.char2 = input.at(input.size() - 1) == char1;
    }

    return exists;
}

const char* run(std::string input) {
    const char* whitespace = " \t";

    int index = 0;

    if (input.find_first_not_of(whitespace) != std::string::npos) {
        input = input.substr(input.find_first_not_of(whitespace), input.find_last_not_of(whitespace) + 1); // trim input string

        for (auto const& element : functions) { // iterate through functions
            size_t funcNameLen = element.first.size();

            if (input.substr(index, funcNameLen) == element.first) { // check if user is calling a function
                //std::cout << "funcName: " << element.first << "\n"; // func name print
                index += funcNameLen;

                index += input.substr(index).find_first_not_of(whitespace);

                charExist dblQtCheck = areCharsAtEnds(input.substr(index), '"'); // check if both double quotes exist
                charExist sinQtCheck = areCharsAtEnds(input.substr(index), '\''); // check if both single quotes exist
                charExist prtQtCheck = areCharsAtEnds(input.substr(index), '(', ')'); // check if both parentheses exist

                if (!(prtQtCheck.char1 && prtQtCheck.char2) && !(sinQtCheck.char1 && sinQtCheck.char2) && !(dblQtCheck.char1 && dblQtCheck.char2)) { // check parentheses and quotes
                    return (prtQtCheck.char1 && !prtQtCheck.char2) ? "Expected ')' (to close '(' at line 1), got <eof>" : "Incomplete statement: expected assignment or a function call";
                }
                else {
                    const char* result;

                    if (element.second == functions["close"]) {
                        result = "close";
                    }
                    else {
                        result = NULL;
                    }

                    index++;

                    std::string argument1 = input.substr(index, input.size() - 1 - index);

                    if ((prtQtCheck.char1 && prtQtCheck.char2) && argument1.size() > 0) { // check if there are parentheses
                        //check if argument 1 is a string or not

                        if (argument1.size() > 1) {
                            charExist dblQtCheck = areCharsAtEnds(argument1, '"');
                            charExist sinQtCheck = areCharsAtEnds(argument1, '\'');


                            if (!(sinQtCheck.char1 && sinQtCheck.char2) && !(dblQtCheck.char1 && dblQtCheck.char2)) { // check quotes
                                if (isnum(argument1)) {
                                    element.second((any)std::stoi(argument1));
                                }
                                else if (argument1 == "true" || argument1 == "false") {
                                    element.second((any)argument1.c_str());
                                }
                                else {
                                    return "Incomplete statement: expected assignment or a function call";
                                }
                            }
                            else {
                                element.second((any)argument1.substr(1, argument1.size() - 2).c_str());
                            }
                        }
                        else if (isnum(argument1)) {
                            element.second((any)std::stoi(argument1));
                        }
                        else {
                            return "Incomplete statement: expected assignment or a function call";
                        }
                    }
                    else {
                        element.second((any)argument1.c_str());
                    }
                    return result;
                }
            }
        }
        return "Incomplete statement: expected assignment or a function call\n";
    }
    return NULL;
}

int thread() {
    while (true) {
        std::string input;

        std::getline(std::cin, input);

        const char* returnValue = run(input);

        if (returnValue) {
            if (returnValue == "close") {
                break;
            }
            else {
                std::cerr << "Error: " << returnValue;
            }
        }

        std::cout << "\n";
    }

    return 0;
}

int createConsole() {
    FILE* newIO;

    AllocConsole();
    freopen_s(&newIO, "CONOUT$", "w", stdout);
    freopen_s(&newIO, "CONOUT$", "w", stderr);
    freopen_s(&newIO, "CONIN$", "r", stdin);


    // not copied from stack overflow chat i swear on my left sock (i have no socks on)
    #define newWIO(n) CreateFile(_T(n), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL) // new wide output

    SetStdHandle(STD_OUTPUT_HANDLE, newWIO("CONOUT$"));
    SetStdHandle(STD_ERROR_HANDLE, newWIO("CONOUT$"));
    SetStdHandle(STD_INPUT_HANDLE, newWIO("CONIN$"));

    #undef newWIO

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&thread, NULL, 0, NULL);

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        createConsole();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
