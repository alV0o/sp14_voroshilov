// sp14_voroshilov.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <Windows.h>
#include <random>

#define MAX_CLIENTS 20
#define CLUB_CAPACITY 4

struct ClientRecord {
    DWORD threadId = 0; //id потока
    DWORD arriveTick; //время прихода посетителя
    DWORD startTick = 0; //время начала обслуживания
    DWORD endTick = 0; //время завершения обслуживания
    BOOL served = false; // был ли обслужен
    BOOL timeout = false; //ушел ли по таймауту
};

struct ClubState {
    ClientRecord* clients[MAX_CLIENTS] = {}; // Информация о посетителях
    LONG currentVisitors = 0; // Текущее число занятых мест
    LONG maxVisitors = 0; // Максимум одновременно занятых мест
    LONG servedCount = 0; // Количество обслуженных посетителей
    LONG timeoutCount = 0; // Количество ушедших по таймауту
};

ClubState club;

DWORD WINAPI ClientThread(LPVOID _client) {
    std::random_device rd;
    std::mt19937 gen(rd());

    ClientRecord* client = (ClientRecord*)_client;
    client->arriveTick = GetTickCount64();

    club.clients[client->threadId - 1] = client;

    HANDLE hSemaphore;
    hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, L"CountUsingPCSemaphore");
    if (hSemaphore == NULL)
        return GetLastError();

    if (WaitForSingleObject(hSemaphore, 3000) == WAIT_OBJECT_0) {
        client->startTick = GetTickCount64();
        int timeForJob = gen() % 5 + 1;
        if (timeForJob == 1) timeForJob++;
        Sleep(timeForJob*1000);
        ReleaseSemaphore(hSemaphore, 1, NULL);
        client->endTick = GetTickCount64();
        client->served = true;
        client->timeout = false;
    }
    else {
        client->served = false;
        client->timeout = true;
    }

    CloseHandle(hSemaphore);
    return 0;
}

DWORD WINAPI ObserverThread() {
    while (club.timeoutCount+club.servedCount < MAX_CLIENTS) {
        Sleep(200);
        club.servedCount = 0;
        club.timeoutCount = 0;
        club.currentVisitors = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (club.clients[i]->threadId > 0) {
                if (club.clients[i]->served)
                    club.servedCount++;
                else if (club.clients[i]->timeout)
                    club.timeoutCount++;
                else if (club.clients[i]->startTick > 0 && club.clients[i]->endTick == 0)
                    club.currentVisitors++;
            }
        }
        if (club.maxVisitors < club.currentVisitors) club.maxVisitors = club.currentVisitors;
        system("cls");
        std::cout << "count taken seats - " << club.currentVisitors<< std::endl;
        std::cout << "count served clients - " << club.servedCount << std::endl;
        std::cout << "count timeout clients - " << club.timeoutCount << std::endl;
    }

    std::cout << "max taken seats - " << club.maxVisitors << std::endl;

    double avgTimeWaiting = 0;
    double avgServiceTime= 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (club.clients[i]->startTick > 0) {
            avgTimeWaiting += (club.clients[i]->startTick - club.clients[i]->arriveTick) / 1000.0;
            avgServiceTime += (club.clients[i]->endTick - club.clients[i]->startTick) / 1000.0;
        }
        else if (club.clients[i]->timeout) {
            std::cout << "timeout client - " << club.clients[i]->threadId << std::endl;
        }
    }
    
    std::cout << "avg waiting time - " << avgTimeWaiting/(double)club.servedCount<< std::endl;
    std::cout << "avg service time - " << avgServiceTime / (double)club.servedCount << std::endl;


    return 0;
}

int main()
{
    HANDLE hSemaphore;
    hSemaphore = CreateSemaphore(NULL, CLUB_CAPACITY, CLUB_CAPACITY, L"CountUsingPCSemaphore");
    if (hSemaphore == NULL)
        return GetLastError();

    HANDLE hThreads[MAX_CLIENTS];
    DWORD IDThreads[MAX_CLIENTS];

    HANDLE hObsThread;
    DWORD IDObsThread;

    ClientRecord clients[MAX_CLIENTS];


    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].threadId = i+1;
        hThreads[i] = CreateThread(NULL, 0, ClientThread, &clients[i], NULL, &IDThreads[i]);
        if (hThreads[i] == NULL)
            return GetLastError();
        if (i >= 0 && i <= 7) SetThreadPriority(hThreads[i], THREAD_PRIORITY_NORMAL);
        else if (i >= 8 && i <= 15) SetThreadPriority(hThreads[i], THREAD_PRIORITY_BELOW_NORMAL);
        else if (i >= 16 && i <= 19) SetThreadPriority(hThreads[i], THREAD_PRIORITY_HIGHEST);
    }

    hObsThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ObserverThread, NULL, NULL, &IDObsThread);
    if (hObsThread == NULL)
        return GetLastError();

    SetThreadPriority(hObsThread, THREAD_PRIORITY_LOWEST);

    WaitForMultipleObjects(MAX_CLIENTS, hThreads, TRUE, INFINITE);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        CloseHandle(hThreads[i]);
    }
    WaitForSingleObject(hObsThread, INFINITE);
    CloseHandle(hObsThread);
    CloseHandle(hSemaphore);

}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
