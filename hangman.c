/*
 * hangman.c — Hangman game logic
 *
 * This is Rahim's original game logic (same word list, same 6 starting
 * attempts, same letter-matching rules), split into two commands instead
 * of one interactive loop, because the server spawns this program fresh
 * for every request rather than keeping one long-running process:
 *
 *   ./hangman start
 *       Picks a random word and prints a fresh blank pattern for it.
 *
 *   ./hangman guess <word> <maskedSoFar> <attemptsLeft> <letter>
 *       Applies one guessed letter to the given state and prints the
 *       updated pattern, remaining attempts, and game status.
 *
 * The actual word only ever lives on the server side (see server.js) —
 * this program just answers "given this word and this guess, what
 * happens next", the same way the original loop body did.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define WORD_COUNT 5
#define WORD_LEN 20
#define START_ATTEMPTS 6

/* Same word list as the original terminal game */
const char *WORDS[WORD_COUNT] = {
    "programming", "mongodb", "computer", "keyboard", "console"
};

/* Picks a random word (rand() % 5, same as the original) and prints a
   fresh all-blank guessed pattern for it. */
void cmd_start(void) {
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    const char *word = WORDS[rand() % WORD_COUNT];
    int len = strlen(word);

    char masked[WORD_LEN];
    for (int i = 0; i < len; i++) masked[i] = '_';
    masked[len] = '\0';

    printf("WORD:%s\n", word);
    printf("MASKED:%s\n", masked);
    printf("ATTEMPTS:%d\n", START_ATTEMPTS);
    printf("STATUS:playing\n");
}

/* Applies one guessed letter using the exact same matching loop as the
   original: reveal every matching position, and only lose an attempt if
   nothing at all was found. */
void cmd_guess(const char *word, const char *maskedIn, int attempts, char guess) {
    int len = strlen(word);
    char masked[WORD_LEN];
    strncpy(masked, maskedIn, WORD_LEN - 1);
    masked[WORD_LEN - 1] = '\0';

    int found = 0;
    for (int i = 0; i < len; i++) {
        if (word[i] == guess && masked[i] == '_') {
            masked[i] = guess;
            found = 1;
        }
    }

    if (!found) {
        attempts--;
    }

    const char *status = "playing";
    if (strcmp(masked, word) == 0) {
        status = "won";
    } else if (attempts <= 0) {
        status = "lost";
    }

    printf("WORD:%s\n", word);
    printf("MASKED:%s\n", masked);
    printf("ATTEMPTS:%d\n", attempts);
    printf("FOUND:%s\n", found ? "yes" : "no");
    printf("STATUS:%s\n", status);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage:\n  %s start\n  %s guess <word> <maskedSoFar> <attemptsLeft> <letter>\n",
                argv[0], argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "start") == 0) {
        cmd_start();
        return 0;
    }

    if (strcmp(argv[1], "guess") == 0) {
        if (argc != 6) {
            fprintf(stderr, "Usage: %s guess <word> <maskedSoFar> <attemptsLeft> <letter>\n", argv[0]);
            return 1;
        }
        const char *word = argv[2];
        const char *maskedIn = argv[3];
        int attempts = atoi(argv[4]);
        char guess = argv[5][0];
        cmd_guess(word, maskedIn, attempts, guess);
        return 0;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return 1;
}
