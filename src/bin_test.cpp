#include "dummy_gp.cpp"
#include <chrono>

using namespace std;

int main(int argc, char** argv){
    dataBase_ptr db = DataBase_init(argv[1]);
    
    FM_Die::dummy_gp(db);

    chrono::system_clock::time_point start = chrono::system_clock::now();

    FM_Die::bin_FM(db, 30, 30, 30);

    chrono::system_clock::time_point end = chrono::system_clock::now();
    chrono::milliseconds time = chrono::duration_cast<chrono::milliseconds>(end-start);
    cout << "time: " << time.count() << "ms" << endl;
    return 0;
}