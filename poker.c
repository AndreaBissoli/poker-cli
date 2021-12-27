#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "string.h"
#include "errno.h"
#include "limits.h"
#include "time.h"

#define DECK_SIZE 52
#define VALUES_NUM 13
#define SUITS_NUM 4
#define TABLE_SIZE 5
#define STATE_SIZE 7
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
char values[VALUES_NUM] = {'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'}; // Because aces are 13 in value, 2s are 1 in value, 3s are 2 in value etc, function dump_state works correctly to represent cards
char suits[SUITS_NUM] = {'H', 'D', 'S', 'C'};


int gen_rand_num(int lower, int upper)
{
    return (rand() % (upper - lower + 1)) +lower;
}

Card* init_deck()
{
    static Card deck[DECK_SIZE];
    for(int i=1;i<=VALUES_NUM;i++)
    {
        for(int j=1;j<=SUITS_NUM;j++)
        {
            deck[VALUES_NUM * (j - 1) + i - 1].value=i;
            deck[VALUES_NUM * (j - 1) + i - 1].suit=j;
        }
    }
    return deck;
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
    for(int i=0;i<STATE_SIZE;i++)
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


int compare (const void * a, const void * b)
{

  IntPair *pairA = (IntPair *)a;
  IntPair *pairB = (IntPair*)b;

  return ( pairB->second - pairA->second );
}

int compare_winning_hands (const void * a, const void * b)
{

  int *arrayA = (int *)a;
  int *arrayB = (int*)b;

  for(int i=0;i<6;i++)
  {
      if(arrayB[i] > arrayA[i]) return 1;
      else if(arrayB[i] < arrayA[i]) return -1;
  }
  return 0;
}

int* find_best_hand(Hand player, Card *table)
{

    IntPair state[STATE_SIZE];
    // State array to store how many cards of each value
    // state[i].first is the amount of cards with value state[i].second
    // state[i].second == -1 means uninitialized value
    // the array is 7 long because there can be at most 7 different values, 2 in hand and 5 on the table.
    int result_code = NOTHING;
    for(int i=0;i<STATE_SIZE;i++)
    {
        state[i].first = 0;
        state[i].second = -1;
    }
    update_state(state, player.cardA.value);
    update_state(state, player.cardB.value);
    for(int j=0;j<TABLE_SIZE;j++)
    {
        update_state(state, table[j].value);
    }
    qsort(state, STATE_SIZE, sizeof(IntPair), compare);
    int positionA = -1, positionB = -1; // Positions at which I have a pair, a three etc, so that I can know what to look at to decide which values to put in the best hand
    for(int i=0;i<STATE_SIZE;i++)
    {
        if(state[i].first == 2)
        {
            if(result_code == NOTHING) {
                result_code = PAIR; // If I had nothing and see a pair, I have a pair
                positionA = i;
            }
            else if(result_code == PAIR) {
                result_code = TWO_PAIR; // If I had a pair and see a pair, I have a two-pair
                positionB = i;
            }
            else if(result_code == THREE_KIND) {
                result_code = FULL_HOUSE; // If I had a three of a kind and see a pair, I have a full house
                positionB = i;
            }
            // If I have a full house or a four of a kind, another pair is irrelevant (except for values).
            // TODO: implement higher or lower pairs (state[i].second is the value)
            // probably sort the state by value, pass through and save whichever value the best pairs or threes have, with an array of booleans to mark which values are
            // involved with the pairs. Then, save (5-how many cards are marked) values, to complete the 5 card hand.
        }
        else if(state[i].first == 3)
        {
            if(result_code == NOTHING) {
                result_code = THREE_KIND;
                positionA = i;
            }
            else if(result_code == PAIR) {
                result_code = FULL_HOUSE;
                int temp;
                temp = positionA;
                positionA = i;
                positionB = temp;
            }
            else if(result_code == TWO_PAIR) {
                result_code = FULL_HOUSE;
                int temp;
                temp = positionA;
                positionA = i;
                positionB = temp;
            }
            else if(result_code == THREE_KIND) {
                result_code = FULL_HOUSE;
                state[i].first--; // VERY peculiar case, two three of a kinds: I get a full house and decrease the amount of cards in the second three. This should not be a problem, because there are at most 5 cards (3+2) in the winning hand, and makes it easy to look at it later
                positionB = i;
            }
        }
        else if(state[i].first == 4) {
            result_code = FOUR_KIND;
            positionA = i;
            positionB = -1;
        }
    }
    static int final_result[6];
    int pointer = 1;
    final_result[0] = result_code;
    if(positionA != -1)
    {
        for(int i=0;i<state[positionA].first;i++)
        {
            final_result[pointer] = state[positionA].second;
            pointer++;
        }
        state[positionA].first = -1; // Mark this not to count when I will fill the final result with the values not including pairs, threes etc
    }
    if(positionB != -1)
    {
        for(int i=0;i<state[positionB].first;i++)
        {
            final_result[pointer] = state[positionB].second;
            pointer++;
        }
        state[positionA].first = -1; // Mark this not to count when I will fill the final result with the values not including pairs, threes etc
    }
    for(int i=0; i<STATE_SIZE && pointer != 6; i++)
    {
        if(state[i].first != -1)
        {
            final_result[pointer] = state[i].second;
            pointer++;
            state[i].first = -1;
        }
    }
    /*for(int i=0;i<6;i++)
    {
        printf("final_result[%d] is %d \n", i, final_result[i]);
    }*/
    return final_result;
   /*for(int i=0;i<7;i++)
   {
       printf("state[%d].first is %d and second is %d\n", i, state[i].first, state[i].second);
   }*/
}



void play_game(Card *deck, int player_num)
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
    int winning_hands[player_num][6];
    for(int i=0;i<player_num;i++)
    {
        int* player_winning_hand = find_best_hand(players[i], table);
        for(int j=0;j<6;j++)
        {
            winning_hands[i][j] = player_winning_hand[j];
        }
    }
    qsort(winning_hands, player_num, sizeof(int[6]), compare_winning_hands);
    for(int i=0;i<player_num;i++)
    {
        for(int j=0;j<6;j++)
        {
            printf("%d ", winning_hands[i][j]);
        }
        printf("\n");
    }
}

void print_help_message()
{
    printf("Usage: poker -p player_num\n");
    printf("Options: \n-p player_num play a game with player_num players");
}

void print_error_message()
{
    fprintf(stderr, "Usage: poker -p player_num"); // poker [-p player_num | -otheroptions otherarguments]
    fprintf(stderr, "Use 'poker --help for more information' ");
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    if(argc == 1 || argc == 2) print_help_message();
    else if(argc == 3)
    {
        if(strcmp(argv[1], "-p") == 0)
        {
            char* p;
            errno = 0;
            long p_num = strtol(argv[2], &p, 10);
            if(*p == '\0' && errno == 0)
            {
                if(p_num <= 0 || p_num > INT_MAX)
                {
                    fprintf(stderr, "Number out of range");
                    print_error_message();
                    return 1;
                }
                else
                {
                    int player_num = p_num;
                    Card* deck = init_deck();
                    play_game(deck, player_num);
                }
            }
            else
            {
                fprintf(stderr, "Incorrect number format");
                print_error_message();
                return 1;
            }
        }
    }
    return 0;
}
