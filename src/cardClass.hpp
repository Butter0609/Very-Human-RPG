#include <vector>
#include <cmath>
#include <string>
#include <set>
#include <algorithm>
#include <cstdlib> 
#include <ctime>   
using namespace std;

const int MAX_HAND_SIZE = 5;

enum class status{
    NONE,
    POISON,
};

struct card{
    string name{};
    int damage{};
    int plusDef{};
    status effect = status::NONE;
};

struct actor {
    string name{};
    vector<card> hand{};
    int health = 10;
    float atkbuffs = 1;
    int def{};
    status status = status::NONE;
    bool player = false;
};

void playerDeath(actor player){

}

void enemyDeath(actor enemy){

}

//Card should be entered as user.hand.at('n')
void playCard(actor &user, actor &opponent,card &c){
    opponent.health -= clamp(((int)round(c.damage * user.atkbuffs) - opponent.def),0,100);
    opponent.def -= (int)round(c.damage * user.atkbuffs);
    opponent.status = c.effect;
    if (opponent.def < 0) opponent.def = 0;
    user.def += c.plusDef;
    if(user.status == status::POISON){
        user.health--;
    }
    if(user.player && user.health <= 0){
        playerDeath(user);
    } else if(opponent.player && opponent.health <= 0){
        playerDeath(opponent);
    } else if(!user.player && user.health <= 0){
        enemyDeath(user);
    } else if(!opponent.player && opponent.health <= 0){
        enemyDeath(opponent);
    }
    if(user.player){
        swap(c,user.hand.at((MAX_HAND_SIZE - 1)));
        user.hand.pop_back();
    }

}

void drawFirstHand(actor &player, vector<card> pool){
    for(int i = 0; i < MAX_HAND_SIZE; i++){
        int randNum = rand() % (pool.size() - 1);
        player.hand.at(i) = pool.at(randNum);
        swap(pool.at(randNum),pool.at(pool.size() - 1));
        pool.pop_back();
    }


}

card poison{"poison",0,0,status::POISON};
card attack{"attack",1,0};
card defend{"defend",0,2};

int main2() {
    string name = "Man";
    vector<card> pool = {poison,poison,attack,attack,attack,attack,attack,defend,defend,defend};
    vector<card> hand{MAX_HAND_SIZE};
    actor player = {name,hand,10,1,0,status::NONE,true};
    return 0;
}