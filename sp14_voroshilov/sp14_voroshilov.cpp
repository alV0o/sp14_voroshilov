// sp14_voroshilov.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <Windows.h>
#include <random>

#define MAX_CLIENTS 20
#define CLUB_CAPACITY 4

struct ClientRecord {
    DWORD threadId; //id потока
    DWORD arriveTick; //время прихода посетителя
    DWORD startTick; //время начала обслуживания
    DWORD endTick; //время завершения обслуживания
    BOOL served; // был ли обслужен
    BOOL timeout; //ушел ли по таймауту
};

struct ClubState {
    ClientRecord clients[MAX_CLIENTS]; // Информация о посетителях
    LONG currentVisitors; // Текущее число занятых мест
    LONG maxVisitors; // Максимум одновременно занятых мест
    LONG servedCount; // Количество обслуженных посетителей
    LONG timeoutCount; // Количество ушедших по таймауту
};

CRITICAL_SECTION cs;

DWORD WINAPI ClientThread(LPVOID _client) {
    std::random_device rd;
    std::mt19937 gen(rd());

    ClientRecord* client = (ClientRecord*)_client;
    client->arriveTick = GetTickCount64();

    HANDLE hSemaphore;
    hSemaphore = OpenSemaphore(SYNCHRONIZE, TRUE, (LPCWSTR)"CountUsingPCSemaphore");
    if (hSemaphore == NULL)
        return GetLastError();


    if (WaitForSingleObject(hSemaphore, 3000) == WAIT_OBJECT_0) {
        client->startTick = GetTickCount64();
        int timeForJob = gen() % 5 + 1;
        if (timeForJob == 1) timeForJob++;
        Sleep(timeForJob*100);
        ReleaseSemaphore(hSemaphore, 1, NULL);
        client->endTick = GetTickCount64();
        client->served = true;
        client->timeout = false;
        EnterCriticalSection(&cs);
        std::cout << "join " << client->threadId << " - " << client->startTick << std::endl;
        std::cout << "leave " << client->threadId << " - " << client->endTick << std::endl;
        LeaveCriticalSection(&cs);
    }
    else {
        client->served = false;
        client->timeout = true;
        EnterCriticalSection(&cs);
        std::cout << "timeout " << client->threadId << std::endl;
        LeaveCriticalSection(&cs);
    }
    return 0;
}

int main()
{
    HANDLE hSemaphore;
    hSemaphore = CreateSemaphore(NULL, CLUB_CAPACITY, CLUB_CAPACITY, (LPCWSTR)"CountUsingPCSemaphore");
    if (hSemaphore == NULL)
        return GetLastError();

    HANDLE hThreads[20];
    DWORD IDThreads[20];

    ClientRecord clients[20];

    InitializeCriticalSection(&cs);

    for (int i = 0; i < 20; i++) {
        clients[i].threadId = i;
        hThreads[i] = CreateThread(NULL, 0, ClientThread, &clients[i], NULL, &IDThreads[i]);
        if (hThreads[i] == NULL)
            return GetLastError();
    }
    WaitForMultipleObjects(20, hThreads, TRUE, INFINITE);
    DeleteCriticalSection(&cs);
    for (int i = 0; i < 20; i++) {
        CloseHandle(hThreads[i]);
    }

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
