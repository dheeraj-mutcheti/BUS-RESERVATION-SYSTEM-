/*
 * BUS RESERVATION SYSTEM  (Lab 12 - Mini Project)
 * ------------------------------------------------
 * Features:
 *   1. Passenger registration
 *   2. Seat booking
 *   3. Ticket cancellation
 *   4. Seat availability
 *   5. Fare calculation
 *   6. Booking report
 *   + Data is saved in a file (bus_data.dat) so it is not lost
 *
 * Libraries used: stdio.h, string.h only
 */

#include <stdio.h>
#include <string.h>

#define MAX_BUSES        3
#define SEATS_PER_BUS    20
#define MAX_PASSENGERS   100
#define MAX_TICKETS      200
#define DATA_FILE        "bus_data.dat"

/* ---------- Structures ---------- */

struct Passenger {
    int  id;
    char name[30];
    int  age;
    char gender;          /* M or F */
    char phone[15];
};

struct Ticket {
    int   ticketNo;
    int   passengerId;
    int   busNo;
    int   seatNo;
    float fare;
    int   active;         /* 1 = booked, 0 = cancelled */
};

struct Bus {
    int   busNo;
    char  name[30];
    char  source[20];
    char  destination[20];
    float baseFare;
    int   seats[SEATS_PER_BUS];   /* 0 = free, otherwise ticket number */
};

/* ---------- Global data ---------- */

struct Bus       buses[MAX_BUSES];
struct Passenger passengers[MAX_PASSENGERS];
struct Ticket    tickets[MAX_TICKETS];

int passengerCount = 0;
int ticketCount    = 0;
int nextPassengerId = 1;
int nextTicketNo    = 1001;

/* ---------- Function declarations ---------- */

void  initBuses(void);
void  saveData(void);
void  loadData(void);
int   readInt(void);
int   findPassenger(int id);
int   findTicket(int ticketNo);
void  displayBuses(void);
void  registerPassenger(void);
void  checkSeatAvailability(void);
float calculateFare(float baseFare, int age);
void  bookSeat(void);
void  cancelTicket(void);
void  bookingReport(void);

/* ---------- Helper: safe integer input ---------- */

int readInt(void)
{
    int x;
    while (scanf("%d", &x) != 1) {
        while (getchar() != '\n');          /* throw away bad input */
        printf("Invalid input! Enter a number: ");
    }
    return x;
}

/* ---------- Setup the 3 buses ---------- */

void initBuses(void)
{
    int i, j;

    buses[0].busNo = 1;
    strcpy(buses[0].name, "Express Travels");
    strcpy(buses[0].source, "Hyderabad");
    strcpy(buses[0].destination, "Bangalore");
    buses[0].baseFare = 800.0;

    buses[1].busNo = 2;
    strcpy(buses[1].name, "Royal Cruiser");
    strcpy(buses[1].source, "Hyderabad");
    strcpy(buses[1].destination, "Chennai");
    buses[1].baseFare = 950.0;

    buses[2].busNo = 3;
    strcpy(buses[2].name, "City Link");
    strcpy(buses[2].source, "Hyderabad");
    strcpy(buses[2].destination, "Vijayawada");
    buses[2].baseFare = 450.0;

    for (i = 0; i < MAX_BUSES; i++)
        for (j = 0; j < SEATS_PER_BUS; j++)
            buses[i].seats[j] = 0;
}

/* ---------- File handling ---------- */

void saveData(void)
{
    FILE *fp = fopen(DATA_FILE, "wb");
    if (fp == NULL) {
        printf("Warning: could not save data to file.\n");
        return;
    }
    fwrite(buses, sizeof(struct Bus), MAX_BUSES, fp);
    fwrite(&passengerCount, sizeof(int), 1, fp);
    fwrite(passengers, sizeof(struct Passenger), passengerCount, fp);
    fwrite(&ticketCount, sizeof(int), 1, fp);
    fwrite(tickets, sizeof(struct Ticket), ticketCount, fp);
    fwrite(&nextPassengerId, sizeof(int), 1, fp);
    fwrite(&nextTicketNo, sizeof(int), 1, fp);
    fclose(fp);
}

void loadData(void)
{
    FILE *fp = fopen(DATA_FILE, "rb");
    if (fp == NULL) {              /* first run: no file yet */
        initBuses();
        return;
    }
    fread(buses, sizeof(struct Bus), MAX_BUSES, fp);
    fread(&passengerCount, sizeof(int), 1, fp);
    fread(passengers, sizeof(struct Passenger), passengerCount, fp);
    fread(&ticketCount, sizeof(int), 1, fp);
    fread(tickets, sizeof(struct Ticket), ticketCount, fp);
    fread(&nextPassengerId, sizeof(int), 1, fp);
    fread(&nextTicketNo, sizeof(int), 1, fp);
    fclose(fp);
}

/* ---------- Search helpers ---------- */

int findPassenger(int id)
{
    int i;
    for (i = 0; i < passengerCount; i++)
        if (passengers[i].id == id)
            return i;
    return -1;
}

int findTicket(int ticketNo)
{
    int i;
    for (i = 0; i < ticketCount; i++)
        if (tickets[i].ticketNo == ticketNo)
            return i;
    return -1;
}

/* ---------- Show list of buses ---------- */

void displayBuses(void)
{
    int i;
    printf("\n%-5s %-18s %-12s %-12s %-8s\n",
           "Bus", "Name", "From", "To", "Fare");
    printf("---------------------------------------------------------\n");
    for (i = 0; i < MAX_BUSES; i++) {
        printf("%-5d %-18s %-12s %-12s Rs.%.0f\n",
               buses[i].busNo, buses[i].name, buses[i].source,
               buses[i].destination, buses[i].baseFare);
    }
}

/* ---------- 1. Passenger registration ---------- */

void registerPassenger(void)
{
    struct Passenger p;

    if (passengerCount >= MAX_PASSENGERS) {
        printf("Passenger list is full!\n");
        return;
    }

    p.id = nextPassengerId;

    printf("\n--- Passenger Registration ---\n");
    printf("Enter name   : ");
    scanf(" %29[^\n]", p.name);

    printf("Enter age    : ");
    p.age = readInt();
    while (p.age <= 0 || p.age > 120) {
        printf("Enter a valid age: ");
        p.age = readInt();
    }

    printf("Enter gender (M/F): ");
    scanf(" %c", &p.gender);

    printf("Enter phone  : ");
    scanf(" %14s", p.phone);

    passengers[passengerCount] = p;
    passengerCount++;
    nextPassengerId++;
    saveData();

    printf("\nRegistered successfully! Your Passenger ID is %d\n", p.id);
}

/* ---------- 4. Seat availability ---------- */

void checkSeatAvailability(void)
{
    int busNo, i, free = 0;

    displayBuses();
    printf("\nEnter bus number: ");
    busNo = readInt();

    if (busNo < 1 || busNo > MAX_BUSES) {
        printf("Invalid bus number!\n");
        return;
    }

    struct Bus *b = &buses[busNo - 1];

    printf("\nSeat status for %s (%s -> %s)\n", b->name, b->source, b->destination);
    printf("[ 1] = free     [XX] = booked\n\n");

    for (i = 0; i < SEATS_PER_BUS; i++) {
        if (b->seats[i] == 0) {
            printf("[%2d] ", i + 1);
            free++;
        } else {
            printf("[XX] ");
        }
        if ((i + 1) % 4 == 0)
            printf("\n");
    }
    printf("\nAvailable seats: %d / %d\n", free, SEATS_PER_BUS);
}

/* ---------- 5. Fare calculation ---------- */
/* Child (below 12) gets 50% off, Senior (60+) gets 30% off */

float calculateFare(float baseFare, int age)
{
    if (age < 12)
        return baseFare * 0.50;
    else if (age >= 60)
        return baseFare * 0.70;
    else
        return baseFare;
}

/* ---------- 2. Seat booking ---------- */

void bookSeat(void)
{
    int pid, pIndex, busNo, seatNo;
    float fare;
    struct Ticket t;

    if (ticketCount >= MAX_TICKETS) {
        printf("Ticket list is full!\n");
        return;
    }

    printf("\n--- Book a Seat ---\n");
    printf("Enter your Passenger ID: ");
    pid = readInt();
    pIndex = findPassenger(pid);
    if (pIndex == -1) {
        printf("Passenger not found! Please register first.\n");
        return;
    }

    displayBuses();
    printf("\nEnter bus number: ");
    busNo = readInt();
    if (busNo < 1 || busNo > MAX_BUSES) {
        printf("Invalid bus number!\n");
        return;
    }

    /* show seats of chosen bus */
    int i;
    printf("\nSeats ([XX] = booked):\n");
    for (i = 0; i < SEATS_PER_BUS; i++) {
        if (buses[busNo - 1].seats[i] == 0)
            printf("[%2d] ", i + 1);
        else
            printf("[XX] ");
        if ((i + 1) % 4 == 0)
            printf("\n");
    }

    printf("\nEnter seat number (1-%d): ", SEATS_PER_BUS);
    seatNo = readInt();
    if (seatNo < 1 || seatNo > SEATS_PER_BUS) {
        printf("Invalid seat number!\n");
        return;
    }
    if (buses[busNo - 1].seats[seatNo - 1] != 0) {
        printf("Sorry, seat %d is already booked!\n", seatNo);
        return;
    }

    fare = calculateFare(buses[busNo - 1].baseFare, passengers[pIndex].age);

    /* create the ticket */
    t.ticketNo    = nextTicketNo;
    t.passengerId = pid;
    t.busNo       = busNo;
    t.seatNo      = seatNo;
    t.fare        = fare;
    t.active      = 1;

    tickets[ticketCount] = t;
    ticketCount++;
    nextTicketNo++;
    buses[busNo - 1].seats[seatNo - 1] = t.ticketNo;
    saveData();

    /* print the ticket */
    printf("\n=========== TICKET ===========\n");
    printf("Ticket No  : %d\n", t.ticketNo);
    printf("Passenger  : %s (Age %d)\n", passengers[pIndex].name, passengers[pIndex].age);
    printf("Bus        : %s\n", buses[busNo - 1].name);
    printf("Route      : %s -> %s\n", buses[busNo - 1].source, buses[busNo - 1].destination);
    printf("Seat No    : %d\n", seatNo);
    printf("Fare       : Rs.%.2f\n", fare);
    printf("==============================\n");
}

/* ---------- 3. Ticket cancellation ---------- */
/* 10% of the fare is kept as cancellation charge */

void cancelTicket(void)
{
    int ticketNo, index;
    float refund;

    printf("\n--- Cancel Ticket ---\n");
    printf("Enter ticket number: ");
    ticketNo = readInt();

    index = findTicket(ticketNo);
    if (index == -1) {
        printf("Ticket not found!\n");
        return;
    }
    if (tickets[index].active == 0) {
        printf("This ticket is already cancelled.\n");
        return;
    }

    refund = tickets[index].fare * 0.90;

    tickets[index].active = 0;
    buses[tickets[index].busNo - 1].seats[tickets[index].seatNo - 1] = 0;
    saveData();

    printf("Ticket %d cancelled. Seat %d is free again.\n",
           ticketNo, tickets[index].seatNo);
    printf("Refund amount (after 10%% charge): Rs.%.2f\n", refund);
}

/* ---------- 6. Booking report ---------- */

void bookingReport(void)
{
    int i, pIndex, activeCount = 0, cancelledCount = 0;
    float total = 0;

    printf("\n================= BOOKING REPORT =================\n");

    if (ticketCount == 0) {
        printf("No bookings yet.\n");
        return;
    }

    printf("%-8s %-15s %-5s %-5s %-9s %-10s\n",
           "Ticket", "Passenger", "Bus", "Seat", "Fare", "Status");
    printf("--------------------------------------------------\n");

    for (i = 0; i < ticketCount; i++) {
        pIndex = findPassenger(tickets[i].passengerId);
        printf("%-8d %-15s %-5d %-5d %-9.2f %-10s\n",
               tickets[i].ticketNo,
               passengers[pIndex].name,
               tickets[i].busNo,
               tickets[i].seatNo,
               tickets[i].fare,
               tickets[i].active ? "Booked" : "Cancelled");

        if (tickets[i].active) {
            activeCount++;
            total += tickets[i].fare;
        } else {
            cancelledCount++;
        }
    }

    printf("--------------------------------------------------\n");
    printf("Total tickets booked    : %d\n", activeCount);
    printf("Total tickets cancelled : %d\n", cancelledCount);
    printf("Total collection        : Rs.%.2f\n", total);
}

/* ---------- Main menu ---------- */

int main(void)
{
    int choice;

    loadData();

    do {
        printf("\n****** BUS RESERVATION SYSTEM ******\n");
        printf("1. Passenger Registration\n");
        printf("2. Book Seat\n");
        printf("3. Cancel Ticket\n");
        printf("4. Check Seat Availability\n");
        printf("5. View Buses & Fares\n");
        printf("6. Booking Report\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        choice = readInt();

        switch (choice) {
            case 1: registerPassenger();      break;
            case 2: bookSeat();               break;
            case 3: cancelTicket();           break;
            case 4: checkSeatAvailability();  break;
            case 5: displayBuses();           break;
            case 6: bookingReport();          break;
            case 0: printf("Thank you! Goodbye.\n"); break;
            default: printf("Invalid choice! Try again.\n");
        }
    } while (choice != 0);

    return 0;
}
