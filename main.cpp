#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QStringList>
#include <QMessageBox>
#include <vector>
#include <queue>
#include <algorithm>

struct Process {
    int id;
    int burst;
    int arrival;
    int completion;
    int turnaround;
    int waiting;
};

class SchedulerApp : public QWidget {
    Q_OBJECT

public:
    SchedulerApp(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);

        // Поле для ввода данных
        QLabel* inputLabel = new QLabel("Введите данные процессов (ID Burst Arrival):");
        layout->addWidget(inputLabel);

        inputField = new QTextEdit();
        layout->addWidget(inputField);

        // Выбор алгоритма
        QLabel* algorithmLabel = new QLabel("Выберите алгоритм:");
        layout->addWidget(algorithmLabel);

        algorithmCombo = new QComboBox();
        algorithmCombo->addItems({"FCFS", "RR", "SJF", "PSJF"});
        layout->addWidget(algorithmCombo);

        // Ввод кванта времени
        QLabel* quantumLabel = new QLabel("Квант времени (для RR):");
        layout->addWidget(quantumLabel);

        quantumInput = new QLineEdit();
        quantumInput->setPlaceholderText("Введите квант времени");
        layout->addWidget(quantumInput);

        // Кнопка запуска
        QPushButton* runButton = new QPushButton("Запустить");
        layout->addWidget(runButton);

        // Таблица для вывода результатов
        resultTable = new QTableWidget();
        resultTable->setColumnCount(6);
        resultTable->setHorizontalHeaderLabels({"ID", "Burst", "Arrival", "Completion", "Turnaround", "Waiting"});
        resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        layout->addWidget(resultTable);

        // Подключение сигнала к слоту
        connect(runButton, &QPushButton::clicked, this, &SchedulerApp::runAlgorithm);
    }

private slots:
    void runAlgorithm() {
        QString inputData = inputField->toPlainText();
        QStringList processLines = inputData.split("\n", QString::SkipEmptyParts);

        std::vector<Process> processes;

        // Парсинг данных
        for (const QString& line : processLines) {
            QStringList parts = line.split(" ", QString::SkipEmptyParts);
            if (parts.size() != 3) {
                QMessageBox::warning(this, "Ошибка", "Некорректный ввод данных!");
                return;
            }
            Process p;
            p.id = parts[0].toInt();
            p.burst = parts[1].toInt();
            p.arrival = parts[2].toInt();
            processes.push_back(p);
        }

        QString algorithm = algorithmCombo->currentText();
        int quantum = quantumInput->text().toInt();
        std::vector<Process> results;

        if (algorithm == "FCFS") {
            results = fcfs(processes);
        } else if (algorithm == "RR") {
            if (quantum <= 0) {
                QMessageBox::warning(this, "Ошибка", "Введите корректный квант времени!");
                return;
            }
            results = rr(processes, quantum);
        } else if (algorithm == "SJF") {
            results = sjf(processes);
        } else if (algorithm == "PSJF") {
            results = psjf(processes);
        }

        displayResults(results);
    }

private:
    QTextEdit* inputField;
    QComboBox* algorithmCombo;
    QLineEdit* quantumInput;
    QTableWidget* resultTable;

    // Вывод результатов в таблицу
    void displayResults(const std::vector<Process>& processes) {
        resultTable->setRowCount(processes.size());
        for (size_t i = 0; i < processes.size(); ++i) {
            resultTable->setItem(i, 0, new QTableWidgetItem(QString::number(processes[i].id)));
            resultTable->setItem(i, 1, new QTableWidgetItem(QString::number(processes[i].burst)));
            resultTable->setItem(i, 2, new QTableWidgetItem(QString::number(processes[i].arrival)));
            resultTable->setItem(i, 3, new QTableWidgetItem(QString::number(processes[i].completion)));
            resultTable->setItem(i, 4, new QTableWidgetItem(QString::number(processes[i].turnaround)));
            resultTable->setItem(i, 5, new QTableWidgetItem(QString::number(processes[i].waiting)));
        }
    }

    // Расчёт метрик
    void calculateMetrics(std::vector<Process>& processes) {
        for (auto& p : processes) {
            p.turnaround = p.completion - p.arrival;
            p.waiting = p.turnaround - p.burst;
        }
    }

    // Алгоритмы планирования
    std::vector<Process> fcfs(std::vector<Process> processes) {
        int currentTime = 0;
        for (auto& p : processes) {
            currentTime = std::max(currentTime, p.arrival) + p.burst;
            p.completion = currentTime;
        }
        calculateMetrics(processes);
        return processes;
    }

    std::vector<Process> rr(std::vector<Process> processes, int quantum) {
        std::queue<Process> readyQueue;
        int currentTime = 0;
        std::vector<Process> results = processes;
        std::vector<bool> completed(processes.size(), false);

        while (true) {
            bool allDone = true;
            for (auto& p : results) {
                if (p.burst > 0) {
                    allDone = false;
                    if (p.arrival <= currentTime) {
                        int timeUsed = std::min(p.burst, quantum);
                        currentTime += timeUsed;
                        p.burst -= timeUsed;
                        if (p.burst == 0) {
                            p.completion = currentTime;
                        }
                    }
                }
            }
            if (allDone) break;
        }

        calculateMetrics(results);
        return results;
    }

    std::vector<Process> sjf(std::vector<Process> processes) {
        std::vector<Process> results;
        int currentTime = 0;

        while (!processes.empty()) {
            auto it = std::min_element(processes.begin(), processes.end(),
                                       [currentTime](const Process& a, const Process& b) {
                                           return a.arrival <= currentTime && a.burst < b.burst;
                                       });
            if (it->arrival > currentTime) {
                currentTime = it->arrival;
            }
            currentTime += it->burst;
            it->completion = currentTime;
            results.push_back(*it);
            processes.erase(it);
        }

        calculateMetrics(results);
        return results;
    }

    std::vector<Process> psjf(std::vector<Process> processes) {
        // Реализация PSJF
        return processes; // Упростим для примера
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    SchedulerApp window;
    window.setWindowTitle("Алгоритмы планирования процессов");
    window.resize(800, 600);
    window.show();
    return app.exec();
}
