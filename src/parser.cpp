#include "parser.h"
#include "global.h"
#include "out_utils.h"

vector<long int> label_history;
long int lab_ctr = 0;
long int if_ctr = 0;

bool isfunc = false;

void type(vector<string>& command){
    for(int i = 0; i < command.size(); i++){
        if(command[i].length() >= 2 && command[i][1] == ':'){
            if(command[i][0] == 'b'){
                command[i] = "byte " + command[i].substr(2);
            } else if(command[i][0] == 'w'){
                command[i] = "word " + command[i].substr(2);
            } else if(command[i][0] == 'd'){
                command[i] = "dword " + command[i].substr(2);
            } else if(command[i][0] == 'q'){
                command[i] = "qword " + command[i].substr(2);
            }
        }
    }
}

void chkcom(){    
    size_t first = line.find_first_not_of(" \t\r\n");
    if (first == string::npos) return;
    line = line.substr(first);

    vector<string> command;
    vector<string> arg;
    int ccom = 0;
    int carg = 0;
    bool inbkt = false;
    bool cmf = false;

    if (line[0] == '`') {
        outtext(line.substr(1, line.length() - 2));
        return;
    }

    for (int i = 0; i < line.length(); i++) {
        if(line[i] == ';') break;
        if (ccom >= command.size()) command.push_back("");
        if (line[i] == '(') {
            inbkt = true;
            continue;
        }
        if (line[i] == ')') {
            inbkt = false;
            continue;
        }

        if (inbkt) {
            if (carg >= arg.size()) arg.push_back("");
            if (line[i] == ',') {
                if (!arg[carg].empty()) carg++;
            } else if (line[i] != ' ' && line[i] != '\t') {
                arg[carg] += line[i];
            }
        } else {
            if (line[i] == ' ' || line[i] == '\t') {
                if (!command[ccom].empty()) ccom++;
            } else {
                command[ccom] += line[i];
            }
        }
    }

    for (auto& a : arg) {
        size_t start = a.find_first_not_of(" \t");
        if (start != string::npos) a = a.substr(start);
    }
    string aft = line.substr(line.find('=') + 1);
    int aftpos = line.find('=');
    if(command.size() >= 1){
        type(command);
        
        if(command[0] == "#mode64"){
            mode64 = true; mode32 = false; mode16 = false;
        } else if(command[0] == "#mode32"){
            mode64 = false; mode32 = true; mode16 = false;
        } else if(command[0] == "#mode16"){
            mode64 = false; mode32 = false; mode16 = true;
        }
        else if(command[0] == "<<<") {
            if(arg[0] == "all") {outtext("pusha"); return;};
            if(command.size() > 1) outtext("push " + command[1]);
            type(arg);
            for(int i = 0; i < arg.size(); i++) if(!arg[i].empty()) outtext("push " + arg[i]);
        }
        else if(command[0] == ">>>") {
            if(arg[0] == "all") {outtext("popa"); return;};
            if(command.size() > 1) outtext("pop " + command[1]);
            type(arg);
            for(int i = 0; i < arg.size(); i++) if(!arg[i].empty()) outtext("pop " + arg[i]);
        }
        else if(command.size() >= 3 && command[1] == "=") outtext("mov " + command[0] + ", " + command[2]); 
        else if(command.size() >= 3 && command[1] == "+=") outtext("add " + command[0] + ", " + command[2]); 
        else if(command.size() >= 3 && command[1] == "-=") outtext("sub " + command[0] + ", " + command[2]);
        else if(command[0] == "syscall"){
            if(!arg.empty()) outtext("mov rax, " + arg[0]);
            outtext("syscall");
        } 
        else if(command[0] == "if" && arg.size() >= 3){
            type(arg);
            outtext("cmp " + arg[0] + ", " + arg[2]);
            if(arg[1] == "=="){
                label_history.push_back(lab_ctr);
                outtext("jne .LBL_" + to_string(lab_ctr));
                lab_ctr++;
            } else if(arg[1] == "!="){
                label_history.push_back(lab_ctr);
                outtext("je .LBL_" + to_string(lab_ctr));
                lab_ctr++;
            } else if(arg[1] == "<"){
                label_history.push_back(lab_ctr);
                outtext("jge .LBL_" + to_string(lab_ctr));
                lab_ctr++;
            } else if(arg[1] == "<="){
                label_history.push_back(lab_ctr);
                outtext("jg .LBL_" + to_string(lab_ctr));
                lab_ctr++;
            } else if(arg[1] == ">"){
                label_history.push_back(lab_ctr);
                outtext("jle .LBL_" + to_string(lab_ctr));
                lab_ctr++;
            } else if(arg[1] == ">="){
                label_history.push_back(lab_ctr);
                outtext("jl .LBL_" + to_string(lab_ctr));
                lab_ctr++;
            }
        }
        else if(command[0] == "end."){
            if(!label_history.empty()){
                int last_label = label_history.back();
                label_history.pop_back();
                outtext(".LBL_" + to_string(last_label) + ":");
            }
            else if(isfunc){
                if(mode64) {outtext("pop rbp");}
                else if(mode32) {outtext("pop ebp");}
                else if(mode16) {outtext("pop bp");}
                outtext("ret");
                isfunc = false;
            }
        }
        else if(command[0] == "func"){
            outtext(command[1] + ":");
            if(mode64) {outtext("push rbp"); outtext("mov rbp, rsp");}
            else if(mode32) {outtext("push ebp"); outtext("mov ebp, esp");}
            else if(mode16) {outtext("push bp"); outtext("mov bp, sp");}
            isfunc = true;
        }
        else if(command[0] == "label"){
            outtext(command[1] + ":");
        }
        else if(command[0] == "goto"){
            if(arg.size() >= 2)
                outtext(arg[1] + " " + arg[0]);
            else if(arg.size() >= 1)
                outtext("jmp " + arg[0]);
        }

        else if(command[0] == "byte" || command[0] == "char") outtext(command[1] + " db " + aft);
        else if(command[0] == "short") outtext(command[1] + " dw " + aft);
        else if(command[0] == "int") outtext(command[1] + " dd " + aft);
        else if(command[0] == "bigint") outtext(command[1] + " dq " + aft);
        else if(command[0] == "const") outtext(command[1] + " equ " + aft);


        else if(command[0][0] == '@'){
            if(arg.size() >= 2) outtext(command[0].substr(1) + " " + arg[0] + ", " + arg[1]);
            else if(arg.size() >= 1) outtext(command[0].substr(1) + " " + arg[0]);
            else outtext(command[0].substr(1));
        }
        else {
            if(command[0].back() == ':'){
                outtext(command[0]);
            } else {
                cmf = true;
            }
        }

        if(cmf){
            type(arg);
            for(int i = 0; i < arg.size(); i++) if(!arg[i].empty()) outtext("push " + arg[i]);
            outtext("call " + command[0]);
            if(!arg.empty()){
                int wordSize = mode64 ? 8 : (mode32 ? 4 : 2);
                string reg = mode64 ? "rsp" : (mode32 ? "esp" : "sp");
                outtext("add " + reg + ", " + to_string(arg.size() * wordSize));
            }
        }
    }
}