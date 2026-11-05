#include "TotalHeader.h"

using namespace std;

// Глобальные переменные
LPProblem currentProblem;
SimplexResult currentResult;


// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ


wstring stringToWstring(const string& str) {
    // Конвертация std::string в std::wstring для совместимости с LibXL
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void waitForEnter() {
    cout << "Нажмите Enter для продолжения... ";
    cin.ignore(10000, '\n');
}

bool isValidFilename(const string& filename) {
    if (filename.empty()) return false;
    const string invalidChars = "<>:\"/\\|?*=";
    for (char c : filename) {
        if (invalidChars.find(c) != string::npos) return false;
    }
    return true;
}

bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

string inputFilename(const string& prompt) {
    while (true) {
        cout << prompt;
        string filename;
        getline(cin, filename);
        if (filename == "#") return "#";

        size_t start = filename.find_first_not_of(" \t");
        size_t end = filename.find_last_not_of(" \t");
        if (start == string::npos) {
            cout << "Ошибка: имя файла не может быть пустым!\nПопробуйте снова.\n\n";
            continue;
        }
        filename = filename.substr(start, end - start + 1);

        if (!isValidFilename(filename)) {
            cout << "Ошибка: имя файла содержит недопустимые символы!\nПопробуйте снова.\n\n";
            continue;
        }
        return filename;
    }
}

void show_menu(const string menu[], int num, int active) {
    const string act_mark = " >> ";
    const string inact_mark = "    ";
    for (int i = 0; i < num; i++) {
        if (i == active) cout << act_mark;
        else cout << inact_mark;
        cout << menu[i] << endl;
    }
}

int menu_main_action() {
    const int m = 6;
    const string menu[m] = {
        "Ввести задачу с клавиатуры",
        "Загрузить задачу из текстового файла",
        "Решить задачу симплекс-методом",
        "Сохранить отчет в Excel (.xlsx)",
        "Очистить текущую задачу",
        "Выход из программы"
    };
    int n = 0;
    unsigned char ch;
    do {
        system("cls");
        cout << "СИМПЛЕКС-МЕТОД: РЕШЕНИЕ ЗАДАЧ ЛИНЕЙНОГО ПРОГРАММИРОВАНИЯ\n\n";
        cout << "Выберите действие:\n";
        show_menu(menu, m, n);

        // Отображение статуса текущей задачи
        cout << "\n--- ТЕКУЩИЙ СТАТУС ---\n";
        if (currentProblem.numVars == 0) {
            cout << "Задача не определена.\n";
        }
        else {
            cout << "Переменных: " << currentProblem.numVars
                << ", Ограничений: " << currentProblem.numConstraints
                << ", Тип: " << (currentProblem.isMax ? "Максимизация" : "Минимизация") << "\n";
            if (currentResult.isSolved) {
                cout << "Статус: Решена. Оптимальное значение: " << currentResult.optimalValue << "\n";
            }
        }
        cout << "----------------------\n\n";

        ch = _getch();
        if (ch == 224) {
            ch = _getch();
            if (ch == 72) { n--; if (n < 0) n = m - 1; }
            if (ch == 80) { n++; if (n > m - 1) n = 0; }
        }
    } while (ch != 13);
    return n + 1;
}


// ВВОД ДАННЫХ


void clearProblem() {
    currentProblem = LPProblem();
    currentResult = SimplexResult();
    cout << "Данные задачи очищены.\n";
    waitForEnter();
}

void inputProblemFromKeyboard() {
    system("cls");
    cout << "ВВОД ЗАДАЧИ С КЛАВИАТУРЫ\n\n";
    clearProblem();

    cout << "Введите тип задачи (1 - максимизация, 2 - минимизация): ";
    int type;
    cin >> type;
    currentProblem.isMax = (type == 1);

    cout << "Введите количество переменных: ";
    cin >> currentProblem.numVars;
    cout << "Введите количество ограничений: ";
    cin >> currentProblem.numConstraints;

    currentProblem.c.resize(currentProblem.numVars);
    currentProblem.varNames.resize(currentProblem.numVars);
    currentProblem.A.resize(currentProblem.numConstraints, vector<double>(currentProblem.numVars, 0.0));
    currentProblem.b.resize(currentProblem.numConstraints, 0.0);
    currentProblem.constraintTypes.resize(currentProblem.numConstraints);

    cout << "\nВведите коэффициенты целевой функции (через пробел):\n";
    for (int i = 0; i < currentProblem.numVars; i++) {
        currentProblem.varNames[i] = "x" + to_string(i + 1);
        cout << "C" << (i + 1) << " = ";
        cin >> currentProblem.c[i];
    }

    cout << "\nВведите ограничения:\n";
    for (int i = 0; i < currentProblem.numConstraints; i++) {
        cout << "Ограничение " << (i + 1) << ":\n";
        for (int j = 0; j < currentProblem.numVars; j++) {
            cout << "  Коэффициент при " << currentProblem.varNames[j] << ": ";
            cin >> currentProblem.A[i][j];
        }
        cout << "  Знак (<= введите L, >= введите G, = введите E): ";
        cin >> currentProblem.constraintTypes[i];
        cout << "  Правая часть (b): ";
        cin >> currentProblem.b[i];
        cout << "\n";
    }

    // Очистка буфера ввода
    cin.ignore(10000, '\n');
    cout << "Задача успешно введена!\n";
    waitForEnter();
}

void inputProblemFromFile(const string& filename) {
    system("cls");
    cout << "ЗАГРУЗКА ЗАДАЧИ ИЗ ФАЙЛА\n\n";
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Ошибка: не удалось открыть файл " << filename << "\n";
        waitForEnter();
        return;
    }

    clearProblem();
    string line;

    // Чтение типа задачи
    getline(file, line);
    currentProblem.isMax = (line.find("max") != string::npos || line.find("Max") != string::npos);

    // Чтение размеров
    getline(file, line);
    stringstream ss(line);
    ss >> currentProblem.numVars >> currentProblem.numConstraints;

    currentProblem.c.resize(currentProblem.numVars);
    currentProblem.varNames.resize(currentProblem.numVars);
    currentProblem.A.resize(currentProblem.numConstraints, vector<double>(currentProblem.numVars, 0.0));
    currentProblem.b.resize(currentProblem.numConstraints, 0.0);
    currentProblem.constraintTypes.resize(currentProblem.numConstraints);

    for (int i = 0; i < currentProblem.numVars; i++) {
        currentProblem.varNames[i] = "x" + to_string(i + 1);
        file >> currentProblem.c[i];
    }

    for (int i = 0; i < currentProblem.numConstraints; i++) {
        for (int j = 0; j < currentProblem.numVars; j++) {
            file >> currentProblem.A[i][j];
        }
        file >> currentProblem.constraintTypes[i];
        file >> currentProblem.b[i];
    }

    file.close();
    cout << "Задача успешно загружена из файла!\n";
    waitForEnter();
}


// СИМПЛЕКС-МЕТОД (М-метод для универсальности)


SimplexResult solveSimplex(const LPProblem& prob) {
    SimplexResult res;
    res.isSolved = false;
    res.isUnbounded = false;
    res.isInfeasible = false;
    res.optimalValue = 0.0;

    // Если задача на минимизацию, преобразуем её в максимизацию (умножаем C на -1)
    // Это стандартный приём для унификации симплекс-таблицы
    vector<double> c_obj = prob.c;
    bool wasMin = !prob.isMax;
    if (wasMin) {
        for (int i = 0; i < prob.numVars; i++) c_obj[i] = -c_obj[i];
    }

    // Подсчет дополнительных переменных (балансовые, избыточные, искусственные)
    int numSlack = 0;
    int numSurplus = 0;
    int numArtificial = 0;
    for (char type : prob.constraintTypes) {
        if (type == 'L') numSlack++;
        else if (type == 'G') { numSurplus++; numArtificial++; }
        else if (type == 'E') numArtificial++;
    }

    int totalVars = prob.numVars + numSlack + numSurplus + numArtificial;
    int rows = prob.numConstraints + 1; // +1 для строки целевой функции (Z)
    int cols = totalVars + 1; // +1 для столбца правых частей (RHS)

    // Инициализация симплекс-таблицы нулями
    vector<vector<double>> tableau(rows, vector<double>(cols, 0.0));
    vector<string> headers;

    // Заголовки для исходных переменных
    for (int i = 0; i < prob.numVars; i++) headers.push_back(prob.varNames[i]);

    // Формирование таблицы ограничений
    int slackIdx = 0, surplusIdx = 0, artIdx = 0;
    vector<int> basicVars(prob.numConstraints, -1); // Индексы базисных переменных

    const double M = 1000000.0; // Большое число для М-метода

    for (int i = 0; i < prob.numConstraints; i++) {
        // Копируем исходные коэффициенты
        for (int j = 0; j < prob.numVars; j++) {
            tableau[i][j] = prob.A[i][j];
        }
        tableau[i][totalVars] = prob.b[i]; // Правая часть

        if (prob.constraintTypes[i] == 'L') {
            tableau[i][prob.numVars + slackIdx] = 1.0;
            basicVars[i] = prob.numVars + slackIdx;
            headers.push_back("s" + to_string(slackIdx + 1));
            slackIdx++;
        }
        else if (prob.constraintTypes[i] == 'G') {
            tableau[i][prob.numVars + numSlack + surplusIdx] = -1.0; // Избыточная
            tableau[i][prob.numVars + numSlack + numSurplus + artIdx] = 1.0; // Искусственная
            basicVars[i] = prob.numVars + numSlack + numSurplus + artIdx;
            headers.push_back("e" + to_string(surplusIdx + 1));
            headers.push_back("a" + to_string(artIdx + 1));
            surplusIdx++;
            artIdx++;
        }
        else if (prob.constraintTypes[i] == 'E') {
            tableau[i][prob.numVars + numSlack + numSurplus + artIdx] = 1.0; // Искусственная
            basicVars[i] = prob.numVars + numSlack + numSurplus + artIdx;
            headers.push_back("a" + to_string(artIdx + 1));
            artIdx++;
        }
    }

    // Формирование строки целевой функции (Z)
    for (int j = 0; j < prob.numVars; j++) {
        tableau[rows - 1][j] = -c_obj[j]; // Z - C*X = 0 => Z_row = -C
    }
    headers.push_back("RHS");

    // Устранение искусственных переменных из базиса в строке Z (стандартный шаг М-метода)
    // Если искусственная переменная в базисе, её коэффициент в Z-строке должен быть 0.
    for (int i = 0; i < prob.numConstraints; i++) {
        if (basicVars[i] >= prob.numVars + numSlack + numSurplus) { // Это искусственная переменная
            for (int j = 0; j < cols; j++) {
                tableau[rows - 1][j] += M * tableau[i][j];
            }
        }
    }

    // Сохраняем начальную таблицу
    res.tableaus.push_back(tableau);
    res.tableauHeaders.push_back(headers);

    int iter = 0;
    const int MAX_ITER = 100;

    while (iter < MAX_ITER) {
        // 1. Поиск разрешающего столбца (наиболее отрицательный элемент в Z-строке)
        int pivotCol = -1;
        double minVal = 0.0;
        for (int j = 0; j < totalVars; j++) {
            if (tableau[rows - 1][j] < minVal) {
                minVal = tableau[rows - 1][j];
                pivotCol = j;
            }
        }

        // Если нет отрицательных элементов, решение оптимально
        if (pivotCol == -1) {
            // Проверка на наличие искусственных переменных в базисе (недопустимость)
            bool artificialInBasis = false;
            for (int i = 0; i < prob.numConstraints; i++) {
                if (basicVars[i] >= prob.numVars + numSlack + numSurplus && abs(tableau[i][totalVars]) > 1e-6) {
                    artificialInBasis = true;
                    break;
                }
            }
            if (artificialInBasis) {
                res.isInfeasible = true;
            }
            else {
                res.isSolved = true;
            }
            break;
        }

        // 2. Поиск разрешающей строки (минимальное положительное отношение RHS к элементу столбца)
        int pivotRow = -1;
        double minRatio = 1e30;
        for (int i = 0; i < prob.numConstraints; i++) {
            if (tableau[i][pivotCol] > 1e-6) {
                double ratio = tableau[i][totalVars] / tableau[i][pivotCol];
                if (ratio < minRatio) {
                    minRatio = ratio;
                    pivotRow = i;
                }
            }
        }

        // Если все элементы в разрешающем столбце <= 0, функция неограниченна
        if (pivotRow == -1) {
            res.isUnbounded = true;
            break;
        }

        // Сохраняем координаты разрешающего элемента
        res.pivotHistory.push_back({ pivotRow, pivotCol });

        // 3. Приведение таблицы к каноническому виду (метод Гаусса)
        double pivotElement = tableau[pivotRow][pivotCol];

        // Делим разрешающую строку на разрешающий элемент
        for (int j = 0; j < cols; j++) {
            tableau[pivotRow][j] /= pivotElement;
        }
        basicVars[pivotRow] = pivotCol; // Обновляем базисную переменную

        // Обнуляем остальные элементы в разрешающем столбце
        for (int i = 0; i < rows; i++) {
            if (i != pivotRow) {
                double factor = tableau[i][pivotCol];
                for (int j = 0; j < cols; j++) {
                    tableau[i][j] -= factor * tableau[pivotRow][j];
                }
            }
        }

        // Сохраняем итерацию
        res.tableaus.push_back(tableau);
        res.tableauHeaders.push_back(headers);
        iter++;
    }

    // Если решение найдено, извлекаем значения переменных
    if (res.isSolved) {
        res.solution.assign(prob.numVars, 0.0);
        for (int i = 0; i < prob.numConstraints; i++) {
            if (basicVars[i] < prob.numVars) {
                res.solution[basicVars[i]] = tableau[i][totalVars];
            }
        }
        res.optimalValue = tableau[rows - 1][totalVars];
        // Если исходная задача была на минимизацию, возвращаем знак
        if (wasMin) {
            res.optimalValue = -res.optimalValue;
        }
    }

    return res;
}

void printTableau(const vector<vector<double>>& tableau, const vector<string>& headers, int pivotRow, int pivotCol) {
    cout << left << setw(10) << "Базис";
    for (const string& h : headers) {
        cout << setw(10) << h;
    }
    cout << endl;
    cout << string((headers.size() + 1) * 10, '-') << endl;

    for (size_t i = 0; i < tableau.size(); i++) {
        if (i < tableau.size() - 1) {
            cout << setw(10) << ("X" + to_string(i + 1)); // Упрощенное имя базисной переменной
        }
        else {
            cout << setw(10) << "Z";
        }

        for (size_t j = 0; j < tableau[i].size(); j++) {
            // Выделение разрешающего элемента цветом (в консоли - скобками)
            if ((int)i == pivotRow && (int)j == pivotCol) {
                cout << "[" << setw(8) << fixed << setprecision(2) << tableau[i][j] << "] ";
            }
            else {
                cout << setw(10) << fixed << setprecision(2) << tableau[i][j];
            }
        }
        cout << endl;
    }
    cout << endl;
}


// ЭКСПОРТ В EXCEL (LibXL)


bool exportToExcel(const LPProblem& prob, const SimplexResult& res, const string& filename) {
    // Инициализация книги Excel
    libxl::Book* book = xlCreateXMLBook();

    // Лицензионный ключ (для учебной версии можно оставить пустым)
    book->setKey(L"Your Name", L"Your License Key");

    if (!book) {
        cout << "Ошибка инициализации LibXL.\n";
        return false;
    }

    // Форматы для Excel
    libxl::Format* titleFormat = book->addFormat();
    libxl::Font* titleFont = book->addFont();
    titleFont->setBold(true);
    titleFont->setSize(14);
    titleFormat->setFont(titleFont);

    libxl::Format* headerFormat = book->addFormat();
    libxl::Font* headerFont = book->addFont();
    headerFont->setBold(true);
    headerFormat->setFont(headerFont);
    headerFormat->setFillPattern(libxl::FILLPATTERN_SOLID);
    headerFormat->setPatternForegroundColor(libxl::COLOR_BLACK);

    libxl::Format* pivotFormat = book->addFormat();
    libxl::Font* pivotFont = book->addFont();
    pivotFont->setBold(true);
    pivotFormat->setFont(pivotFont);
    pivotFormat->setFillPattern(libxl::FILLPATTERN_SOLID);
    pivotFormat->setPatternForegroundColor(libxl::COLOR_YELLOW);

    libxl::Format* numFormat = book->addFormat();
    numFormat->setNumFormat(libxl::NUMFORMAT_NUMBER);

    
    // ЛИСТ 1: Титульный лист и постановка задачи
    /
    libxl::Sheet* sheet1 = book->addSheet(L"Постановка задачи");
    if (!sheet1) return false;

    sheet1->writeStr(0, 0, L"Отчет по решению задачи линейного программирования", titleFormat);
    sheet1->writeStr(2, 0, L"1. Постановка задачи:", 0);

    wstring taskType = prob.isMax ? L"максимум" : L"минимум";
    wstring targetLine = L"Найти " + taskType + L" целевой функции:";
    sheet1->writeStr(3, 0, targetLine.c_str(), 0);

    wstring objFunc = L"Z = ";
    for (int i = 0; i < prob.numVars; i++) {
        objFunc += to_wstring(prob.c[i]) + L"*" + stringToWstring(prob.varNames[i]);
        if (i < prob.numVars - 1 && prob.c[i + 1] >= 0) objFunc += L" + ";
    }
    sheet1->writeStr(4, 1, objFunc.c_str(), 0);

    sheet1->writeStr(6, 0, L"2. Исходные данные (ограничения):", 0);
    for (int i = 0; i < prob.numConstraints; i++) {
        wstring constraint = L"";
        for (int j = 0; j < prob.numVars; j++) {
            constraint += to_wstring(prob.A[i][j]) + L"*" + stringToWstring(prob.varNames[j]);
            if (j < prob.numVars - 1) constraint += L" + ";
        }
        wstring sign = (prob.constraintTypes[i] == 'L') ? L" <=" :
            (prob.constraintTypes[i] == 'G') ? L" >=" : L" =";
        constraint += sign + L" " + to_wstring(prob.b[i]);
        sheet1->writeStr(7 + i, 1, constraint.c_str(), 0);
    }

    wstring typeStr = prob.isMax ? L"Максимизация (Max)" : L"Минимизация (Min)";
    wstring typeLine = L"3. Тип задачи: " + typeStr;
    sheet1->writeStr(7 + prob.numConstraints + 2, 0, typeLine.c_str(), 0);

    sheet1->setCol(0, 0, 30);
    sheet1->setCol(1, 1, 60);

    
    // ЛИСТ 2: Процесс решения (Симплекс-таблицы)
   
    libxl::Sheet* sheet2 = book->addSheet(L"Процесс решения");
    int currentRow = 0;

    for (size_t iter = 0; iter < res.tableaus.size(); iter++) {
        wstring iterTitle = L"Итерация " + to_wstring(iter);
        sheet2->writeStr(currentRow, 0, iterTitle.c_str(), titleFormat);
        currentRow += 2;

        const auto& tableau = res.tableaus[iter];
        const auto& headers = res.tableauHeaders[iter];

        for (size_t j = 0; j < headers.size(); j++) {
            wstring headerStr = stringToWstring(headers[j]);
            sheet2->writeStr(currentRow, (int)j, headerStr.c_str(), headerFormat);
        }
        currentRow++;

        int pivotR = -1, pivotC = -1;
        if (iter > 0 && iter - 1 < res.pivotHistory.size()) {
            pivotR = res.pivotHistory[iter - 1].first;
            pivotC = res.pivotHistory[iter - 1].second;
        }

        for (size_t i = 0; i < tableau.size(); i++) {
            for (size_t j = 0; j < tableau[i].size(); j++) {
                if ((int)i == pivotR && (int)j == pivotC) {
                    sheet2->writeNum(currentRow + (int)i, (int)j, tableau[i][j], pivotFormat);
                }
                else {
                    sheet2->writeNum(currentRow + (int)i, (int)j, tableau[i][j], numFormat);
                }
            }
        }
        currentRow += (int)tableau.size() + 3;
    }

   
    // ЛИСТ 3: Результаты
   
    libxl::Sheet* sheet3 = book->addSheet(L"Результаты");
    sheet3->writeStr(0, 0, L"ИТОГОВЫЕ РЕЗУЛЬТАТЫ", titleFormat);

    if (res.isSolved) {
        sheet3->writeStr(2, 0, L"Статус: Оптимальное решение найдено.", 0);
        sheet3->writeStr(4, 0, L"Оптимальное значение целевой функции (Z):", 0);
        sheet3->writeNum(4, 2, res.optimalValue, numFormat);

        sheet3->writeStr(6, 0, L"Значения переменных в оптимальной точке:", 0);
        for (int i = 0; i < prob.numVars; i++) {
            wstring varLine = stringToWstring(prob.varNames[i]) + L" =";
            sheet3->writeStr(7 + i, 0, varLine.c_str(), 0);
            sheet3->writeNum(7 + i, 2, res.solution[i], numFormat);
        }
    }
    else if (res.isUnbounded) {
        sheet3->writeStr(2, 0, L"Статус: Целевая функция неограниченна (решения нет).", 0);
    }
    else if (res.isInfeasible) {
        sheet3->writeStr(2, 0, L"Статус: Область допустимых решений пуста.", 0);
    }
    else {
        sheet3->writeStr(2, 0, L"Статус: Не удалось найти решение.", 0);
    }

    sheet3->setCol(0, 0, 40);
    sheet3->setCol(2, 2, 15);

    // Сохранение файла
    wstring wFilename = stringToWstring(filename);
    bool saveResult = book->save(wFilename.c_str());
    book->release();

    return saveResult;
}