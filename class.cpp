#include <vector>
#include <cmath>
#include <string>
#include <set>
using namespace std;

const int decksize = 5;


struct card{
    string name{};
    int damage{};
    int plusDef{};

};

struct actor {
    string name{};
    vector<card> deck{};
    float atkbuffs{};
    int def{};
};

int main(){
    string name = "Man";
    card basic1 = {"attack",1,0};
    card basic2 = {"defence",0,1};
    vector<card> deck{decksize};
    actor player = {name,deck,0,0};

}