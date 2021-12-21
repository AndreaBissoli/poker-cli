#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include <sys/random.h>

#define DECK_SIZE 52
#define VALUES_NUM 13
#define SUITS_NUM 4
#define TABLE_SIZE 5
#define ARRAY_LEN(a) sizeof a / sizeof a[0]

// 0 is nothing, 1 is pair, 2 is two-pair, 3 is three of a kind, 4 is four of a kind, 5 is full house
#define NOTHING 0
#define PAIR 1
#define TWO_PAIR 2
#define THREE_KIND 3
#define FULL_HOUSE 4 // 4 should be four of a kind, but this makes it very easy to check what hand is better: higher value, better hand
#define FOUR_KIND 5  // it also makes sense to have these defines to make it clear and easily change values when i'll add flushes and straights

struct Card {
    int value, suit;
};
typedef struct Card Card;

struct Hand {
    Card cardA, cardB;
};

typedef struct Hand Hand;

struct IntPair {
    int first, second;
};
typedef struct IntPair IntPair;

// Arrays to map values and suits to numbers
char values[VALUES_NUM] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};
char suits[SUITS_NUM] = {'H', 'D', 'S', 'C'};

// Copy-pasted, it works on linux so whatever
// No way to generate actually good random numbers more than once a cardB in C without libraries, wow
void init_rand_num(){
    unsigned int buf = 0;
    getrandom(&buf, 4, 1);
    srand(buf);
}

int gen_rand_num(int lower, int upper)
{
    init_rand_num();
    return (rand() % (upper - lower + 1)) +lower;
}


void dump_state(Hand *players, Card*table, int player_num)
{
    for(int i=0;i<player_num;i++)
    {
        printf("Player %d's hand: %c %c --- %c %c \n\n", i+1, values[players[i].cardA.value - 1], suits[players[i].cardA.suit - 1], values[players[i].cardB.value - 1], suits[players[i].cardB.suit - 1]);
    }
    printf("\nCards on the table: \n");
    for(int i=0;i<TABLE_SIZE;i++)
    {
        printf("%c %c    ", values[table[i].value - 1], suits[table[i].suit - 1]);
    }
    printf("\n");
}

void update_state(IntPair *state, int value)
{
    int free = 0;
    for(int i=0;i<7;i++)
    {
        if(state[i].second == - 1)
        {
            free = i;
            break; // No dynamic arrays in C, gotta know if it exists or not
        }
        if(state[i].second == value)
        {
            state[i].first++;
            return;
        }
    }
    // TODO: implement error handling if state full and different value (impossible in a game)
    state[free].second = value;
    state[free].first++;
}


void check_winner(Hand *players, Card *table, int player_num)
{

   IntPair state[7];
   int result_code = 0;
   for(int i=0;i<player_num;i++)
   {
       // State array to store how many cards of each value
       // state[i].first is the amount of cards with value state[i].second
       // state[i].second == -1 means uninitialized value
       // the array is 7 long because there can be at most 7 different values, 2 in hand and 5 on the table.
       for(int i=0;i<7;i++)
       {
           state[i].first = 0;
           state[i].second = -1;
       }
       result_code = NOTHING;
       update_state(state, players[i].cardA.value);
       update_state(state, players[i].cardB.value);
       for(int j=0;j<TABLE_SIZE;j++)
       {
           update_state(state, table[j].value);
       }
       for(int i=0;i<7;i++)
       {
           if(state[i].first == 2)
           {
               if(result_code == NOTHING) result_code = PAIR; // If I had nothing and see a pair, I have a pair
               else if(result_code == PAIR) result_code = TWO_PAIR; // If I had a pair and see a pair, I have a two-pair
               else if(result_code == THREE_KIND) result_code = FULL_HOUSE; // If I had a three of a kind and see a pair, I have a full house
               // If I have a full house or a four of a kind, another pair is irrelevant (except for values).
               // TODO: implement higher or lower pairs (state[i].second is the value)
               // probably sort the state by value, pass through and save whichever value the best pairs or threes have, with an array of booleans to mark which values are
               // involved with the pairs. Then, save (5-how many cards are marked) values, to complete the 5 card hand.
           }
           if(state[i].first == 3)
           {
               if(result_code == NOTHING) result_code = THREE_KIND;
               else if(result_code == PAIR) result_code = FULL_HOUSE;
               else if(result_code == TWO_PAIR) result_code = FULL_HOUSE;
           }
           if(state[i].first == 4) result_code = FOUR_KIND;
       }
       printf("Result code: %d", result_code);
   }
   /*for(int i=0;i<7;i++)
   {
       printf("state[%d].first is %d and second is %d\n", i, state[i].first, state[i].second);
   }*/
}



void deal_cards(Card *deck, int player_num)
{
    bool deck_state[DECK_SIZE];
    Hand players[player_num];
    for(int i=0;i<player_num;i++)
    {
        //While loop to ensure choosing a card not chosen already
        while(true)
        {
            int cardA = gen_rand_num(0, DECK_SIZE - 1);
            int cardB = gen_rand_num(0, DECK_SIZE - 1);
            if(deck_state[cardA] || deck_state[cardB] || cardA == cardB) continue;
            players[i].cardA = deck[cardA];
            players[i].cardB = deck[cardB];
            deck_state[cardA] = true;
            deck_state[cardB] = true;
            break;
        }
    }
    Card table[TABLE_SIZE];
    for(int i=0;i<TABLE_SIZE;i++)
    {
        //While loop to ensure choosing a card not chosen already
        while(true)
        {
            int draw = gen_rand_num(0, DECK_SIZE - 1);
            if(deck_state[draw]) continue;
            table[i] = deck[draw];
            deck_state[draw] = true;
            deck_state[draw] = true;
            break;
        }
    }
    dump_state(players, table, player_num);
    check_winner(players, table, player_num);
}

int main()
{
    Card deck[DECK_SIZE];
    // Initialize the deck
    for(int i=1;i<=VALUES_NUM;i++)
    {
        for(int j=1;j<=SUITS_NUM;j++)
        {
            deck[VALUES_NUM * (j - 1) + i - 1].value=i;
            deck[VALUES_NUM * (j - 1) + i - 1].suit=j;
        }
    }

    deal_cards(deck, 2);
    return 0;
}
