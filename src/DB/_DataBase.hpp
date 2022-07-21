#ifndef _DB_H_
#define _DB_H_

#include<vector>
#include<string>

using namespace std;

class MasterCell;
class MCPin; //실제 instance에 위치한 pin이 아닌 tech변화 고려를 위해
             //master cell의 pin의 정보를 저장해두는 object
             //얘의 free는 MasterCell객체의 destructor에서 발생
class Instance;
class Net;
class Pin;
class Die; //cell die
class Terminal;
class TDie; //terminal die or termianl grid

//whole data structure for control dynamic allocation
class DataBase
{
    public:
        //map or vector
        vector<MasterCell*> MC_vec;
        vector<Instance*> Inst_vec;
        vector<Net*> Net_vec;
        vector<Pin*> Pin_vec; //pin의 동적할당을 해제해주기 위해 만듬.
                              //동적할당 해제를 위한 다른 방법이 있다면 없애도 상관 없을 것.
        vector<Die*> Die_vec;
        vector<Terminal*> Term_vec;
        TDie* TDie_;

        ~DataBase(); //for free heap memory(dynamic allocation)
};

/*
예상(?)해본 input file -> database 과정입니다.
1. die 및 terminal은 서로 영향 x이므로 file 아무렇게나 집어 넣기
2. mastercell, instance, net, pin, mcpin은 서로 영향을 주고 받음
    1). tech file 통해서 mastercell, mcpin 생성
    2). instanace file 통해서 instance 생성
        - 이때 C1 MC1이라면, C1의 mastercell = MC_vec[0]
        - Instance object내의 pin_vec은 아직 생성 x.
    3). netlist file 통해서 net과 pin 생성
        - pin은 Net, Instance, Database의 Net_vec에 모두 집어 넣어주기
        - pin의 offset은 필요할 경우 MasterCell->MCPin 접근을 통해 update
    
    * net, instance, cell, pin, mcpin, mastercell의 경우
      vector내에서 index가 실제 index-1이 되도록 순서 지켜서 집어넣어주기(?)
*/

class MasterCell
{
    public:
        int index; //MC1에서 MC빼고 1만 사용(?) (MC8일 경우 index = 8)
                   //MC_vec에서 vector element's index + 1 = index
                   //ex) MC_vec[0] = MC1
                   //Instance, Net, Pin도 동일한 index 사용
                   //만약 index 사용이 불편할 경우, hash map을 사용해 name or index를 key로 사용 가능
        
        //assume always number of technology = 2
        //for tech A
        int A_sizeX;
        int A_sizeY;
        int A_area;
        vector<MCPin*> A_MCPin_vec;

        //for tech B
        int B_sizeX;
        int B_sizeY;
        int B_area;
        vector<MCPin*> B_MCPin_vec;

        ~MasterCell(); //for free MCPin_vecs

};

class MCPin
{
    public:
        //MasterCell* mastercell;
        int index; //MC1 P1이면 index=1
        int offsetX;
        int offsetY;
};

class Instance
{
    public:
        int index; //C1 -> index = 1
        MasterCell* master_cell; //from cell list
        bool top; //in top die -> true, bottom die -> false
        bool tech_A; //top die일 때 반드시 tech_A인것은 아니니까 flag추가.

        int centerX;
        int centerY;
        int sizeX;
        int sizeY;
        int area;

        vector<Pin*> Pin_vec; //net list를 직접 갖고 있지 않고, pin을 통해 접근
};

class Net
{
    public:
        int index;

        vector<Pin*> Pin_vec; //cell list를 직접 갖고 있지 않고, pin을 통해 접근
};

class Pin
{
    public:
        int index; //instance에 속해있는 pin 중에서 index
                   //C1/P1이라면 1, C1/P3라면 3.

        Instance* instance;
        Net* net;
        
        int offsetX; //from Instance's center(?)
        int offsetY;
};

class Die
{
    public:
        //current state
        bool top;
        bool tech_A; //if tech A -> True
        int utilization_curr;

        //from input file
        int utilization_ideal;
        int lowerLeftX;
        int lowerLeftY;
        int upperRightX;
        int upperRightY;
        int startX;
        int startY;
        int rowLenght;
        int rowHeight;
        int repeatCount;
};

class Terminal
{
    public:
        int sizeX;
        int sizeY;
        int spacing;
        int centerX;
        int centerY;
};

class TDie
{
    public:
        int utilization_ideal;
        int utilization_curr;
        int lowerLeftX;
        int lowerLeftY;
        int upperRightX;
        int upperRightY;
        int startX;
        int startY;
};

#endif