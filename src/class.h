#include <vector>
#include <cmath>
#include <string>
#include <set>
#include <algorithm>
#include <iostream>
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
    int health = 10;
    float atkbuffs = 1;
    int def{};
};

int playCard(actor &user, actor &enemy,const card &c){
    enemy.health -= clamp(((int)round(c.damage * user.atkbuffs) - enemy.def),0,100);
    enemy.def -= (int)round(c.damage * user.atkbuffs);
    if (enemy.def < 0) enemy.def = 0;
    user.def += c.plusDef;
}

int main2(){
    string name = "Man";
    card basic1 = {"attack",1,0};
    card basic2 = {"defence",0,1};
    vector<card> pool = {};
    vector<card> deck{decksize};
    actor player = {name,deck,1,0};

}