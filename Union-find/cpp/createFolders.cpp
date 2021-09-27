#include <sstream>
#include <iostream>

using namespace std;

void createFolders(string method) {
    int n[4] = {20, 30, 40, 50};
    string d[3] = {"ld", "md", "hd"};
    stringstream ss;

    for(int i=0; i<4; i++) {
        for(int j=0; j<3; j++) {
            ss.str("");
            ss << "mkdir -p " << method << "/instances/g1/" << n[i] << "/" << d[j];
            int ignoreReturn = system(ss.str().c_str());
        }
    }

    int n2[6] = {50, 100, 200, 400, 500, 1000};
    int label[6][4] = {{12, 25, 50, 62}, {25, 50, 100, 125}, {50, 100, 200, 250}, {100, 200, 400, 500}, {125, 250, 500, 625}, {250, 500, 1000, 1250}};

    for(int i=0; i<6; i++) {
        for(int j=0; j<3; j++) {
            for(int k=0; k<4; k++) {
                ss.str("");
                ss << "mkdir -p " << method << "/instances/g2/" << n2[i] << "/" << d[j] << "/" << label[i][k];
                int ignoreReturn = system(ss.str().c_str());
            }
        }
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        cout << "Missing folder to store the outputs. Use ./createFolders.out <folder>" << endl;
        return 0;
    }

    createFolders(argv[1]);
}