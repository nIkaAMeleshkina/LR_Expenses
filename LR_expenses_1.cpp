#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iomanip>

struct Expense {
    std::string payer = "";
    double amount = 0.0;
    std::set<std::string> excluded;
};

void processTest(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream infile(inputFile);
    std::ofstream outfile(outputFile);
    if (!infile.is_open() || !outfile.is_open()) {
        std::cerr << "Нельзя открыть файл: " << inputFile << " или " << outputFile << std::endl;
        return;
    }

    std::string line;

    // Пропускаем пустые строки, ищем первую непустую с именами участников
    while (getline(infile, line)) {
        if (!line.empty()) break;
    }
    if (line.empty()) {
        std::cerr << "Ошибка. Входной файл не содержит данных: " << inputFile << std::endl;
        return;
    }

    std::istringstream iss(line);
    int numParticipants;
    iss >> numParticipants;

    std::vector<std::string> participants;
    std::string name;
    while (iss >> name) {
        participants.push_back(name);
    }

    std::map<std::string, double> spent; // Фактические расходы
    std::map<std::string, double> share; // Сколько должен был потратить
    for (const auto& p : participants) {
        spent[p] = 0.0;
        share[p] = 0.0;
    }

    // Считываем расходы
    std::vector<Expense> expenses;
    while (getline(infile, line)) {
        if (line.empty()) continue;  // пустые строки игнорируем

        Expense e;
        size_t slashPos = line.find('/');
        std::string left = line.substr(0, slashPos);
        std::string right = (slashPos != std::string::npos) ? line.substr(slashPos + 1) : "";

        std::istringstream ls(left);
        ls >> e.payer >> e.amount;
        spent[e.payer] += e.amount;

        // Исключаем тех, на кого не распространялись траты
        std::istringstream rs(right);
        std::string ex;
        while (std::getline(rs, ex, ',')) {
            if (!ex.empty()) {
                size_t start = ex.find_first_not_of(" ");
                size_t end = ex.find_last_not_of(" ");
                e.excluded.insert(ex.substr(start, end - start + 1));
            }
        }
        expenses.push_back(e);
    }

    // Расчёт доли участников
    for (const auto& e : expenses) {
        std::vector<std::string> included;
        if (e.excluded.empty()) {
            included = participants; // Исключённых нет - трата на всех участников
        }
        else {
            for (const auto& p : participants) {
                if (e.excluded.find(p) == e.excluded.end()) {
                    included.push_back(p);
                }
            }
        }
        double part = e.amount / included.size();
        for (const auto& p : included) {
            share[p] += part;
        }
    }

    // Сколько заплатил и норма
    for (const auto& p : participants) {
        outfile << p << " " << std::fixed << std::setprecision(2)
            << spent[p] << " " << share[p] << std::endl;
    }

    // Транзакции
    struct Person { std::string name; double balance; };
    std::vector<Person> creditors, debtors;
    for (const auto& p : participants) {
        double diff = spent[p] - share[p];
        if (diff > 0.01) creditors.push_back({ p, diff });
        else if (diff < -0.01) debtors.push_back({ p, -diff });
    }

    for (auto& d : debtors) {
        for (auto& c : creditors) {
            if (d.balance < 0.01) break;
            double payment = std::min(d.balance, c.balance);
            outfile << d.name << " " << std::fixed << std::setprecision(2)
                << payment << " " << c.name << std::endl;
            d.balance -= payment;
            c.balance -= payment;
        }
    }

    infile.close();
    outfile.close();
}

int main() {
    processTest("test1.txt", "output1.txt");
    processTest("test2.txt", "output2.txt");
    return 0;
}