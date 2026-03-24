#include "parser.h"
#include "global.h"

int main(int argc, char* argv[]){
    string inputFile = argv[1];
    string outputFile;

    if (argc >= 4 && string(argv[2]) == "-o") outputFile = argv[3];

    in.open(inputFile);
    out.open(outputFile);

    if (in.is_open() && out.is_open())
    {
        while (getline(in, line))
        {
            if(line.empty()) {
                continue;
            }
            chkcom();
        }
        
    }
    

    in.close();
    out.close();
}