#define _CRT_SECURE_NO_WARNINGS
#include <iostream>   
#include <string>        
#include <windows.h>      
#include "TotalHeader.h"  

using namespace std;

int main() {
    // Настройка локали для корректного отображения русского языка в консоли
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    bool exitProgram = false;

    while (!exitProgram) {
        int choice = menu_main_action();

        switch (choice) {
        case 1: // Ввести задачу с клавиатуры
            inputProblemFromKeyboard();
            break;

        case 2: // Загрузить задачу из файла
        {
            string prompt = "Введите имя файла для загрузки (или # для возврата): ";
            string filename = inputFilename(prompt);
            if (filename == "#") break;

            if (!fileExists(filename)) {
                cout << "Ошибка: файл \"" << filename << "\" не найден!\n";
                waitForEnter();
            }
            else {
                inputProblemFromFile(filename);
            }
        }
        break;

        case 3: // Решить задачу симплекс-методом
        {
            if (currentProblem.numVars == 0) {
                cout << "Ошибка: Сначала введите или загрузите задачу!\n";
                waitForEnter();
                break;
            }
            system("cls");
            cout << "РЕШЕНИЕ ЗАДАЧИ СИМПЛЕКС-МЕТОДОМ\n\n";
            currentResult = solveSimplex(currentProblem);

            // Вывод всех итераций в консоль
            for (size_t i = 0; i < currentResult.tableaus.size(); i++) {
                cout << "Итерация " << i << " \n";
                int pR = -1, pC = -1;
                if (i > 0 && i - 1 < currentResult.pivotHistory.size()) {
                    pR = currentResult.pivotHistory[i - 1].first;
                    pC = currentResult.pivotHistory[i - 1].second;
                }
                printTableau(currentResult.tableaus[i], currentResult.tableauHeaders[i], pR, pC);
                waitForEnter();
                system("cls");
            }

            cout << "ИТОГОВЫЙ РЕЗУЛЬТАТ\n";
            if (currentResult.isSolved) {
                cout << "Оптимальное решение найдено!\n";
                cout << "Z = " << currentResult.optimalValue << "\n";
                for (int i = 0; i < currentProblem.numVars; i++) {
                    cout << currentProblem.varNames[i] << " = " << currentResult.solution[i] << "\n";
                }
            }
            else if (currentResult.isUnbounded) {
                cout << "Целевая функция неограниченна (решения нет).\n";
            }
            else if (currentResult.isInfeasible) {
                cout << "Область допустимых решений пуста (ограничения противоречивы).\n";
            }
            waitForEnter();
        }
        break;

        case 4: // Сохранить отчет в Excel
        {
            if (!currentResult.isSolved && !currentResult.isUnbounded && !currentResult.isInfeasible) {
                cout << "Ошибка: Сначала решите задачу!\n";
                waitForEnter();
                break;
            }
            string prompt = "Введите имя файла для сохранения отчета (например, otchet.xlsx, или # для отмены): ";
            string filename = inputFilename(prompt);
            if (filename == "#") break;

            // Принудительное добавление расширения .xlsx, если его нет
            if (filename.length() < 5 || filename.substr(filename.length() - 5) != ".xlsx") {
                filename += ".xlsx";
            }

            cout << "Формирование отчета...\n";
            if (exportToExcel(currentProblem, currentResult, filename)) {
                cout << "Отчет успешно сохранен в файл: " << filename << "\n";
            }
            else {
                cout << "Ошибка при сохранении файла. Проверьте права доступа и подключение LibXL.\n";
            }
            waitForEnter();
        }
        break;

        case 5: // Очистить текущую задачу
            clearProblem();
            break;

        case 6: // Выход из программы
            exitProgram = true;
            break;

        default:
            exitProgram = true;
            break;
        }
    }

    system("cls");
    cout << "Программа завершена. До свидания!\n";
    return 0;
}