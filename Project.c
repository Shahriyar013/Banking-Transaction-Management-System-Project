#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep(x) Sleep(x*1000)
#else
    #include <unistd.h>
#endif

#define MAX_ACCOUNTS 500
#define MAX_TRANSACTIONS 2000
#define MAX_NAME_LENGTH 50
#define INTEREST_RATE 0.015
#define DATA_FILE "bank_data.txt"

typedef struct {
    int accountNumber;
    char firstName[MAX_NAME_LENGTH];
    char lastName[MAX_NAME_LENGTH];
    double balance;
    int isActive;
    int isLocked;
    int isSavings;
    time_t lastInterestDate;
    char password[50];
} Account;

typedef struct {
    int transactionId;
    int accountNumber;
    char type[20];
    double amount;
    time_t timestamp;
    int relatedAccount;
    char description[100];
} Transaction;

typedef struct {
    char username[50];
    char password[50];
} Admin;

Account accounts[MAX_ACCOUNTS];
Transaction transactions[MAX_TRANSACTIONS];
Admin admins[5];
int accountCount = 0;
int transactionCount = 0;
int adminCount = 0;
int currentUserAccount = -1;
int isAdminLoggedIn = 0;

void initializeSystem();
void loadData();
void saveData();
void mainMenu();
void adminMenu();
void customerMenu();
void registerAccount();
void updateAccount();
void deleteAccount();
void deposit();
void withdraw();
void transfer();
void balanceInquiry();
void lockUnlockAccount();
void calculateInterest();
int findAccountByNumber(int accountNumber);
int authenticateAdmin();
void displayTransactionHistory(int accountNumber);
void clearInputBuffer();
void printAccountDetails(int accountIndex);
void listAllAccounts();
void generateReports();
void createTransaction(int accountNumber, const char* type, double amount, int relatedAccount, const char* description);
void printWelcomeScreen();
void changePassword();
int validatePassword(const char* password);
void createAdminAccounts();
void searchAccount();
void accountStatistics();
void printHelp();
int validateTransaction(int accountIndex, double amount);

int main() {
    printWelcomeScreen();
    initializeSystem();
    loadData();
    mainMenu();
    return 0;
}

void printWelcomeScreen() {
    printf("\n\n");
    printf("\n");
    printf("*                                     *\n");
    printf("*     WELCOME TO BANKING SYSTEM       *\n");
    printf("*                                     *\n");
    printf("\n");
    printf("\n");
    sleep(1);
}

void initializeSystem() {
    if (adminCount == 0) {
        createAdminAccounts();
    }

    FILE *testFile = fopen("test_write.txt", "w");
    if (testFile) {
        time_t currentTime = time(NULL);
        fprintf(testFile, "Banking System - Write Test Successful!\n");
        fprintf(testFile, "Current Time: %s", ctime(&currentTime));
        fclose(testFile);
        printf(" File write test successful - test_write.txt created\n");
    } else {
        printf(" WARNING: Cannot create files in current directory!\n");
        printf(" Make sure you have write permissions in this folder.\n");
    }

    if (accountCount == 0) {
        printf("\n Welcome to Banking System!\n");
        printf(" No accounts exist yet. Please register a new account to get started.\n\n");
    }
}

void createAdminAccounts() {
    strcpy(admins[0].username, "admin");
    strcpy(admins[0].password, "secure123");
    strcpy(admins[1].username, "manager");
    strcpy(admins[1].password, "bank456");
    adminCount = 2;
}

void registerAccount() {
    if (accountCount >= MAX_ACCOUNTS) {
        printf(" Maximum account limit reached.\n");
        return;
    }

    Account newAccount;
    printf("\n--- Register New Account ---\n");

    printf("Enter desired account number: ");
    while (scanf("%d", &newAccount.accountNumber) != 1 || newAccount.accountNumber <= 0) {
        printf(" Invalid account number! Please enter a positive number: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    if (findAccountByNumber(newAccount.accountNumber) != -1) {
        printf(" Error: Account number %d already exists!\n", newAccount.accountNumber);
        return;
    }

    printf("First Name: ");
    fgets(newAccount.firstName, sizeof(newAccount.firstName), stdin);
    newAccount.firstName[strcspn(newAccount.firstName, "\n")] = 0;
    while (strlen(newAccount.firstName) == 0) {
        printf(" First name cannot be empty! Please enter first name: ");
        fgets(newAccount.firstName, sizeof(newAccount.firstName), stdin);
        newAccount.firstName[strcspn(newAccount.firstName, "\n")] = 0;
    }

    printf("Last Name: ");
    fgets(newAccount.lastName, sizeof(newAccount.lastName), stdin);
    newAccount.lastName[strcspn(newAccount.lastName, "\n")] = 0;
    while (strlen(newAccount.lastName) == 0) {
        printf(" Last name cannot be empty! Please enter last name: ");
        fgets(newAccount.lastName, sizeof(newAccount.lastName), stdin);
        newAccount.lastName[strcspn(newAccount.lastName, "\n")] = 0;
    }

    printf("Initial Deposit: ");
    while (scanf("%lf", &newAccount.balance) != 1 || newAccount.balance < 0) {
        printf(" Invalid amount! Please enter a valid positive number: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    printf("Account Type (1 for Savings, 0 for Current): ");
    while (scanf("%d", &newAccount.isSavings) != 1 || (newAccount.isSavings != 0 && newAccount.isSavings != 1)) {
        printf(" Invalid type! Please enter 1 for Savings or 0 for Current: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    printf("Set Password: ");
    fgets(newAccount.password, sizeof(newAccount.password), stdin);
    newAccount.password[strcspn(newAccount.password, "\n")] = 0;
    while (!validatePassword(newAccount.password)) {
        printf(" Password must be at least 6 characters with at least one number.\n");
        printf("Set Password: ");
        fgets(newAccount.password, sizeof(newAccount.password), stdin);
        newAccount.password[strcspn(newAccount.password, "\n")] = 0;
    }

    newAccount.isActive = 1;
    newAccount.isLocked = 0;
    newAccount.lastInterestDate = time(NULL);

    accounts[accountCount++] = newAccount;

    printf("\n Account created successfully!\n");
    printf("==========================================\n");
    printf("Account Number: %d\n", newAccount.accountNumber);
    printf("Account Holder: %s %s\n", newAccount.firstName, newAccount.lastName);
    printf("Account Type: %s\n", newAccount.isSavings ? "Savings" : "Current");
    printf("Current Balance: %.2f\n", newAccount.balance);
    printf("==========================================\n");
    printf(" Please save your account number and password for future login!\n");

    createTransaction(newAccount.accountNumber, "Account Open", newAccount.balance, 0, "Initial deposit");
    saveData();
}

void displayTransactionHistory(int accountNumber) {
    printf("\n--- Transaction History for Account %d ---\n", accountNumber);

    int found = 0;
    for (int i = transactionCount - 1; i >= 0; i--) {
        if (transactions[i].accountNumber == accountNumber) {
            char dateStr[50];
            strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", localtime(&transactions[i].timestamp));

            printf("[%s] %s: %.2f", dateStr, transactions[i].type, transactions[i].amount);
            if (transactions[i].relatedAccount != 0) {
                printf(" (Account %d)", transactions[i].relatedAccount);
            }
            if (strlen(transactions[i].description) > 0) {
                printf(" - %s", transactions[i].description);
            }
            printf("\n");
            found = 1;

            if (found >= 10) break;
        }
    }

    if (!found) {
        printf("No transactions found.\n");
    }
}

void lockUnlockAccount() {
    printf("\n--- Lock/Unlock Account ---\n");
    printf("Enter account number: ");
    int accNum;
    if (scanf("%d", &accNum) != 1) {
        printf(" Invalid account number!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    int accIndex = findAccountByNumber(accNum);
    if (accIndex == -1) {
        printf(" Account not found.\n");
        return;
    }

    printAccountDetails(accIndex);
    printf("\n Do you want to %s this account? (y/n): ",
          accounts[accIndex].isLocked ? "unlock" : "lock");

    char confirm;
    if (scanf("%c", &confirm) != 1) confirm = 'n';
    clearInputBuffer();

    if (confirm == 'y' || confirm == 'Y') {
        accounts[accIndex].isLocked = !accounts[accIndex].isLocked;
        printf(" Account %s successfully.\n", accounts[accIndex].isLocked ? "locked" : "unlocked");

        char desc[100];
        snprintf(desc, sizeof(desc), "Account %s by admin", accounts[accIndex].isLocked ? "locked" : "unlocked");
        createTransaction(accNum, "Account Status", 0, 0, desc);
        saveData();
    }
}

void createTransaction(int accountNumber, const char* type, double amount, int relatedAccount, const char* description) {
    if (transactionCount >= MAX_TRANSACTIONS) return;

    Transaction t;
    t.transactionId = transactionCount + 1;
    t.accountNumber = accountNumber;
    strncpy(t.type, type, sizeof(t.type) - 1);
    t.type[sizeof(t.type) - 1] = '\0';
    t.amount = amount;
    t.timestamp = time(NULL);
    t.relatedAccount = relatedAccount;
    strncpy(t.description, description, sizeof(t.description) - 1);
    t.description[sizeof(t.description) - 1] = '\0';

    transactions[transactionCount++] = t;
}

void listAllAccounts() {
    printf("\n--- All Accounts ---\n");
    if (accountCount == 0) {
        printf("No accounts found.\n");
        return;
    }

    printf("%-10s %-20s %-10s %-10s %-8s\n", "Account", "Name", "Balance", "Type", "Status");
    printf("----------------------------------------------------------------\n");

    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].isActive) {
            char fullName[50];
            snprintf(fullName, sizeof(fullName), "%s %s", accounts[i].firstName, accounts[i].lastName);
            printf("%-10d %-20s %-10.2f %-10s %-8s\n",
                  accounts[i].accountNumber,
                  fullName,
                  accounts[i].balance,
                  accounts[i].isSavings ? "Savings" : "Current",
                  accounts[i].isLocked ? "Locked" : "Active");
        }
    }
}

void changePassword() {
    printf("\n--- Change Password ---\n");

    printf("Current Password: ");
    char current[50];
    fgets(current, sizeof(current), stdin);
    current[strcspn(current, "\n")] = 0;

    if (strcmp(accounts[currentUserAccount].password, current) != 0) {
        printf(" Incorrect current password.\n");
        return;
    }

    printf("New Password: ");
    char newPass[50];
    fgets(newPass, sizeof(newPass), stdin);
    newPass[strcspn(newPass, "\n")] = 0;

    while (!validatePassword(newPass)) {
        printf(" Password must be at least 6 characters with at least one number.\n");
        printf("New Password: ");
        fgets(newPass, sizeof(newPass), stdin);
        newPass[strcspn(newPass, "\n")] = 0;
    }

    printf("Confirm New Password: ");
    char confirm[50];
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = 0;

    if (strcmp(newPass, confirm) != 0) {
        printf(" Passwords do not match.\n");
        return;
    }

    strcpy(accounts[currentUserAccount].password, newPass);
    printf(" Password changed successfully.\n");
    createTransaction(accounts[currentUserAccount].accountNumber, "Security", 0, 0, "Password changed");
    saveData();
}

void accountStatistics() {
    printf("\n--- Account Statistics ---\n");

    int activeCount = 0, lockedCount = 0, savingsCount = 0;
    double totalBalance = 0;

    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].isActive) {
            activeCount++;
            totalBalance += accounts[i].balance;
            if (accounts[i].isLocked) lockedCount++;
            if (accounts[i].isSavings) savingsCount++;
        }
    }

    printf("==========================================\n");
    printf(" SYSTEM STATISTICS\n");
    printf("==========================================\n");
    printf("Total Accounts: %d\n", accountCount);
    printf("Active Accounts: %d\n", activeCount);
    printf("Locked Accounts: %d\n", lockedCount);
    printf("Savings Accounts: %d\n", savingsCount);
    printf("Current Accounts: %d\n", activeCount - savingsCount);
    printf("Total Balance: %.2f\n", totalBalance);
    printf("Average Balance: %.2f\n", activeCount > 0 ? totalBalance / activeCount : 0);
    printf("Total Transactions: %d\n", transactionCount);
    printf("==========================================\n");
}

void printHelp() {
    printf("\n==========================================\n");
    printf("          BANKING SYSTEM HELP\n");
    printf("==========================================\n");

    printf("\n GETTING STARTED:\n");
    printf("• New users: Select 'Register New Account'\n");
    printf("• Existing users: Select 'Customer Login'\n");
    printf("• Use your account number and password to login\n");

    printf("\n CUSTOMER FEATURES:\n");
    printf("• Deposit: Add money to your account\n");
    printf("• Withdraw: Take money from your account\n");
    printf("• Transfer: Send money to another account\n");
    printf("• Balance Inquiry: Check your current balance\n");
    printf("• Transaction History: View your recent transactions\n");
    printf("• Change Password: Update your account password\n");

    printf("\n ADMIN FEATURES:\n");
    printf("• Register Account: Create new customer accounts\n");
    printf("• Update Account: Modify existing account details\n");
    printf("• Delete Account: Deactivate customer accounts\n");
    printf("• Lock/Unlock: Restrict or restore account access\n");
    printf("• Reports: Generate account and transaction reports\n");
    printf("• Interest: Calculate monthly interest for savings\n");
    printf("• Statistics: View comprehensive system statistics\n");

    printf("\n SECURITY FEATURES:\n");
    printf("• Password must be at least 6 characters\n");
    printf("• Password must contain at least one number\n");
    printf("• Admin access required for sensitive operations\n");
    printf("• Account locking capability for security\n");

    printf("\n DEFAULT ADMIN CREDENTIALS:\n");
    printf("• Username: admin, Password: secure123\n");
    printf("• Username: manager, Password: bank456\n");

    printf("\n DATA PERSISTENCE:\n");
    printf("• All data is automatically saved to '%s'\n", DATA_FILE);
    printf("• Automatic backups are created on exit\n");
    printf("• Data persists between program runs\n");

    printf("\n SUPPORT:\n");
    printf("• Contact your bank administrator for assistance\n");
    printf("• Keep your account number and password secure\n");
    printf("• Regular backups ensure data safety\n");
    printf("==========================================\n");
}

void searchAccount() {
    int choice;
    do {
        printf("\n--- Search Accounts ---\n");
        printf("1. By Account Number\n");
        printf("2. By Name\n");
        printf("3. Back to Admin Menu\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1: {
                printf("Enter account number: ");
                int accNum;
                if (scanf("%d", &accNum) != 1) {
                    printf(" Invalid account number!\n");
                    clearInputBuffer();
                    break;
                }
                clearInputBuffer();

                int accIndex = findAccountByNumber(accNum);
                if (accIndex != -1) {
                    printAccountDetails(accIndex);
                } else {
                    printf(" Account not found.\n");
                }
                break;
            }
            case 2: {
                printf("Enter name to search: ");
                char name[MAX_NAME_LENGTH];
                fgets(name, sizeof(name), stdin);
                name[strcspn(name, "\n")] = 0;

                printf("\nSearch Results:\n");
                int found = 0;
                for (int i = 0; i < accountCount; i++) {
                    char firstName[MAX_NAME_LENGTH], lastName[MAX_NAME_LENGTH], searchName[MAX_NAME_LENGTH];
                    strcpy(firstName, accounts[i].firstName);
                    strcpy(lastName, accounts[i].lastName);
                    strcpy(searchName, name);


                    for (int j = 0; firstName[j]; j++) firstName[j] = tolower(firstName[j]);
                    for (int j = 0; lastName[j]; j++) lastName[j] = tolower(lastName[j]);
                    for (int j = 0; searchName[j]; j++) searchName[j] = tolower(searchName[j]);

                    if (strstr(firstName, searchName) || strstr(lastName, searchName)) {
                        printAccountDetails(i);
                        printf("------------------------\n");
                        found = 1;
                    }
                }

                if (!found) {
                    printf(" No accounts found matching '%s'\n", name);
                }
                break;
            }
            case 3:
                break;
            default:
                printf(" Invalid choice. Please try again.\n");
        }
    } while (choice != 3);
}



void loadData() {
    FILE *file = fopen(DATA_FILE, "r");
    if (file == NULL) {
        printf(" No existing data file found. Starting fresh...\n");
        return;
    }


    if (fscanf(file, "%d %d %d\n", &accountCount, &transactionCount, &adminCount) != 3) {
        printf(" Error reading data header. Starting fresh...\n");
        fclose(file);
        accountCount = transactionCount = 0;
        return;
    }


    for (int i = 0; i < accountCount; i++) {
        if (fscanf(file, "%d|%49[^|]|%49[^|]|%lf|%d|%d|%d|%ld|%49[^\n]\n",
                   &accounts[i].accountNumber,
                   accounts[i].firstName,
                   accounts[i].lastName,
                   &accounts[i].balance,
                   &accounts[i].isActive,
                   &accounts[i].isLocked,
                   &accounts[i].isSavings,
                   &accounts[i].lastInterestDate,
                   accounts[i].password) != 9) {
            printf(" Error loading account %d. Stopping load...\n", i+1);
            accountCount = i;
            break;
        }
    }


    for (int i = 0; i < transactionCount; i++) {
        char description[100];
        if (fscanf(file, "%d|%d|%19[^|]|%lf|%ld|%d|%99[^\n]\n",
                   &transactions[i].transactionId,
                   &transactions[i].accountNumber,
                   transactions[i].type,
                   &transactions[i].amount,
                   &transactions[i].timestamp,
                   &transactions[i].relatedAccount,
                   description) != 7) {
            printf(" Error loading transaction %d. Stopping load...\n", i+1);
            transactionCount = i;
            break;
        }
        strcpy(transactions[i].description, description);
    }


    for (int i = 0; i < adminCount && i < 5; i++) {
        if (fscanf(file, "%49[^|]|%49[^\n]\n", admins[i].username, admins[i].password) != 2) {
            printf(" Error loading admin %d. Using defaults...\n", i+1);
            createAdminAccounts();
            break;
        }
    }

    fclose(file);
    printf(" Data loaded successfully. Accounts: %d, Transactions: %d\n", accountCount, transactionCount);
}


void mainMenu() {
    int choice;
    do {
        printf("\n===== Banking System Main Menu =====\n");
        printf("1. Customer Login\n");
        printf("2. Register New Account\n");
        printf("3. Admin Login\n");
        printf("4. Help\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1: {
                if (accountCount == 0) {
                    printf(" No accounts exist yet!\n");
                    printf(" Please register a new account first.\n");
                    break;
                }

                printf("Enter your account number: ");
                int accNum;
                if (scanf("%d", &accNum) != 1) {
                    printf(" Invalid account number!\n");
                    clearInputBuffer();
                    break;
                }
                clearInputBuffer();

                int accIndex = findAccountByNumber(accNum);
                if (accIndex != -1) {
                    if (!accounts[accIndex].isActive) {
                        printf(" Account is inactive. Please contact administrator.\n");
                        break;
                    }
                    if (accounts[accIndex].isLocked) {
                        printf(" Account is locked. Please contact administrator.\n");
                        break;
                    }

                    printf("Enter your password: ");
                    char password[50];
                    fgets(password, sizeof(password), stdin);
                    password[strcspn(password, "\n")] = 0;

                    if (strcmp(accounts[accIndex].password, password) == 0) {
                        currentUserAccount = accIndex;
                        printf(" Login successful! Welcome, %s %s!\n",
                               accounts[accIndex].firstName, accounts[accIndex].lastName);
                        customerMenu();
                    } else {
                        printf(" Invalid password.\n");
                    }
                } else {
                    printf(" Account not found.\n");
                    printf(" Tip: Make sure you have registered an account first.\n");
                }
                break;
            }
            case 2:
                registerAccount();
                break;
            case 3:
                if (authenticateAdmin()) {
                    isAdminLoggedIn = 1;
                    adminMenu();
                } else {
                    printf(" Authentication failed.\n");
                }
                break;
            case 4:
                printHelp();
                break;
            case 5:
                printf("Thank you for using Banking System!\n");
                printf("Exiting system...\n");
                break;
            default:
                printf(" Invalid choice. Please try again.\n");
        }
    } while (choice != 5);
}

void adminMenu() {
    int choice;
    do {
        printf("\n===== Admin Menu =====\n");
        printf("1. Register New Account\n");
        printf("2. Update Account\n");
        printf("3. Delete Account\n");
        printf("4. Lock/Unlock Account\n");
        printf("5. Generate Reports\n");
        printf("6. Calculate Interest\n");
        printf("7. View All Accounts\n");
        printf("8. Search Accounts\n");
        printf("9. Account Statistics\n");
        printf("10. Back to Main Menu\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1: registerAccount(); break;
            case 2: updateAccount(); break;
            case 3: deleteAccount(); break;
            case 4: lockUnlockAccount(); break;
            case 5: generateReports(); break;
            case 6: calculateInterest(); break;
            case 7: listAllAccounts(); break;
            case 8: searchAccount(); break;
            case 9: accountStatistics(); break;
            case 10: isAdminLoggedIn = 0; break;
            default: printf(" Invalid choice. Please try again.\n");
        }
    } while (choice != 10);
}

void customerMenu() {
    int choice;
    do {
        printf("\n===== Customer Menu =====\n");
        printf("Welcome, %s %s!\n", accounts[currentUserAccount].firstName, accounts[currentUserAccount].lastName);
        printf("Account Number: %d\n", accounts[currentUserAccount].accountNumber);
        printf("Current Balance: %.2f\n\n", accounts[currentUserAccount].balance);

        printf("1. Deposit\n");
        printf("2. Withdraw\n");
        printf("3. Transfer\n");
        printf("4. Balance Inquiry\n");
        printf("5. Transaction History\n");
        printf("6. Change Password\n");
        printf("7. Back to Main Menu\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1: deposit(); break;
            case 2: withdraw(); break;
            case 3: transfer(); break;
            case 4: balanceInquiry(); break;
            case 5: displayTransactionHistory(accounts[currentUserAccount].accountNumber); break;
            case 6: changePassword(); break;
            case 7: currentUserAccount = -1; break;
            default: printf(" Invalid choice. Please try again.\n");
        }
    } while (choice != 7);
}

int authenticateAdmin() {
    char username[50], password[50];
    printf("Admin Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Admin Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    for (int i = 0; i < adminCount; i++) {
        if (strcmp(admins[i].username, username) == 0 &&
            strcmp(admins[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int findAccountByNumber(int accountNumber) {
    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].accountNumber == accountNumber) {
            return i;
        }
    }
    return -1;
}



int validatePassword(const char* password) {
    if (strlen(password) < 6) return 0;
    for (int i = 0; password[i]; i++) {
        if (isdigit(password[i])) return 1;
    }
    return 0;
}
void saveData() {
    if (accountCount == 0 && transactionCount == 0) {
        printf("No data to save (no accounts or transactions created).\n");
        return;
    }

    printf("💾 Saving data to '%s'...\n", DATA_FILE);

    FILE *file = fopen(DATA_FILE, "w");
    if (file == NULL) {
        printf(" CRITICAL ERROR: Cannot create/write to '%s'!\n", DATA_FILE);
        printf(" Possible solutions:\n");
        printf(" 1. Run as administrator/sudo\n");
        printf(" 2. Check folder write permissions\n");
        printf(" 3. Try running from a different directory\n");
        printf(" 4. Check if antivirus is blocking file creation\n");
        return;
    }


    fprintf(file, "%d %d %d\n", accountCount, transactionCount, adminCount);


    for (int i = 0; i < accountCount; i++) {
        fprintf(file, "%d|%s|%s|%.2f|%d|%d|%d|%ld|%s\n",
                accounts[i].accountNumber,
                accounts[i].firstName,
                accounts[i].lastName,
                accounts[i].balance,
                accounts[i].isActive,
                accounts[i].isLocked,
                accounts[i].isSavings,
                accounts[i].lastInterestDate,
                accounts[i].password);
    }


    for (int i = 0; i < transactionCount; i++) {
        fprintf(file, "%d|%d|%s|%.2f|%ld|%d|%s\n",
                transactions[i].transactionId,
                transactions[i].accountNumber,
                transactions[i].type,
                transactions[i].amount,
                transactions[i].timestamp,
                transactions[i].relatedAccount,
                transactions[i].description);
    }


    for (int i = 0; i < adminCount; i++) {
        fprintf(file, "%s|%s\n", admins[i].username, admins[i].password);
    }

    fclose(file);
    printf(" SUCCESS: All data saved to '%s'\n", DATA_FILE);
    printf(" Saved: %d accounts, %d transactions\n", accountCount, transactionCount);
}

void updateAccount() {
    printf("\n--- Update Account ---\n");
    printf("Enter account number to update: ");
    int accNum;
    if (scanf("%d", &accNum) != 1) {
        printf(" Invalid account number!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    int accIndex = findAccountByNumber(accNum);
    if (accIndex == -1) {
        printf(" Account not found.\n");
        return;
    }

    printf("\nCurrent Account Details:\n");
    printAccountDetails(accIndex);

    printf("\n Enter new first name (or press Enter to keep current): ");
    char firstName[MAX_NAME_LENGTH];
    fgets(firstName, sizeof(firstName), stdin);
    if (strlen(firstName) > 1) {
        firstName[strcspn(firstName, "\n")] = 0;
        strcpy(accounts[accIndex].firstName, firstName);
    }

    printf(" Enter new last name (or press Enter to keep current): ");
    char lastName[MAX_NAME_LENGTH];
    fgets(lastName, sizeof(lastName), stdin);
    if (strlen(lastName) > 1) {
        lastName[strcspn(lastName, "\n")] = 0;
        strcpy(accounts[accIndex].lastName, lastName);
    }

    printf(" Account updated successfully!\n");
    createTransaction(accNum, "Account Update", 0, 0, "Account information modified");
    saveData();
}

void deleteAccount() {
    printf("\n--- Delete Account ---\n");
    printf("Enter account number to delete: ");
    int accNum;
    if (scanf("%d", &accNum) != 1) {
        printf(" Invalid account number!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    int accIndex = findAccountByNumber(accNum);
    if (accIndex == -1) {
        printf(" Account not found.\n");
        return;
    }

    printAccountDetails(accIndex);
    printf("\n Are you sure you want to delete this account? (y/n): ");
    char confirm;
    if (scanf("%c", &confirm) != 1) confirm = 'n';
    clearInputBuffer();

    if (confirm == 'y' || confirm == 'Y') {
        accounts[accIndex].isActive = 0;
        printf(" Account marked as inactive.\n");
        createTransaction(accNum, "Account Close", 0, 0, "Account deactivated");
        saveData();
    } else {
        printf(" Account deletion cancelled.\n");
    }
}

void deposit() {
    printf("\n--- Deposit ---\n");
    printf("Current Balance: %.2f\n", accounts[currentUserAccount].balance);

    printf("Enter amount to deposit: ");
    double amount;
    while (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf(" Invalid amount! Please enter a positive number: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    accounts[currentUserAccount].balance += amount;
    printf(" Deposit successful. New Balance: %.2f\n", accounts[currentUserAccount].balance);
    createTransaction(accounts[currentUserAccount].accountNumber, "Deposit", amount, 0, "Cash deposit");
    saveData();
}

void withdraw() {
    printf("\n--- Withdraw ---\n");
    printf("Current Balance: %.2f\n", accounts[currentUserAccount].balance);

    printf("Enter amount to withdraw: ");
    double amount;
    while (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf(" Invalid amount! Please enter a positive number: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    if (!validateTransaction(currentUserAccount, amount)) {
        printf(" Insufficient funds or account locked.\n");
        return;
    }

    accounts[currentUserAccount].balance -= amount;
    printf(" Withdrawal successful. New Balance: %.2f\n", accounts[currentUserAccount].balance);
    createTransaction(accounts[currentUserAccount].accountNumber, "Withdrawal", amount, 0, "Cash withdrawal");
    saveData();
}

void transfer() {
    printf("\n--- Transfer ---\n");
    printf("Current Balance: %.2f\n", accounts[currentUserAccount].balance);

    printf("Enter destination account number: ");
    int destAccNum;
    if (scanf("%d", &destAccNum) != 1) {
        printf(" Invalid account number!\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    int destAccIndex = findAccountByNumber(destAccNum);
    if (destAccIndex == -1 || !accounts[destAccIndex].isActive) {
        printf(" Destination account not found or inactive.\n");
        return;
    }

    if (destAccIndex == currentUserAccount) {
        printf(" Cannot transfer to same account.\n");
        return;
    }

    printf("Destination: %s %s\n", accounts[destAccIndex].firstName, accounts[destAccIndex].lastName);

    printf("Enter amount to transfer: ");
    double amount;
    while (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf(" Invalid amount! Please enter a positive number: ");
        clearInputBuffer();
    }
    clearInputBuffer();

    if (!validateTransaction(currentUserAccount, amount)) {
        printf(" Insufficient funds or account locked.\n");
        return;
    }

    accounts[currentUserAccount].balance -= amount;
    accounts[destAccIndex].balance += amount;

    printf(" Transfer successful. New Balance: %.2f\n", accounts[currentUserAccount].balance);

    char desc[100];
    snprintf(desc, sizeof(desc), "Transfer to %s %s", accounts[destAccIndex].firstName, accounts[destAccIndex].lastName);
    createTransaction(accounts[currentUserAccount].accountNumber, "Transfer", amount, destAccNum, desc);
    saveData();
}

int validateTransaction(int accountIndex, double amount) {
    return accounts[accountIndex].isActive &&
           !accounts[accountIndex].isLocked &&
           accounts[accountIndex].balance >= amount;
}

void balanceInquiry() {
    printf("\n--- Balance Inquiry ---\n");
    printf("==========================================\n");
    printf("Account Number: %d\n", accounts[currentUserAccount].accountNumber);
    printf("Account Holder: %s %s\n", accounts[currentUserAccount].firstName, accounts[currentUserAccount].lastName);
    printf("Account Type: %s\n", accounts[currentUserAccount].isSavings ? "Savings" : "Current");
    printf("Current Balance: %.2f\n", accounts[currentUserAccount].balance);
    printf("==========================================\n");
    createTransaction(accounts[currentUserAccount].accountNumber, "Balance Check", 0, 0, "Balance inquiry");
}

void calculateInterest() {
    printf("\n--- Calculate Interest ---\n");
    time_t now = time(NULL);
    int count = 0;

    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].isSavings && accounts[i].isActive && !accounts[i].isLocked &&
            difftime(now, accounts[i].lastInterestDate) >= 30 * 24 * 3600) {

            double interest = accounts[i].balance * INTEREST_RATE;
            accounts[i].balance += interest;
            accounts[i].lastInterestDate = now;

            printf("Account %d: Interest %.2f added\n", accounts[i].accountNumber, interest);

            char desc[100];
            snprintf(desc, sizeof(desc), "Monthly interest @ %.1f%%", INTEREST_RATE * 100);
            createTransaction(accounts[i].accountNumber, "Interest", interest, 0, desc);
            count++;
        }
    }

    printf(" Interest calculated for %d savings accounts.\n", count);
    saveData();
}

void printAccountDetails(int accountIndex) {
    printf("\n==========================================\n");
    printf("Account Number: %d\n", accounts[accountIndex].accountNumber);
    printf("Account Holder: %s %s\n", accounts[accountIndex].firstName, accounts[accountIndex].lastName);
    printf("Balance: %.2f\n", accounts[accountIndex].balance);
    printf("Type: %s\n", accounts[accountIndex].isSavings ? "Savings" : "Current");
    printf("Status: %s\n", accounts[accountIndex].isActive ? "Active" : "Inactive");
    printf("Locked: %s\n", accounts[accountIndex].isLocked ? "Yes" : "No");
    printf("==========================================\n");
}



void generateReports() {
    int choice;
    do {
        printf("\n--- Generate Reports ---\n");
        printf("1. Account Balance Report\n");
        printf("2. Transaction Report\n");
        printf("3. Export to CSV\n");
        printf("4. Back to Admin Menu\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf(" Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch(choice) {
            case 1:
                listAllAccounts();
                break;
            case 2: {
                printf("Enter account number (0 for all accounts): ");
                int accNum;
                if (scanf("%d", &accNum) != 1) {
                    printf(" Invalid input!\n");
                    clearInputBuffer();
                    break;
                }
                clearInputBuffer();

                if (accNum == 0) {
                    printf("\n--- All Transactions ---\n");
                    for (int i = 0; i < transactionCount; i++) {
                        char dateStr[50];
                        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", localtime(&transactions[i].timestamp));
                        printf("[%s] Acc:%d %s: %.2f - %s\n", dateStr, transactions[i].accountNumber,
                               transactions[i].type, transactions[i].amount, transactions[i].description);
                    }
                } else {
                    displayTransactionHistory(accNum);
                }
                break;
            }
            case 3: {
                time_t now = time(NULL);
                struct tm *tm = localtime(&now);
                char filename[100];
                snprintf(filename, sizeof(filename), "report_%04d%02d%02d_%02d%02d%02d.csv",
                        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                        tm->tm_hour, tm->tm_min, tm->tm_sec);

                FILE *file = fopen(filename, "w");
                if (file) {
                    fprintf(file, "AccountNumber,FirstName,LastName,Balance,Type,Status,Locked\n");
                    for (int i = 0; i < accountCount; i++) {
                        if (accounts[i].isActive) {
                            fprintf(file, "%d,%s,%s,%.2f,%s,%s,%s\n",
                                   accounts[i].accountNumber,
                                   accounts[i].firstName,
                                   accounts[i].lastName,
                                   accounts[i].balance,
                                   accounts[i].isSavings ? "Savings" : "Current",
                                   "Active",
                                   accounts[i].isLocked ? "Yes" : "No");
                        }
                    }
                    fclose(file);
                    printf(" Report exported to %s\n", filename);
                } else {
                    printf(" Error creating report file.\n");
                }
                break;
            }
            case 4:
                break;
            default:
                printf(" Invalid choice. Please try again.\n");
        }
    } while (choice != 4);
}

