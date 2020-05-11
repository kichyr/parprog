#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <omp.h>

// Mac OS запуск программы
// clang -Xpreprocessor -fopenmp -lomp calc.c 

// Инициализируем функцию интеграл которой будем считать
double F(double x) {
    if(x > 2) { // Проверяем значения на пригодность
        return 0;
    }
    return sqrt(4 - x*x);
}
// Задаем границы интегрирования
double a = 0, b = 2;

// Реализация формулы Котеса
double Integrate(size_t left_index, size_t right_index, double h) {
    double I = (F(right_index * h) + F(left_index * h)) / 2;
    for(size_t i = left_index + 1; i < right_index; i++) {
        I += F(i * h);
    }
    return I * h;
}

int main(int argc, char **argv) {
    // Задаем количество разбиений отрезка
    int N = 100000; // default value
    // Задаем количество процессов OpenMP
    int num_of_threads = 1; // default value

    //Присваиваем введенные значения
    if (argc > 1) {
        N = atoll(argv[1]);
		if (argc > 2) {
            num_of_threads = atoi(argv[2]);
        }
    }
    // Определяем мелкость разбиения отрезков
    double delta = (b - a) / N;
    double result = 0.0;
    
    // Создаем и инициализируем lock
    omp_lock_t lock;
    omp_init_lock(&lock);

        // Параллельная секция
        #pragma omp parallel num_threads(num_of_threads)
        {
            // Устанавливаем ранг процесса
            int rank = omp_get_thread_num();
            // Передаем каждому процессу "свои" индексы интегрирования
            size_t left_index = rank * (N / num_of_threads);
            size_t right_index = (rank != num_of_threads - 1) ? (rank + 1) * (N / num_of_threads) : N;
            // Определяем интеграл на заданном интервале
            double integral = Integrate(left_index, right_index, delta);

            // Блокируем lock
            omp_set_lock(&lock);
            // Собираем значения интегралов со всех потоков (редукция по +)
            result += integral;
            // Разблокировать lock
            omp_unset_lock(&lock);
        }
    // Удаляем lock
    omp_destroy_lock(&lock);  // необязательно делать

    // Вывод, количество процессов, полученное значение интеграла
    printf("Number of threads: %d\nIntegral value: %lf\n", num_of_threads, result);

	return 0;
}