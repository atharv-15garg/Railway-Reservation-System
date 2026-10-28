#include <stdio.h>
#include <string.h>

#define MAX_TRAINS 5
#define MAX_BOOKINGS 200
#define MAX_WAITING 100
#define NAME_LEN 60
#define STATION_LEN 40

typedef struct {
    int number;
    char name[NAME_LEN];
    char source[STATION_LEN];
    char destination[STATION_LEN];
    int totalSeats;
    int availableSeats;
} Train;

typedef struct {
    int pnr;
    char passengerName[NAME_LEN];
    int age;
    char gender[12];
    int trainNumber;
    int seatNumber;
    int status; /* 1 = confirmed, 2 = waiting, 0 = cancelled */
} Booking;

Train trains[MAX_TRAINS] = {
    {12001, "Shatabdi Express", "Delhi", "Chandigarh", 3, 3},
    {12002, "Shatabdi Express", "Delhi", "Bhopal", 3, 3},
    {12951, "Mumbai Rajdhani", "Mumbai", "Delhi", 3, 3},
    {12301, "Howrah Rajdhani", "Kolkata", "Delhi", 3, 3},
    {12627, "Karnataka Express", "Bengaluru", "Delhi", 3, 3}
};

Booking bookings[MAX_BOOKINGS];
int bookingCount = 0;
int nextPNR = 1001;

/* A simple FIFO waiting list. Each entry stores the booking-array index. */
int waitingQueue[MAX_WAITING];
int waitFront = 0, waitRear = 0, waitCount = 0;

int findTrainIndex(int trainNumber) {
    for (int i = 0; i < MAX_TRAINS; i++) {
        if (trains[i].number == trainNumber) return i;
    }
    return -1;
}

int findBookingIndex(int pnr) {
    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].pnr == pnr) return i;
    }
    return -1;
}

void displayTrains(void) {
    printf("\n%-8s %-22s %-15s %-15s %-10s\n",
           "Number", "Train", "Source", "Destination", "Seats");
    printf("--------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_TRAINS; i++) {
        printf("%-8d %-22s %-15s %-15s %d/%d\n",
               trains[i].number, trains[i].name, trains[i].source,
               trains[i].destination, trains[i].availableSeats,
               trains[i].totalSeats);
    }
}

void searchTrain(void) {
    int choice, number, found = 0;
    char source[STATION_LEN], destination[STATION_LEN];

    printf("\nSearch by: 1. Train number  2. Source and destination\nChoice: ");
    if (scanf("%d", &choice) != 1) { while (getchar() != '\n'); return; }

    if (choice == 1) {
        printf("Enter train number: ");
        if (scanf("%d", &number) != 1) { while (getchar() != '\n'); return; }
        int idx = findTrainIndex(number);
        if (idx >= 0) {
            printf("%d | %s | %s -> %s | Seats available: %d\n",
                   trains[idx].number, trains[idx].name, trains[idx].source,
                   trains[idx].destination, trains[idx].availableSeats);
        } else {
            printf("Train not found.\n");
        }
    } else if (choice == 2) {
        printf("Enter source (use the spelling shown in the train list): ");
        scanf(" %39s", source);
        printf("Enter destination: ");
        scanf(" %39s", destination);
        for (int i = 0; i < MAX_TRAINS; i++) {
            if (strcmp(trains[i].source, source) == 0 &&
                strcmp(trains[i].destination, destination) == 0) {
                printf("%d | %s | %s -> %s | Seats available: %d\n",
                       trains[i].number, trains[i].name, trains[i].source,
                       trains[i].destination, trains[i].availableSeats);
                found = 1;
            }
        }
        if (!found) printf("No matching train found.\n");
    } else {
        printf("Invalid choice.\n");
    }
}

int enqueueWaiting(int bookingIndex) {
    if (waitCount >= MAX_WAITING) return 0;
    waitingQueue[waitRear] = bookingIndex;
    waitRear = (waitRear + 1) % MAX_WAITING;
    waitCount++;
    return 1;
}

int dequeueWaiting(void) {
    if (waitCount == 0) return -1;
    int index = waitingQueue[waitFront];
    waitFront = (waitFront + 1) % MAX_WAITING;
    waitCount--;
    return index;
}

/* After a cancellation, promote the oldest active waiting passenger for that train. */
void promoteWaitingPassenger(int trainIndex) {
    int temp[MAX_WAITING], tempCount = 0, promoted = 0;

    while (waitCount > 0) {
        int idx = dequeueWaiting();
        if (idx < 0 || idx >= bookingCount) continue;

        if (!promoted && bookings[idx].status == 2 &&
            bookings[idx].trainNumber == trains[trainIndex].number) {
            bookings[idx].status = 1;
            bookings[idx].seatNumber =
                trains[trainIndex].totalSeats - trains[trainIndex].availableSeats + 1;
            printf("Waiting passenger %s (PNR %d) is now CONFIRMED. Seat: %d\n",
                   bookings[idx].passengerName, bookings[idx].pnr,
                   bookings[idx].seatNumber);
            promoted = 1;
        } else {
            temp[tempCount++] = idx;
        }
    }

    for (int i = 0; i < tempCount; i++) {
        waitingQueue[waitRear] = temp[i];
        waitRear = (waitRear + 1) % MAX_WAITING;
        waitCount++;
    }
}

void bookTicket(void) {
    int trainNumber, age, trainIndex;
    char name[NAME_LEN], gender[12];

    if (bookingCount >= MAX_BOOKINGS) {
        printf("Booking storage is full.\n");
        return;
    }

    displayTrains();
    printf("\nEnter train number: ");
    if (scanf("%d", &trainNumber) != 1) { while (getchar() != '\n'); return; }

    trainIndex = findTrainIndex(trainNumber);
    if (trainIndex < 0) {
        printf("Invalid train number.\n");
        return;
    }

    printf("Passenger name (no spaces): ");
    scanf("%59s", name);
    printf("Age: ");
    if (scanf("%d", &age) != 1 || age < 1 || age > 120) {
        printf("Invalid age.\n");
        while (getchar() != '\n');
        return;
    }
    printf("Gender (M/F/Other): ");
    scanf("%11s", gender);

    Booking b;
    b.pnr = nextPNR++;
    strcpy(b.passengerName, name);
    b.age = age;
    strcpy(b.gender, gender);
    b.trainNumber = trainNumber;
    b.seatNumber = 0;

    if (trains[trainIndex].availableSeats > 0) {
        b.status = 1;
        b.seatNumber = trains[trainIndex].totalSeats -
                        trains[trainIndex].availableSeats + 1;
        trains[trainIndex].availableSeats--;
        bookings[bookingCount] = b;
        printf("\nTicket CONFIRMED!\nPNR: %d\nTrain: %s\nSeat: %d\n",
               b.pnr, trains[trainIndex].name, b.seatNumber);
    } else {
        b.status = 2;
        bookings[bookingCount] = b;
        if (enqueueWaiting(bookingCount)) {
            printf("\nNo seats available. Added to WAITING LIST.\nPNR: %d\n", b.pnr);
        } else {
            bookings[bookingCount].status = 0;
            printf("Waiting list is full. Booking could not be made.\n");
        }
    }
    bookingCount++;
}

void printBooking(const Booking *b) {
    const char *status = b->status == 1 ? "CONFIRMED" :
                         b->status == 2 ? "WAITING" : "CANCELLED";
    int ti = findTrainIndex(b->trainNumber);
    printf("\nPNR: %d\nPassenger: %s\nAge: %d\nGender: %s\nTrain number: %d\n",
           b->pnr, b->passengerName, b->age, b->gender, b->trainNumber);
    if (ti >= 0) {
        printf("Train: %s (%s -> %s)\n", trains[ti].name,
               trains[ti].source, trains[ti].destination);
    }
    printf("Status: %s\n", status);
    if (b->status == 1) printf("Seat number: %d\n", b->seatNumber);
}

void findTicket(void) {
    int pnr;
    printf("Enter PNR: ");
    if (scanf("%d", &pnr) != 1) { while (getchar() != '\n'); return; }
    int idx = findBookingIndex(pnr);
    if (idx < 0) printf("PNR not found.\n");
    else printBooking(&bookings[idx]);
}

void cancelTicket(void) {
    int pnr;
    printf("Enter PNR to cancel: ");
    if (scanf("%d", &pnr) != 1) { while (getchar() != '\n'); return; }

    int idx = findBookingIndex(pnr);
    if (idx < 0) {
        printf("PNR not found.\n");
        return;
    }
    if (bookings[idx].status == 0) {
        printf("This ticket is already cancelled.\n");
        return;
    }

    int trainIndex = findTrainIndex(bookings[idx].trainNumber);
    int wasConfirmed = bookings[idx].status == 1;
    bookings[idx].status = 0;

    if (wasConfirmed && trainIndex >= 0) {
        trains[trainIndex].availableSeats++;
        printf("Confirmed ticket cancelled.\n");
        promoteWaitingPassenger(trainIndex);
    } else {
        printf("Waiting-list ticket cancelled.\n");
    }
}

void showAllBookings(void) {
    if (bookingCount == 0) {
        printf("No bookings yet.\n");
        return;
    }
    for (int i = 0; i < bookingCount; i++) {
        printf("\n------------------------------");
        printBooking(&bookings[i]);
    }
}

void showPassengerList(void) {
    int trainNumber, found = 0;
    printf("Enter train number: ");
    if (scanf("%d", &trainNumber) != 1) { while (getchar() != '\n'); return; }
    if (findTrainIndex(trainNumber) < 0) {
        printf("Train not found.\n");
        return;
    }
    printf("\nPassengers for train %d:\n", trainNumber);
    for (int i = 0; i < bookingCount; i++) {
        if (bookings[i].trainNumber == trainNumber && bookings[i].status != 0) {
            printf("PNR: %d | %-20s | %s",
                   bookings[i].pnr, bookings[i].passengerName,
                   bookings[i].status == 1 ? "CONFIRMED" : "WAITING");
            if (bookings[i].status == 1)
                printf(" | Seat %d", bookings[i].seatNumber);
            printf("\n");
            found = 1;
        }
    }
    if (!found) printf("No active passengers for this train.\n");
}

int main(void) {
    int choice;
    do {
        printf("\n========== RAILWAY RESERVATION SYSTEM ==========\n");
        printf("1. Display all trains\n");
        printf("2. Search train\n");
        printf("3. Book ticket\n");
        printf("4. Cancel ticket\n");
        printf("5. Find ticket by PNR\n");
        printf("6. Show passenger list for a train\n");
        printf("7. View all bookings\n");
        printf("8. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: displayTrains(); break;
            case 2: searchTrain(); break;
            case 3: bookTicket(); break;
            case 4: cancelTicket(); break;
            case 5: findTicket(); break;
            case 6: showPassengerList(); break;
            case 7: showAllBookings(); break;
            case 8: printf("Thank you for using the system.\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 8);

    return 0;
}
