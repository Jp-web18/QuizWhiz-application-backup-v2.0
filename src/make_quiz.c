#include "config.h"

void encrypt_decrypt_xor(char *data, size_t length, char key) {
    for (size_t i = 0; i < length; i++) {
        data[i] ^= key;
    }
}

void make_quiz_menu() {
    if (!login_make_quiz()) return;

    int choice;
    char input[10];

    while (1) {
        system(CLEAR);
        printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
        printf("%s                     MAKE A QUIZ MENU\n", COLOR_YELLOW);
        printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
        printf("\n");
        printf("%s[1] %sMake a Quiz%s\t\t\t%s[3] %sChange PIN%s\n\n",
            COLOR_YELLOW, COLOR_WHITE, COLOR_RESET,
            COLOR_YELLOW, COLOR_WHITE, COLOR_RESET);
        printf("%s[2] %sEdit Existing Quizzes%s\t%s[4] Back to Main Menu%s\n",
            COLOR_YELLOW, COLOR_WHITE, COLOR_RESET,
            COLOR_LIGHT_PURPLE, COLOR_RESET);
        printf("\n");
        printf("%s----------------------------------------------------------%s\n\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
        printf("%sEnter your choice:%s\t\t", COLOR_CYAN, COLOR_RESET);

        if (fgets(input, sizeof(input), stdin)) {
            choice = atoi(input);
            printf("%s", COLOR_RESET);

            switch (choice) {
                case 1: create_new_quiz(); break;
                case 2: edit_existing_quiz(); break;
                case 3: change_pin(); break;
                case 4: return;
                default:
                    printf("%sInvalid choice. Try again.%s\n", COLOR_RED, COLOR_RESET);
                    sleep(1);
            }
        }
    }
}

/* LOGIN FOR MAKING A TEST */
int login_make_quiz() {
    int attempts = 0;
    char entered_pin[MAX_PIN_LENGTH], stored_pin_encrypted[MAX_PIN_LENGTH], stored_pin[MAX_PIN_LENGTH];
    char encryption_key = 'K';
    FILE *pin_file = fopen(PIN_FILE, "rb");

    if (pin_file) {
        size_t bytes_read = fread(stored_pin_encrypted, 1, MAX_PIN_LENGTH - 1, pin_file);
        fclose(pin_file);

        if (bytes_read > 0) {
            stored_pin_encrypted[bytes_read] = '\0';
            strcpy(stored_pin, stored_pin_encrypted);
            encrypt_decrypt_xor(stored_pin, bytes_read, encryption_key);
        } else {
            printf("%sPIN file is corrupted.%s\n", COLOR_RED, COLOR_RESET);
            return 0;
        }
    } else {
        // Create default PIN if file doesn't exist
        char default_pin[] = "1234";
        encrypt_decrypt_xor(default_pin, strlen(default_pin), encryption_key);
        FILE *new_file = fopen(PIN_FILE, "wb");
        if (!new_file || fwrite(default_pin, 1, strlen(default_pin), new_file) < strlen(default_pin)) {
            perror("Failed to create default PIN file");
            return 0;
        }
        fclose(new_file);
#ifndef _WIN32
        set_file_permissions(PIN_FILE, 0600);
#endif
        strcpy(stored_pin, "1234");
    }

    while (attempts < MAX_LOGIN_ATTEMPTS) {
        system(CLEAR);
        // printf("%sMake a quiz%s\n\n%s", COLOR_YELLOW, COLOR_RESET, COLOR_BLUE);
        printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
        printf("%s                      AUTHENTICATION\n", COLOR_YELLOW);
        printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
        printf("\n");
        printf("%sEnter PIN:%s\t\t",COLOR_CYAN, COLOR_WHITE);
        if (fgets(entered_pin, MAX_PIN_LENGTH, stdin)) {
            printf("%s", COLOR_RESET);
            entered_pin[strcspn(entered_pin, "\n")] = '\0';

            if (strcmp(entered_pin, stored_pin) == 0) {
                printf("%sLogin successful.%s\n", COLOR_GREEN, COLOR_RESET);
                sleep(1);
                return 1;
            } else {
                // printf("Incorrect PIN. Attempts remaining: %d\n", MAX_LOGIN_ATTEMPTS - ++attempts);
                printf("%sIncorrect PIN. Attempts remaining: %d%s\n", COLOR_RED, MAX_LOGIN_ATTEMPTS - ++attempts, COLOR_RESET);
                sleep(1);
            }
        } else {
            printf("%sInvalid input.%s\n", COLOR_RED, COLOR_RESET);
            sleep(1);
            attempts++;
        }
    }

    printf("%sToo many failed login attempts. Returning to main menu.%s\n", COLOR_RED, COLOR_RESET);
    sleep(2);
    return 0;
}


/* CHANGE PIN */
void change_pin() {
    char new_pin[MAX_PIN_LENGTH];
    char encryption_key = 'K';

    system(CLEAR);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("%s                       CHANGE PIN\n", COLOR_YELLOW);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("\n");
    printf("%sEnter new PIN:\t\t%s", COLOR_CYAN, COLOR_RESET);
    if (fgets(new_pin, MAX_PIN_LENGTH, stdin)) {
        printf("%s", COLOR_RESET);
        new_pin[strcspn(new_pin, "\n")] = '\0';
        encrypt_decrypt_xor(new_pin, strlen(new_pin), encryption_key);

        FILE *pin_file = fopen(PIN_FILE, "wb");
        if (pin_file && fwrite(new_pin, 1, strlen(new_pin), pin_file)) {
            fclose(pin_file);
#ifndef _WIN32
            set_file_permissions(PIN_FILE, 0600);
            printf("%sPIN changed and encrypted.%s\n", COLOR_GREEN, COLOR_RESET);
#else
            printf("%sPIN changed successfully.%s\n", COLOR_GREEN, COLOR_RESET);
#endif
        } else {
            printf("%sFailed to write PIN file.%s\n", COLOR_RED, COLOR_RESET);
        }
    } else {
        printf("%sInvalid input.%s\n", COLOR_RED, COLOR_RESET);
    }
    sleep(1);
}


/* CREATE NEW QUIZ */
void create_new_quiz() {
    char filename[100], input[100];
    int num_items, duration;
    char encryption_key = 'Q';
    
    system(CLEAR);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("%s                     MAKE A QUIZ\n", COLOR_YELLOW);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("\n");
    printf("%sEnter quiz file name:%s\t\t", COLOR_CYAN, COLOR_RESET);
    if (!fgets(filename, sizeof(filename), stdin)) return;
    filename[strcspn(filename, "\n")] = '\0';

    if (access("quizzes", F_OK) == -1) {
#ifdef _WIN32
        if (mkdir("quizzes") != 0)
#else
        if (mkdir("quizzes", 0777) != 0)
#endif
        {
            perror("Failed to create quizzes directory");
            return;
        }
    }

    char full_filename[128];
    snprintf(full_filename, sizeof(full_filename), "quizzes/%s.quiz", filename);

    FILE *quiz_file = fopen(full_filename, "r");
    if (quiz_file) {
        fclose(quiz_file);
        printf("%sQuiz file exists.\n%sOverwrite? (%sy%s/%sn%s):%s\t\t", COLOR_RED, COLOR_CYAN, COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_CYAN, COLOR_RESET);
        char confirm[3];
        if (fgets(confirm, sizeof(confirm), stdin) && (confirm[0] != 'y' && confirm[0] != 'Y')) {
            printf("%sQuiz not saved.%s\n", COLOR_RED, COLOR_RESET);
            return;
        }
    }

    printf("%sEnter time duration (minutes):%s\t", COLOR_CYAN, COLOR_RESET);
    if (!fgets(input, sizeof(input), stdin)) return;
    duration = atoi(input);

    printf("%sEnter number of items:%s\t\t", COLOR_CYAN, COLOR_RESET);
    if (!fgets(input, sizeof(input), stdin)) return;
    num_items = atoi(input);
    printf("\n");

    if (num_items <= 0) {
        printf("%sInvalid number of items.%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    char correct_answers[num_items]; // Array to store all answers
    for (int i = 0; i < num_items; i++) {
        printf("%sEnter answer for item %d:%s\t", COLOR_CYAN, i + 1, COLOR_RESET);
        if (!fgets(input, sizeof(input), stdin)) return;
        input[strcspn(input, "\n")] = '\0'; // Remove newline
        if (strlen(input) != 1) {
            printf("%sInvalid input. Each answer must be a single character.%s\n", COLOR_RED, COLOR_RESET);
            i--; // Retry the current item
            continue;
        }
        correct_answers[i] = input[0]; // Store the single character answer
    }

    FILE *fp = fopen(full_filename, "wb");
    if (!fp) {
        perror("Failed to create quiz file");
        return;
    }

    // Prepare data for encryption
    char quiz_data[256];
    snprintf(quiz_data, sizeof(quiz_data), "%d\n%d\n", duration, num_items);
    size_t offset = strlen(quiz_data);
    memcpy(quiz_data + offset, correct_answers, num_items);
    offset += num_items;

    // Encrypt data
    encrypt_decrypt_xor(quiz_data, offset, encryption_key);

    // Write encrypted data to file
    if (fwrite(quiz_data, 1, offset, fp) != offset) {
        perror("Failed to write quiz file");
        fclose(fp);
        return;
    }
    fclose(fp);

    printf("\n%sSave quiz?\n%s[1] Yes\n%s[2] No\n%sEnter your Choice:%s\t\t", COLOR_YELLOW, COLOR_GREEN, COLOR_RED, COLOR_CYAN, COLOR_RESET);
    char confirm_str[3];
    if (fgets(confirm_str, sizeof(confirm_str), stdin) && atoi(confirm_str) == 1) {
        printf("%sQuiz saved successfully.%s\n", COLOR_GREEN, COLOR_RESET);
    } else {
        remove(full_filename);
        printf("%sQuiz discarded.%s\n", COLOR_RED, COLOR_RESET);
    }
    sleep(1);
}


/* EDIT EXISTING QUIZ */
void edit_existing_quiz() {
    DIR *dir;
    struct dirent *entry;
    char filenames[100][128];
    int file_count = 0, choice;
    char encryption_key = 'Q';

    system(CLEAR);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("%s                 EDIT EXISTING QUIZZES\n", COLOR_YELLOW);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);

    // Open quizzes directory
    if ((dir = opendir("quizzes")) == NULL) {
        perror("Failed to open quizzes directory");
        sleep(1);
        return;
    }

    printf("\n%sAvailable quizzes:%s\n", COLOR_YELLOW, COLOR_RESET);
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".quiz")) {
            snprintf(filenames[file_count], sizeof(filenames[file_count]), "quizzes/%s", entry->d_name);
            printf("[%d] %s\n", file_count + 1, entry->d_name);
            file_count++;
        }
    }
    closedir(dir);

    if (file_count == 0) {
        printf("%sNo quizzes available to edit/delete.%s\n", COLOR_RED, COLOR_RESET);
        sleep(1);
        return;
    }

    printf("\n%sEnter quiz number to edit/delete (0 to cancel):%s\t", COLOR_CYAN, COLOR_RESET);
    if (scanf("%d", &choice) != 1 || choice < 0 || choice > file_count) {
        printf("%sInvalid choice.%s\n", COLOR_RED, COLOR_RESET);
        sleep(1);
        return;
    }
    getchar(); // Consume leftover newline

    if (choice == 0) {
        printf("%sOperation canceled.%s\n", COLOR_RED, COLOR_RESET);
        sleep(1);
        return;
    }

    char *selected_file = filenames[choice - 1];
    char input[100], correct_answers[100];
    int duration, num_items;

    system(CLEAR);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("%s                 EDIT EXISTING QUIZZES\n", COLOR_YELLOW);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);

    printf("\n%sWhat would you like to do?%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("[1] Edit quiz\n");
    printf("[2] Delete quiz\n");
    printf("%sEnter your choice:%s\t\t", COLOR_CYAN, COLOR_RESET);
    if (fgets(input, sizeof(input), stdin) && input[0] == '2') {
        if (remove(selected_file) == 0) {
            printf("%sQuiz deleted successfully.%s\n", COLOR_GREEN, COLOR_RESET);
        } else {
            perror("Failed to delete quiz");
        }
        sleep(1);
        return;
    }

    FILE *quiz_file = fopen(selected_file, "rb");
    if (!quiz_file) {
        printf("%sQuiz file not found.%s\n", COLOR_RED, COLOR_RESET);
        sleep(1);
        return;
    }

    // Read and decrypt quiz data
    char quiz_data[256];
    size_t bytes_read = fread(quiz_data, 1, sizeof(quiz_data), quiz_file);
    fclose(quiz_file);

    if (bytes_read == 0) {
        printf("%sFailed to read quiz file or file is corrupted.%s\n", COLOR_RED, COLOR_RESET);
        sleep(1);
        return;
    }

    encrypt_decrypt_xor(quiz_data, bytes_read, encryption_key);

    // Parse decrypted data
    sscanf(quiz_data, "%d\n%d\n", &duration, &num_items);
    size_t offset = strlen(quiz_data) - num_items;
    memcpy(correct_answers, quiz_data + offset, num_items);

    system(CLEAR);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("%s                 EDIT EXISTING QUIZZES\n", COLOR_YELLOW);
    printf("%s==========================================================%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);

    printf("\n%sCurrent duration: \t %s%d minutes%s\n", COLOR_YELLOW, COLOR_WHITE, duration, COLOR_RESET);
    printf("%sCurrent number of items: %s%d%s\n", COLOR_YELLOW, COLOR_WHITE, num_items, COLOR_RESET);
    printf("%sCurrent correct answers:%s\n", COLOR_YELLOW, COLOR_RESET);
    for (int i = 0; i < num_items; i++) {
        //printf("Item %d:\t%c\n", i + 1, correct_answers[i]);
        printf("%sItem %d:\t%s%c%s\n", COLOR_YELLOW, i + 1, COLOR_WHITE, correct_answers[i], COLOR_RESET);

    }

    printf("\n%s----------------------------------------------------------%s\n\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
    printf("%sEnter new duration in minutes (Press Enter to keep current):%s\t", COLOR_CYAN, COLOR_RESET);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        duration = atoi(input);
    }

    printf("%sEnter new number of items (Press Enter to keep current):%s\t", COLOR_CYAN, COLOR_RESET);
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        num_items = atoi(input);
    }

    while (1) {
        printf("%sEnter new correct answers (Press Enter to keep current):%s\n", COLOR_CYAN, COLOR_RESET);
        for (int i = 0; i < num_items; i++) {
            //printf("Item %d (current: %c):\t", i + 1, correct_answers[i]);
            printf("%sItem %d (current: %c):%s\t", COLOR_CYAN, i + 1, correct_answers[i], COLOR_RESET);
            if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
                input[strcspn(input, "\n")] = '\0';
                if (strlen(input) != 1) {
                    printf("%sInvalid input. Each answer must be a single character.%s\n", COLOR_RED, COLOR_RESET);
                    i--; // Retry the current item
                    continue;
                }
                correct_answers[i] = input[0];
            }
        }

        printf("\n%sAre you finished editing the quiz?%s\n", COLOR_YELLOW, COLOR_RESET);
        printf("%s[1] Yes, save.%s\n", COLOR_GREEN, COLOR_RESET);
        printf("%s[2] No, continue.%s\n", COLOR_RED, COLOR_RESET);
        printf("%s[3] Cancel, back to Make Quiz Menu.%s\n", COLOR_LIGHT_PURPLE, COLOR_RESET);
        printf("%sEnter your choice:%s\t", COLOR_CYAN, COLOR_RESET);
        if (fgets(input, sizeof(input), stdin)) {
            int option = atoi(input);
            if (option == 1) {
                break;
            } else if (option == 3) {
                return;
            }
        }
        printf("\n");
    }
    correct_answers[num_items] = '\0'; // Null-terminate the string

    // Save updated quiz
    quiz_file = fopen(selected_file, "wb");
    if (!quiz_file) {
        perror("Failed to save quiz file\n");
        return;
    }

    // Prepare data for encryption
    snprintf(quiz_data, sizeof(quiz_data), "%d\n%d\n", duration, num_items);
    offset = strlen(quiz_data);
    memcpy(quiz_data + offset, correct_answers, num_items);
    offset += num_items;

    // Encrypt data
    encrypt_decrypt_xor(quiz_data, offset, encryption_key);

    // Write encrypted data to file
    if (fwrite(quiz_data, 1, offset, quiz_file) != offset) {
        perror("Failed to write quiz file");
    }
    fclose(quiz_file);

    printf("%sQuiz updated successfully.%s\n", COLOR_GREEN, COLOR_RESET);
    sleep(1);
}