import tkinter as tk
from tkinter import ttk, messagebox
import heapq

# Вспомогательные функции для расчетов
def calculate_metrics(processes, completion_times):
    metrics = []
    for i, process in enumerate(processes):
        T = completion_times[i]
        t = process[1]
        M = T - t
        R = t / T
        P = T / t
        metrics.append((T, M, R, P))
    return metrics

# Алгоритмы планирования
def fcfs(processes):
    current_time = 0
    completion_times = []
    for process in processes:
        current_time += process[1]
        completion_times.append(current_time)
    return completion_times

def rr(processes, quantum):
    queue = processes[:]
    completion_times = [0] * len(processes)
    time = 0
    while queue:
        process = queue.pop(0)
        if process[1] > quantum:
            time += quantum
            process[1] -= quantum
            queue.append(process)
        else:
            time += process[1]
            completion_times[process[0]] = time
    return completion_times

def sjf(processes):
    processes = sorted(processes, key=lambda x: x[1])
    current_time = 0
    completion_times = []
    for process in processes:
        current_time += process[1]
        completion_times.append(current_time)
    return completion_times

def psjf(processes):
    heap = []
    time = 0
    completion_times = [0] * len(processes)
    processes.sort(key=lambda x: x[2])
    index = 0
    while heap or index < len(processes):
        while index < len(processes) and processes[index][2] <= time:
            heapq.heappush(heap, (processes[index][1], processes[index][0]))
            index += 1
        if heap:
            burst, pid = heapq.heappop(heap)
            time += 1
            burst -= 1
            if burst > 0:
                heapq.heappush(heap, (burst, pid))
            else:
                completion_times[pid] = time
        else:
            time += 1
    return completion_times

# Интерфейс программы
class SchedulerApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Алгоритмы планирования процессов")
        self.geometry("600x400")

        # Ввод данных
        self.lbl_input = tk.Label(self, text="Введите данные процессов (Номер, Длительность, Время появления):")
        self.lbl_input.pack()

        self.text_input = tk.Text(self, height=10, width=70)
        self.text_input.pack()

        # Выбор алгоритма
        self.algo_label = tk.Label(self, text="Выберите алгоритм:")
        self.algo_label.pack()

        self.algo_var = tk.StringVar()
        self.algo_combo = ttk.Combobox(self, textvariable=self.algo_var, values=["FCFS", "RR", "SJF", "PSJF"])
        self.algo_combo.pack()

        self.quantum_label = tk.Label(self, text="Квант времени (для RR):")
        self.quantum_label.pack()

        self.quantum_entry = tk.Entry(self)
        self.quantum_entry.pack()

        # Кнопка для запуска
        self.btn_run = tk.Button(self, text="Запустить", command=self.run_algorithm)
        self.btn_run.pack()

        # Вывод результатов
        self.result_text = tk.Text(self, height=10, width=70, state=tk.DISABLED)
        self.result_text.pack()

    def run_algorithm(self):
        # Чтение данных
        try:
            processes_raw = self.text_input.get("1.0", tk.END).strip().split("\n")
            processes = [(int(i.split()[0]), int(i.split()[1]), int(i.split()[2])) for i in processes_raw]
        except Exception:
            messagebox.showerror("Ошибка ввода", "Проверьте формат данных!")
            return

        # Выбор алгоритма
        algorithm = self.algo_var.get()
        quantum = self.quantum_entry.get()
        if algorithm == "RR" and not quantum.isdigit():
            messagebox.showerror("Ошибка ввода", "Введите корректный квант времени!")
            return

        # Запуск алгоритма
        processes_copy = [list(process) for process in processes]  # Копия для работы
        if algorithm == "FCFS":
            completion_times = fcfs(processes_copy)
        elif algorithm == "RR":
            completion_times = rr(processes_copy, int(quantum))
        elif algorithm == "SJF":
            completion_times = sjf(processes_copy)
        elif algorithm == "PSJF":
            completion_times = psjf(processes_copy)
        else:
            messagebox.showerror("Ошибка", "Выберите алгоритм!")
            return

        # Расчет метрик
        metrics = calculate_metrics(processes, completion_times)

        # Вывод результатов
        self.result_text.config(state=tk.NORMAL)
        self.result_text.delete("1.0", tk.END)
        self.result_text.insert(tk.END, f"--- {algorithm} ---\n")
        for i, metric in enumerate(metrics):
            self.result_text.insert(tk.END, f"Процесс {i+1}: T={metric[0]}, M={metric[1]}, R={metric[2]:.2f}, P={metric[3]:.2f}\n")
        self.result_text.config(state=tk.DISABLED)

# Запуск приложения
if __name__ == "__main__":
    app = SchedulerApp()
    app.mainloop()
