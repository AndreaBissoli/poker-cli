#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include <sys/random.h>

#define DECK_SIZE 52
#define VALUES_NUM 13
#define SUITS_NUM 4
#define ARRAY_LEN(a) sizeof a / sizeof a[0]

struct Card {
    int value;
    int suit;
};
typedef struct Card Card;

struct Hand {
    Card first;
    Card second;
};
typedef struct Hand Hand;

char values[VALUES_NUM] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K'};
char suits[SUITS_NUM] = {'H', 'D', 'S', 'C'};

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
        printf("Player %d's hand: %c %c --- %c %c \n\n", i+1, values[players[i].first.value - 1], suits[players[i].first.suit - 1], values[players[i].second.value - 1], suits[players[i].second.suit - 1]);
    }
    printf("\nCards on the table: \n");
    for(int i=0;i<5;i++)
    {
        printf("%c %c    ", values[table[i].value - 1], suits[table[i].suit - 1]);
    }
    printf("\n");
}


void play_game(Card *deck, int player_num)
{
    bool deck_state[DECK_SIZE];
    Hand players[player_num];
    for(int i=0;i<player_num;i++)
    {
        while(true)
        {
            int first = gen_rand_num(0, DECK_SIZE - 1);
            int second = gen_rand_num(0, DECK_SIZE - 1);
            if(deck_state[first] || deck_state[second] || first == second) continue;
            players[i].first = deck[first];
            players[i].second = deck[second];
            deck_state[first] = true;
            deck_state[second] = true;
            break;
        }
    }
    Card table[5];
    for(int i=0;i<5;i++)
    {
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
}



int main()
{
    Card deck[DECK_SIZE];
    for(int i=1;i<=VALUES_NUM;i++)
    {
        for(int j=1;j<=SUITS_NUM;j++)
        {
            deck[VALUES_NUM * (j - 1) + i - 1].value=i;
            deck[VALUES_NUM * (j - 1) + i - 1].suit=j;
        }
    }

    play_game(deck, 10);

    return 0;
}
