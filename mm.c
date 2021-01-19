// Solve every possible mastermind game
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define PEGS 4
#define COLORS 8

typedef struct { unsigned char right, wrong, peg[PEGS]; } pegs;

// Score guess against target, setting guess's right and wrong
static void score(pegs * const target, pegs *guess)
{
    unsigned char gc[COLORS] = {0}, tc[COLORS] = {0};

    guess->right = 0;
    guess->wrong = 0;

    for (int i = 0; i < PEGS; i++)
    {
        // Remember right peg in right place
        guess->right += target->peg[i] == guess->peg[i];

        // Count target and guess colors
        tc[target->peg[i]]++;
        gc[guess->peg[i]]++;
    }

    // Right color in the wrong place is the accumulation of lesser of each
    // counted color ...
    for (int i = 0; i < COLORS; i++)
        guess->wrong += tc[i] < gc[i] ? tc[i] : gc[i];

    // ... minus the right ones
    guess->wrong -= guess->right;
}

// Advance pegs to next position, return true if wrapped
static int inc(pegs *p)
{
    for (int i = 0; i < PEGS; i++)
    {
        if (++p->peg[i] < COLORS) return 0;
        p->peg[i] = 0;
    }
    return 1;
}

// Solve for goal and return total guesses
static int solve(pegs *goal)
{
    pegs prior[COLORS*PEGS];    // prior guesses
    int guesses=0;              // number of guesses

    // show goal
    printf("Goal:");
    for (int i = 0; i < PEGS; i++) printf(" %d", goal->peg[i]);
    printf("\n");

    // Start with different colors
    pegs guess;
    for (int i = 0; i < PEGS; i++) guess.peg[i] = i % COLORS;

    while(1)
    {
        score(goal, &guess);        // score the guess against goal
        prior[guesses++] = guess;   // and remember it

        // show guess
        printf("     ");
        for (int i = 0; i < PEGS; i++) printf(" %d", guess.peg[i]);
        printf(", %d right and %d wrong\n", guess.right, guess.wrong);

        if (guess.right == PEGS) break; // done!

        // advance to next guess
        next:
        inc(&guess);
        for (int i = 0; i < guesses; i++)
        {
            // A valid guess has the same score against all prior guesses, as
            // those guesses had against the goal
            score(&prior[i], &guess);
            if (guess.right != prior[i].right || guess.wrong != prior[i].wrong) goto next;
        }
    }

    return guesses;
}

int main(void)
{
    int games = 0, total = 0, most = 0;       // stats
    pegs goal = {0};                          // iterate all possible goals
    do
    {
        int guesses = solve(&goal);
        if (guesses > most) most = guesses;
        total += guesses;
        games++;
        printf("Game %d solved in %d guesses, %d most, %d total, %f average\n", games, guesses, most, total, (float)total/games);
    } while (!inc(&goal));

    return 0;
}
