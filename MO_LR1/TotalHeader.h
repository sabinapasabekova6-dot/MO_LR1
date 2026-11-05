#pragma once

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>    
#include <string>      
#include <windows.h>    
#include <fstream>           
#include <sstream>     
#include <conio.h>    
#include <vector>
#include <iomanip>
#include <cmath>
#include "libxl.h"     // Подключение библиотеки LibXL для работы с Excel

using namespace std;

// ============================================================================
// СТРУКТУРЫ ДАННЫХ
// ============================================================================

// Структура для хранения исходной задачи линейного программирования
struct LPProblem {
    bool isMax;                   // true = максимизация, false = минимизация
    int numVars;                  // Количество исходных переменных
    int numConstraints;           // Количество ограничений
    vector<double> c;             // Коэффициенты целевой функции
    vector<vector<double>> A;     // Матрица коэффициентов ограничений
    vector<double> b;             // Вектор правых частей ограничений
    vector<char> constraintTypes; // Типы ограничений: 'L' (<=), 'G' (>=), 'E' (=)
    vector<string> varNames;      // Имена переменных для вывода (x1, x2, ...)
};

// Структура для хранения результата решения и истории симплекс-таблиц
struct SimplexResult {
    bool isSolved;                // true, если решение найдено
    bool isUnbounded;             // true, если целевая функция неограниченна
    bool isInfeasible;            // true, если решений нет (область пуста)
    double optimalValue;          // Оптимальное значение целевой функции
    vector<double> solution;      // Значения переменных в оптимальной точке
    vector<vector<vector<double>>> tableaus; // История всех симплекс-таблиц
    vector<vector<string>> tableauHeaders;   // Заголовки для таблиц (имена переменных)
    vector<pair<int, int>> pivotHistory;     // Координаты разрешающих элементов (row, col) для каждой итерации
};

// ============================================================================
// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
// ============================================================================
extern LPProblem currentProblem;  // Текущая задача
extern SimplexResult currentResult; // Результат решения

// ============================================================================
// ФУНКЦИИ МЕНЮ И ВВОДА
// ============================================================================
void show_menu(const string menu[], int num, int active);
int menu_main_action();
string inputFilename(const string& prompt);
bool isValidFilename(const string& filename);
bool fileExists(const string& filename);
void waitForEnter();

// ============================================================================
// ФУНКЦИИ РАБОТЫ С ЗАДАЧЕЙ ЛП
// ============================================================================
void inputProblemFromKeyboard();
void inputProblemFromFile(const string& filename);
void clearProblem();

// ============================================================================
// ФУНКЦИИ РЕШЕНИЯ (СИМПЛЕКС-МЕТОД)
// ============================================================================
// Основной метод решения с использованием М-метода для обработки любых ограничений
SimplexResult solveSimplex(const LPProblem& prob);
void printTableau(const vector<vector<double>>& tableau, const vector<string>& headers, int pivotRow, int pivotCol);

// ============================================================================
// ФУНКЦИИ ЭКСПОРТА В EXCEL (LibXL)
// ============================================================================
// Экспортирует постановку, процесс решения и результаты в файл .xlsx
bool exportToExcel(const LPProblem& prob, const SimplexResult& res, const string& filename);
wstring stringToWstring(const string& str); // Вспомогательная функция для конвертации в широкие символы (требование LibXL)
