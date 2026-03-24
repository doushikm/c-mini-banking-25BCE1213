#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ACCOUNTS_FILE  "accounts.dat"
#define LOG_FILE       "transactions.log"
#define MAX_NAME       50
#define MAX_TRANS_SHOW 5
#define MAX_RECORDS    1000

struct Account {
    int    accNo;
    char   name[MAX_NAME];
    double balance;
};

struct Transaction {
    int    accNo;
    char   type[15];
    double amount;
    char   timestamp[25];
};

void   createAccount();
void   deposit();
void   withdraw();
void   viewAccount();
void   searchAccount();
void   viewLastTransactions();
void   displayAllAccounts();
void   logTransaction(int accNo, const char *type, double amount);
int    accountExists(int accNo, struct Account *acc);
void   saveAccount(struct Account acc);
void   updateAccountBalance(int accNo, double newBalance);
void   getTimestamp(char *buffer);
void   displayMenu();

int main() {
    int choice;
    printf("\n+======================================+\n");
    printf(  "|   MINI BANKING SYSTEM  v1.0          |\n");
    printf(  "+======================================+\n");
    do {
        displayMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: createAccount();        break;
            case 2: deposit();              break;
            case 3: withdraw();             break;
            case 4: viewAccount();          break;
            case 5: searchAccount();        break;
            case 6: viewLastTransactions(); break;
            case 7: displayAllAccounts();   break;
            case 0: printf("\nGoodbye!\n\n"); break;
            default: printf("\n[!] Invalid choice. Enter 0-7.\n");
        }
    } while (choice != 0);
    return 0;
}

void displayMenu() {
    printf("\n+-------------------------------------+\n");
    printf("|            MAIN MENU                |\n");
    printf("+-------------------------------------+\n");
    printf("|  1. Create Account                  |\n");
    printf("|  2. Deposit                         |\n");
    printf("|  3. Withdraw                        |\n");
    printf("|  4. View Account Summary            |\n");
    printf("|  5. Search Account                  |\n");
    printf("|  6. Last 5 Transactions             |\n");
    printf("|  7. Display All Accounts            |\n");
    printf("|  0. Exit                            |\n");
    printf("+-------------------------------------+\n");
}

void createAccount() {
    struct Account acc;
    printf("\n--- Create New Account ---\n");

    printf("Enter Account Number (4-8 digits): ");
    scanf("%d", &acc.accNo);
    if (acc.accNo < 1000 || acc.accNo > 99999999) {
        printf("[!] Invalid account number.\n"); return;
    }
    struct Account tmp;
    if (accountExists(acc.accNo, &tmp)) {
        printf("[!] Account %d already exists!\n", acc.accNo); return;
    }

    printf("Enter Account Holder Name: ");
    scanf(" %49[^\n]", acc.name);
    if (strlen(acc.name) == 0) { printf("[!] Name cannot be empty.\n"); return; }

    printf("Enter Initial Deposit (min Rs.500): ");
    scanf("%lf", &acc.balance);
    if (acc.balance < 500) { printf("[!] Minimum deposit is Rs.500.\n"); return; }

    saveAccount(acc);
    logTransaction(acc.accNo, "CREATE", acc.balance);
    printf("\n[OK] Account created! No:%d  Name:%s  Balance:Rs.%.2f\n",
           acc.accNo, acc.name, acc.balance);
}

void deposit() {
    int accNo; double amount; struct Account acc;
    printf("\n--- Deposit ---\n");
    printf("Enter Account Number: "); scanf("%d", &accNo);
    if (!accountExists(accNo, &acc)) { printf("[!] Account not found.\n"); return; }
    printf("Enter deposit amount: "); scanf("%lf", &amount);
    if (amount <= 0) { printf("[!] Amount must be positive.\n"); return; }
    if (amount > 1000000) { printf("[!] Exceeds transaction limit.\n"); return; }
    acc.balance += amount;
    updateAccountBalance(acc.accNo, acc.balance);
    logTransaction(acc.accNo, "DEPOSIT", amount);
    printf("\n[OK] Deposited Rs.%.2f  | New Balance: Rs.%.2f\n", amount, acc.balance);
}

void withdraw() {
    int accNo; double amount; struct Account acc;
    printf("\n--- Withdraw ---\n");
    printf("Enter Account Number: "); scanf("%d", &accNo);
    if (!accountExists(accNo, &acc)) { printf("[!] Account not found.\n"); return; }
    printf("Enter withdrawal amount: "); scanf("%lf", &amount);
    if (amount <= 0) { printf("[!] Amount must be positive.\n"); return; }
    if (amount > acc.balance) {
        printf("[!] Insufficient balance! Available: Rs.%.2f\n", acc.balance); return;
    }
    if (acc.balance - amount < 500) {
        printf("[!] Min balance Rs.500 required. Max you can withdraw: Rs.%.2f\n",
               acc.balance - 500.0); return;
    }
    acc.balance -= amount;
    updateAccountBalance(acc.accNo, acc.balance);
    logTransaction(acc.accNo, "WITHDRAW", amount);
    printf("\n[OK] Withdrawn Rs.%.2f  | New Balance: Rs.%.2f\n", amount, acc.balance);
}

void viewAccount() {
    int accNo; struct Account acc;
    printf("\n--- Account Summary ---\n");
    printf("Enter Account Number: "); scanf("%d", &accNo);
    if (!accountExists(accNo, &acc)) { printf("[!] Account not found.\n"); return; }
    printf("\n+----------------------------------+\n");
    printf("|         ACCOUNT DETAILS          |\n");
    printf("+----------------------------------+\n");
    printf("| Account No : %-19d|\n", acc.accNo);
    printf("| Name       : %-19s|\n", acc.name);
    printf("| Balance    : Rs.%-16.2f|\n", acc.balance);
    printf("+----------------------------------+\n");
}

void searchAccount() {
    int accNo; struct Account acc;
    printf("\n--- Search Account ---\n");
    printf("Enter Account Number: "); scanf("%d", &accNo);
    if (!accountExists(accNo, &acc)) {
        printf("[!] No account found: %d\n", accNo); return;
    }
    printf("\n[OK] Found -> No:%d  Name:%s  Balance:Rs.%.2f\n",
           acc.accNo, acc.name, acc.balance);
}

void viewLastTransactions() {
    int accNo;
    printf("\n--- Last %d Transactions ---\n", MAX_TRANS_SHOW);
    printf("Enter Account Number: "); scanf("%d", &accNo);
    struct Account tmp;
    if (!accountExists(accNo, &tmp)) { printf("[!] Account not found.\n"); return; }

    FILE *fp = fopen(LOG_FILE, "r");
    if (!fp) { printf("[!] No transaction log found.\n"); return; }

    struct Transaction all[MAX_RECORDS]; int count = 0;
    struct Transaction t;
    while (fscanf(fp, "%d|%14[^|]|%lf|%24[^\n]\n",
                  &t.accNo, t.type, &t.amount, t.timestamp) == 4) {
        if (t.accNo == accNo && count < MAX_RECORDS) all[count++] = t;
    }
    fclose(fp);
    if (count == 0) { printf("[!] No transactions for account %d.\n", accNo); return; }

    int start = (count > MAX_TRANS_SHOW) ? count - MAX_TRANS_SHOW : 0;
    printf("\n%-10s %-12s %-12s %-25s\n", "AccNo", "Type", "Amount", "Timestamp");
    printf("---------------------------------------------------------------\n");
    for (int i = start; i < count; i++)
        printf("%-10d %-12s %-12.2f %-25s\n",
               all[i].accNo, all[i].type, all[i].amount, all[i].timestamp);
}

void displayAllAccounts() {
    FILE *fp = fopen(ACCOUNTS_FILE, "r");
    if (!fp) { printf("[!] No accounts found.\n"); return; }
    struct Account acc; int found = 0;
    printf("\n%-12s %-25s %s\n", "Account No", "Name", "Balance (Rs.)");
    printf("------------------------------------------------------\n");
    while (fscanf(fp, "%d|%49[^|]|%lf\n", &acc.accNo, acc.name, &acc.balance) == 3) {
        printf("%-12d %-25s %.2f\n", acc.accNo, acc.name, acc.balance);
        found++;
    }
    fclose(fp);
    if (!found) { printf("[!] No accounts on record.\n"); return; }
    printf("------------------------------------------------------\n");
    printf("Total Accounts: %d\n", found);
}

int accountExists(int accNo, struct Account *acc) {
    FILE *fp = fopen(ACCOUNTS_FILE, "r");
    if (!fp) return 0;
    while (fscanf(fp, "%d|%49[^|]|%lf\n", &acc->accNo, acc->name, &acc->balance) == 3)
        if (acc->accNo == accNo) { fclose(fp); return 1; }
    fclose(fp);
    return 0;
}

void saveAccount(struct Account acc) {
    FILE *fp = fopen(ACCOUNTS_FILE, "a");
    if (!fp) { printf("[!] Error opening file.\n"); return; }
    fprintf(fp, "%d|%s|%.2f\n", acc.accNo, acc.name, acc.balance);
    fclose(fp);
}

void updateAccountBalance(int accNo, double newBalance) {
    FILE *fp = fopen(ACCOUNTS_FILE, "r");
    if (!fp) return;
    struct Account all[MAX_RECORDS]; int count = 0; struct Account tmp;
    while (fscanf(fp, "%d|%49[^|]|%lf\n", &tmp.accNo, tmp.name, &tmp.balance) == 3) {
        if (tmp.accNo == accNo) tmp.balance = newBalance;
        all[count++] = tmp;
    }
    fclose(fp);
    fp = fopen(ACCOUNTS_FILE, "w");
    if (!fp) return;
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d|%s|%.2f\n", all[i].accNo, all[i].name, all[i].balance);
    fclose(fp);
}

void logTransaction(int accNo, const char *type, double amount) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;
    char ts[25]; getTimestamp(ts);
    fprintf(fp, "%d|%s|%.2f|%s\n", accNo, type, amount, ts);
    fclose(fp);
}

void getTimestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", t);
}
