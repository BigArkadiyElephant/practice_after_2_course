#include <iostream>
#include <windows.h>
#include <string>

#define TextBufferSize  256

using namespace std;

HANDLE connectedPort;
int selectedPort = 7;           //Необходимо выбрать COM-порт
bool isConnected = false;
int targetBaudRate = 57600;

int serialBegin(int BaudRate, int Comport);     //функция, производящая подключение COM-порта
void connectRequest(void);                      //функция, которая регулирует подключение и отключение COM-порт
void SerialWrite(void);                         //функция, передающая данные на COM-порт

int main()
{
    connectRequest();       //подключение
    SerialWrite();          //работа с COM-портом - отправка данных
    connectRequest();       //отключение
    return 1;
}


int serialBegin(int BaudRate, int Comport) 
{
    //CloseHandle(connectedPort);

    connectedPort = CreateFileA(                        //подключение COM-порта
        ("\\\\.\\COM" + to_string(Comport)).c_str(),    //указание COM-порта
        GENERIC_READ | GENERIC_WRITE,                   //параметры доступа
        0,
        NULL,
        OPEN_EXISTING,                                  //режим открытия
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (connectedPort == INVALID_HANDLE_VALUE)  return -4;  //отсутствие устройств
    DCB SerialParams;                                       //параметры
    SerialParams.DCBlength = sizeof(SerialParams);
    if (!GetCommState(connectedPort, &SerialParams)) 
    return -3;                                              //начальные настройки не полученны

    SerialParams.BaudRate = BaudRate;
    SerialParams.ByteSize = 8;
    SerialParams.StopBits = ONESTOPBIT;
    SerialParams.Parity = NOPARITY;
    if (!SetCommState(connectedPort, &SerialParams)) return -2; //не удалось задать новые параметры

    COMMTIMEOUTS SerialTimeouts;
    SerialTimeouts.ReadIntervalTimeout = 1;
    SerialTimeouts.ReadTotalTimeoutConstant = 1;
    SerialTimeouts.ReadTotalTimeoutMultiplier = 1;
    SerialTimeouts.WriteTotalTimeoutConstant = 1;
    SerialTimeouts.WriteTotalTimeoutMultiplier = 1;
    if (!SetCommTimeouts(connectedPort, &SerialTimeouts)) return -1; //не удалось задать длительность ожидания

    return 0;
}

void connectRequest(void) {

    if (isConnected) {
        CloseHandle(connectedPort);
        isConnected = false;
        cout << "disconnected\n";
        return;
    }

    switch (serialBegin(targetBaudRate, selectedPort))
    {
    case -4: cout << "No port\n";
    case -3: cout << "GetState error\n";
    case -2: cout << "SetState error\n";
    case -1: cout << "SetTimeouts error\n";
    case 0: 
        cout << "connected\n";
        isConnected = true;
        return;
    }

    CloseHandle(connectedPort);
}

void addEnld(char* str)         //добавление переноса строки для того, чтобы плата считала данные
{
    int quant = strlen(str);
    str[quant] = '\n';
    str[quant + 1] = 0;
}

void SerialWrite() {
    if (!isConnected) return;           //проверка подключения
    DWORD BytesIterated;

    do
    {
        char* Buffer = new char[256];
        cin.getline(Buffer, 256); 
        addEnld(Buffer);
        WriteFile(connectedPort, Buffer, strlen(Buffer), &BytesIterated, NULL);     //отправка данных на плату
        delete[] Buffer;

        cout << "\n--------------\n";
    } while (1);
}