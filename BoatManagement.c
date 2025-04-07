#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME_LENGTH 127
#define MAX_LENGTH 100
#define MAX_SLIPS 85
#define MAX_STORAGE 50

// Define the boat location types
typedef enum {
    SLIP,
    LAND,
    TRAILOR,
    STORAGE
} LocationType;

// Union for location-specific data
typedef union {
    int slipNumber;            
    char bayLetter;           
    char trailorLicense[10];  
    int storageNumber;         
} LocationDetails;

// Structure to hold boat information
typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    int length;
    LocationType locationType;
    LocationDetails details;
    float amountOwed;
} Boat;

// List of functions
void loadBoats(const char* filename, Boat** boats, int* boatCount);
void saveBoats(const char* filename, Boat** boats, int boatCount);
void displayInventory(Boat** boats, int boatCount);
void addBoat(Boat** boats, int* boatCount, const char* boatData);
void removeBoat(Boat** boats, int* boatCount);
void acceptPayment(Boat** boats, int boatCount);
void updateMonthlyFees(Boat** boats, int boatCount);
int compareBoats(const void* a, const void* b);
void freeAllBoats(Boat** boats, int boatCount);
void printMenu();

int main(int argc, char* argv[]) {
    // Check if the filename is provided
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Boat* boats[MAX_BOATS] = {NULL};
    int boatCount = 0;
    char choice;
    char inputBuffer[256];

    // Load boat data from file
    loadBoats(argv[1], boats, &boatCount);

    // Display welcome message
    printf("\nWelcome to the Boat Management System\n");
    printf("-------------------------------------\n\n");

    // Main program loop
    while (1) {
        printMenu();
        scanf(" %c", &choice);
        choice = toupper(choice);
        
        // Clear input buffer
        while (getchar() != '\n');

        switch (choice) {
            case 'I':
                displayInventory(boats, boatCount);
                break;
            case 'A':
                printf("Please enter the boat data in CSV format                 : ");
                if (fgets(inputBuffer, sizeof(inputBuffer), stdin) != NULL) {
                    // Remove newline character if present
                    size_t len = strlen(inputBuffer);
                    if (len > 0 && inputBuffer[len-1] == '\n') {
                        inputBuffer[len-1] = '\0';
                    }
                    addBoat(boats, &boatCount, inputBuffer);
                }
                break;
            case 'R':
                removeBoat(boats, &boatCount);
                break;
            case 'P':
                acceptPayment(boats, boatCount);
                break;
            case 'M':
                updateMonthlyFees(boats, boatCount);
                break;
            case 'X':
                saveBoats(argv[1], boats, boatCount);
                printf("\nExiting the Boat Management System\n");
                freeAllBoats(boats, boatCount);
                return 0;
            default:
                printf("Invalid option %c\n\n", choice);
        }
    }

    return 0;
}

// Compare function for sort
int compareBoats(const void* a, const void* b) {
    Boat* boatA = *(Boat**)a;
    Boat* boatB = *(Boat**)b;
    return strcasecmp(boatA->name, boatB->name);
}

// Load boats from CSV file
void loadBoats(const char* filename, Boat** boats, int* boatCount) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file %s. Starting with empty inventory.\n", filename);
        return;
    }

    char line[256];
    *boatCount = 0;

    while (fgets(line, sizeof(line), file) && *boatCount < MAX_BOATS) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }

        // Allocate memory for new boat
        boats[*boatCount] = (Boat*)malloc(sizeof(Boat));
        if (boats[*boatCount] == NULL) {
            printf("Memory allocation failed.\n");
            continue;
        }

        // Parse the CSV line
        char locType[10];
        char detailStr[20];
        
        // Format: name,length,type,details,amountOwed
        sscanf(line, "%[^,],%d,%[^,],%[^,],%f", 
            boats[*boatCount]->name, 
            &boats[*boatCount]->length, 
            locType, 
            detailStr, 
            &boats[*boatCount]->amountOwed);

        // Determine location type
        if (strcasecmp(locType, "slip") == 0) {
            boats[*boatCount]->locationType = SLIP;
            sscanf(detailStr, "%d", &boats[*boatCount]->details.slipNumber);
        } else if (strcasecmp(locType, "land") == 0) {
            boats[*boatCount]->locationType = LAND;
            boats[*boatCount]->details.bayLetter = detailStr[0];
        } else if (strcasecmp(locType, "trailor") == 0) {
            boats[*boatCount]->locationType = TRAILOR;
            strncpy(boats[*boatCount]->details.trailorLicense, detailStr, 9);
            boats[*boatCount]->details.trailorLicense[9] = '\0';
        } else if (strcasecmp(locType, "storage") == 0) {
            boats[*boatCount]->locationType = STORAGE;
            sscanf(detailStr, "%d", &boats[*boatCount]->details.storageNumber);
        }

        (*boatCount)++;
    }

    fclose(file);

    // Sort the boats alphabetically by name
    qsort(boats, *boatCount, sizeof(Boat*), compareBoats);
}

// Save boats to CSV file
void saveBoats(const char* filename, Boat** boats, int boatCount) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Could not open file %s for writing.\n", filename);
        return;
    }

    for (int i = 0; i < boatCount; i++) {
        // Write boat data to file
        fprintf(file, "%s,%d,", boats[i]->name, boats[i]->length);
        
        // Write location type and details
        switch (boats[i]->locationType) {
            case SLIP:
                fprintf(file, "slip,%d,", boats[i]->details.slipNumber);
                break;
            case LAND:
                fprintf(file, "land,%c,", boats[i]->details.bayLetter);
                break;
            case TRAILOR:
                fprintf(file, "trailor,%s,", boats[i]->details.trailorLicense);
                break;
            case STORAGE:
                fprintf(file, "storage,%d,", boats[i]->details.storageNumber);
                break;
        }
        
        // Write amount owed
        fprintf(file, "%.2f\n", boats[i]->amountOwed);
    }

    fclose(file);
}

// Display all boats in inventory
void displayInventory(Boat** boats, int boatCount) {
    for (int i = 0; i < boatCount; i++) {
        printf("%-22s %2d' ", boats[i]->name, boats[i]->length);
        
        // Display location type and details
        switch (boats[i]->locationType) {
            case SLIP:
                printf("   slip   # %-2d", boats[i]->details.slipNumber);
                break;
            case LAND:
                printf("   land      %c", boats[i]->details.bayLetter);
                break;
            case TRAILOR:
                printf("trailor %-6s", boats[i]->details.trailorLicense);
                break;
            case STORAGE:
                printf("storage   # %-2d", boats[i]->details.storageNumber);
                break;
        }
        
        // Display amount owed
        printf("   Owes $%6.2f\n", boats[i]->amountOwed);
    }
    printf("\n");
}

// Add a new boat to inventory
void addBoat(Boat** boats, int* boatCount, const char* boatData) {
    if (*boatCount >= MAX_BOATS) {
        printf("Cannot add more boats. Maximum capacity reached.\n");
        return;
    }

    // Allocate memory for new boat
    boats[*boatCount] = (Boat*)malloc(sizeof(Boat));
    if (boats[*boatCount] == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    // Parse the CSV data
    char locType[10];
    char detailStr[20];
    
    // Format: name,length,type,details,amountOwed
    sscanf(boatData, "%[^,],%d,%[^,],%[^,],%f", 
        boats[*boatCount]->name, 
        &boats[*boatCount]->length, 
        locType, 
        detailStr, 
        &boats[*boatCount]->amountOwed);

    // Determine location type
    if (strcasecmp(locType, "slip") == 0) {
        boats[*boatCount]->locationType = SLIP;
        sscanf(detailStr, "%d", &boats[*boatCount]->details.slipNumber);
    } else if (strcasecmp(locType, "land") == 0) {
        boats[*boatCount]->locationType = LAND;
        boats[*boatCount]->details.bayLetter = detailStr[0];
    } else if (strcasecmp(locType, "trailor") == 0) {
        boats[*boatCount]->locationType = TRAILOR;
        strncpy(boats[*boatCount]->details.trailorLicense, detailStr, 9);
        boats[*boatCount]->details.trailorLicense[9] = '\0';
    } else if (strcasecmp(locType, "storage") == 0) {
        boats[*boatCount]->locationType = STORAGE;
        sscanf(detailStr, "%d", &boats[*boatCount]->details.storageNumber);
    }

    (*boatCount)++;

    // Sort the boats alphabetically by name
    qsort(boats, *boatCount, sizeof(Boat*), compareBoats);
}

// Remove a boat from inventory
void removeBoat(Boat** boats, int* boatCount) {
    char boatName[MAX_NAME_LENGTH + 1];
    int foundIndex = -1;

    printf("Please enter the boat name                               : ");
    if (fgets(boatName, sizeof(boatName), stdin) != NULL) {
        // Remove newline character if present
        size_t len = strlen(boatName);
        if (len > 0 && boatName[len-1] == '\n') {
            boatName[len-1] = '\0';
        }

        // Find the boat
        for (int i = 0; i < *boatCount; i++) {
            if (strcasecmp(boats[i]->name, boatName) == 0) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex != -1) {
            // Free memory for the boat
            free(boats[foundIndex]);
            
            // Shift remaining boats
            for (int i = foundIndex; i < *boatCount - 1; i++) {
                boats[i] = boats[i + 1];
            }
            
            boats[*boatCount - 1] = NULL;
            (*boatCount)--;
        } else {
            printf("No boat with that name\n\n");
        }
    }
}

// Accept payment for a boat
void acceptPayment(Boat** boats, int boatCount) {
    char boatName[MAX_NAME_LENGTH + 1];
    float paymentAmount;
    int foundIndex = -1;

    printf("Please enter the boat name                         : ");
    if (fgets(boatName, sizeof(boatName), stdin) != NULL) {
        // Remove newline character if present
        size_t len = strlen(boatName);
        if (len > 0 && boatName[len-1] == '\n') {
            boatName[len-1] = '\0';
        }

        // Find the boat
        for (int i = 0; i < boatCount; i++) {
            if (strcasecmp(boats[i]->name, boatName) == 0) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex != -1) {
            printf("Please enter the amount to be paid                 : ");
            scanf("%f", &paymentAmount);
            while (getchar() != '\n'); // Clear input buffer

            if (paymentAmount > boats[foundIndex]->amountOwed) {
                printf("That is more than the amount owed, $%.2f\n\n",
                boats[foundIndex]->amountOwed);
            } else {
                boats[foundIndex]->amountOwed -= paymentAmount;
            }
        } else {
            printf("No boat with that name\n\n");
        }
    }
}

// Update fees for a new month
void updateMonthlyFees(Boat** boats, int boatCount) {
    for (int i = 0; i < boatCount; i++) {
        switch (boats[i]->locationType) {
            case SLIP:
                boats[i]->amountOwed += 12.50 * boats[i]->length;
                break;
            case LAND:
                boats[i]->amountOwed += 14.00 * boats[i]->length;
                break;
            case TRAILOR:
                boats[i]->amountOwed += 25.00 * boats[i]->length;
                break;
            case STORAGE:
                boats[i]->amountOwed += 11.20 * boats[i]->length;
                break;
        }
    }
    printf("\n");
}

// Free all allocated memory
void freeAllBoats(Boat** boats, int boatCount) {
    for (int i = 0; i < boatCount; i++) {
        if (boats[i] != NULL) {
            free(boats[i]);
            boats[i] = NULL;
        }
    }
}

// Print the menu
void printMenu() {
    printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
}
