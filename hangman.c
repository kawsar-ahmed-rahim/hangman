#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int main() {
    char words[5][20] = {"programming", "mongodb", "computer", "keyboard", "console"};
    srand(time(0));
    char word[20];
    strcpy(word, words[rand() % 5]);

    int len = strlen(word);
    char guessed[20];
    for (int i = 0; i < len; i++) guessed[i] = '_';
    guessed[len] = '\0';

    int attempts = 6;
    char guess;
    int found;

    printf("=== HANGMAN GAME ===\n");
    printf("=== GUESS A WORD RELATED TO IT SECTOR ===\n");

    while (attempts > 0 && strcmp(guessed, word) != 0) {
        printf("\nWord: %s\n", guessed);
        printf("Attempts left: %d\n", attempts);
        printf("Enter a letter: ");
        scanf(" %c", &guess);

        found = 0;
        for (int i = 0; i < len; i++) {
            if (word[i] == guess && guessed[i] == '_') {
                guessed[i] = guess;
                found = 1;
            }
        }

        if (!found) {
            printf("Wrong guess!\n");
            attempts--;
        } else {
            printf("Good guess!\n");
        }
    }

    if (strcmp(guessed, word) == 0)
        printf("\nCongratulations! You guessed the word: %s\n", word);
    else
        printf("\nGame Over! The word was: %s\n", word);

    return 0;
}
