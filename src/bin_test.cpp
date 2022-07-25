#include "dummy_gp.cpp"
#include <chrono>

using namespace std;

void dummy_gp(dataBase_ptr db){
    random_device rd;
    mt19937 gen(rd());

    uniform_real_distribution<> dis(0.0, 1.0);

    double x_length = (double)db->dieDB->upperRightX - (double)db->dieDB->lowerLeftX;
    double y_length = (double)db->dieDB->upperRightY - (double)db->dieDB->lowerLeftY;

    for(int i=0; i<db->instanceDB->numInsts; ++i){
        instance_ptr inst = db->instanceDB->inst_array[i];
        inst->center.x = x_length * dis(gen);
        inst->center.y = y_length * dis(gen);
        place_instance_in_die(db, 0, inst);
    }

}

int main(int argc, char** argv){
    dataBase_ptr db = DataBase_init(argv[1]);

    dummy_gp(db);

    chrono::system_clock::time_point start = chrono::system_clock::now();

    FM_Die::bin_FM(db, 30, 30, atoi(argv[2]));

    chrono::system_clock::time_point end = chrono::system_clock::now();
    chrono::milliseconds time = chrono::duration_cast<chrono::milliseconds>(end-start);
    cout << "time: " << time.count() << "ms" << endl;
    return 0;
}