# Very-Human-RPG
Owen Kolbert: Character and card code
Benjamin Rutledge: Sprites and Image rendering
Ethan Molina: Text, Colors, extra credit

1. Vectors are used to store groups of cards in both the pool of possible cards and the decks for the player and enemies. "cardClass.hpp"
2. Algebra is used to calculate how damage should be done when the target has defence that negates some damage. "cardClass.hpp"
3. The sprites are outputted to the screen and the player uses inputs to move the character "main.cpp"
4. Branches are used to check if the player is playing the card or an enemy is. "cardClass.hpp"
5. Loops are used to initially add cards from the pool and move them into the player's deck. "cardClass.hpp"
6. Functions are used for most repeated actions, like playing cards and drawing cards to the deck. "cardClass.hpp"

Structs are used for both constructing cards and actors (players and enemies). Players and enemies are similar enough that it makes more sense to make them the same struct. "cardClass.hpp"
Cards have a name, integer damage value, integer plus defence value, and a status effect that uses and enum class 'status.' "cardClass.hpp"
Actors have a name, a vector of cards as their hand, integer health that defaults to 10, a float for attack buffs, an integer for defence, a status that uses the same status enum as the cards, and a bool that is true if the actor is the player. "cardClass.hpp"