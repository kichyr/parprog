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
    double result = (F(0) + F(N * delta)) / 2;

    //Используем статическое распределение задач по процессам
    //Итерации распределяются между потоками до начала цикла
    #pragma omp parallel for schedule(static, 10000) num_threads(num_of_threads) reduction(+: result)
        for(int i = 1; i < N; i++) {
            result += F(i * delta);
        }
    result *= delta;

    //Вывод, количество процессов, полученное значение интеграла
    printf("Number of threads: %d\nIntegral value: %lf\n", num_of_threads, result);
	return 0;
}