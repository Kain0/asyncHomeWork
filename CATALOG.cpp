#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <future>
#include <omp.h>

//Попов Андрей
//БПИ197
//Вариант 16
//16. Задача об инвентаризации по рядам.После нового года в
//библиотеке университета обнаружилась пропажа каталога.После поиска и
//наказания виноватых, ректор дал указание восстановить каталог силами
//студентов.Фонд библиотека представляет собой прямоугольное помещение,
//в котором находится M рядов по N шкафов по K книг в каждом шкафу.
//Требуется создать многопоточное приложение, составляющее каталог.При
//решении задачи использовать метод «портфель задач», причем в качестве
//отдельной задачи задается составление каталога одним студентом для одного
//ряда.


/// <summary>
/// ввод целого числа, вводимое пользователем
/// </summary>
/// <param name="message">Сообщегие, котороре будет показано пользователю</param>
/// <param name="min_value">Минимальное значение вводимого числа</param>
/// <param name="max_value">Максимальное значение вводимого числа</param>
/// <returns>возвращает вводимое число</returns>
int get_int(std::string message, int min_value = INT32_MIN, int max_value = INT32_MAX) {
	while (true) {
		int integer;
		std::cout << message;
		std::cin >> integer;
		if (std::cin.fail() || min_value > integer || integer > max_value) {
			std::cin.clear();
			std::cin.ignore(INT32_MAX, '\n');
			std::cout << "Wrong number. Try again.\n";
		}
		else {
			std::cin.ignore(INT32_MAX, '\n');
			return integer;
		}
	}

}

/// <summary>
/// Возвращает случайное целое число включая обе границы
/// </summary>
/// <param name="min_value">левая граница</param>
/// <param name="max_value">правая граница</param>
/// <returns>Возвращает случайное целое число включая обе границы</returns>
int next_int(int min_value, int max_value, int my_seed = 0) {
	return min_value + ((rand()+my_seed) % (max_value - min_value + 1));
}

/// <summary>
/// создаёт случайное название книги
/// </summary>
/// <param name="min_length">минимальная длина</param>
/// <param name="max_length">максимальная длина</param>
/// <returns>случайное название книги</returns>
std::string get_random_title(int min_length = 3, int max_length = 30, int my_seed = 0) {
	std::string str;
	str += (char)next_int('A', 'Z', my_seed);
	for (int i = 0; i < next_int(min_length, max_length, my_seed) - 1; i++) {
		if (next_int(1, 1000, my_seed) < 175) //обусловлено приблизительной частотой пробела в текстах на английском языке
			str += " ";
		else
			str += (char)next_int('a', 'z', my_seed);
	}
	return str;

}


/// <summary>
/// Создаёт ряд в каталоге
/// </summary>
/// <param name="library">каталог</param>
/// <param name="library_row">конкретный ряд этого каталога</param>
/// <param name="M"></param>
/// <param name="N"></param>
/// <param name="K"></param>
void make_row(std::string** library_row, int* catalog_row, int N, int K, int student_number, int row_number) {
	std::vector<std::string> prototype_library_row;
	std::vector<int> prototype_catalog_row;
	for (int j = 0; j < N; j++) {
		for (int k = 0; k < K; k++) {
#pragma omp critical (searching)
			{
				library_row[j][k] = get_random_title(0, 30, row_number);
				std::cout << "student " << student_number << " has found "
					<< '|' << library_row[j][k] << "| at the position " << '[' << row_number << "][" << j << "][" << k << "] \n";
			}
			prototype_library_row.push_back(library_row[j][k]);
			prototype_catalog_row.push_back(j * K + k);
		}
	}
	std::sort(prototype_catalog_row.begin(), prototype_catalog_row.end(),
		[prototype_library_row](int a, int b) {return prototype_library_row[a] < prototype_library_row[b]; });
	for (int j = 0; j < K * N; j++)
		catalog_row[j] = prototype_catalog_row[j];
}

/// <summary>
/// Составляет каталог в нескольких потоках
/// </summary>
/// <param name="library">библиотека</param>
/// <param name="catalog">каталог
/// <param name="M"></param>
/// <param name="N"></param>
/// <param name="K"></param>
/// <returns></returns>
void student_thread(std::string*** library, int** catalog, int M, int N, int K) {
	srand(time(NULL));
#pragma omp parallel for 
	for (int i = 0; i < M; i++) { //реализация "портфеля зада" - поиск рядов, к которым ещё никто не приступал и заполнение этих рядов.
		int student_number = omp_get_thread_num();
		//std::cout << "student " << student_number << " started working with row " << i << '\n'; //для проверок, что потоки работают как надо
		make_row(library[i], catalog[i], N, K, omp_get_thread_num(), i);
		//std::cout << "student " << student_number << " finished working with row " << i << '\n';


	}
}

/// <summary>
/// Выписывает все книги каталога 
/// </summary>
/// <param name="library">библиотека</param>
/// <param name="catalog">каталог</param>
/// <param name="M"></param>
/// <param name="N"></param>
/// <param name="K"></param>
void write_answer(std::string*** library, int** catalog, int M, int N, int K) {
	std::cout << "CATALOG:\n";
	for (int i = 0; i < M; i++) {
		std::cout << "row " << i << ":\n";
		for (int j = 0; j < N * K; j++) {
			int n = catalog[i][j] / K;
			int k = catalog[i][j] % K;
			std::cout << '|' << library[i][n][k] << "| at the position " << '[' << i << "][" << n << "][" << k << "] \n";
		}
	}

}

int main() {
	int M = get_int("Enter the number of rows(M)(Not less than 1, not more then 100): ", 1, 100); // ввод входных данных
	int N = get_int("Enter the number of bookcases(N)(Not less than 1, not more than 100): ", 1, 100);
	int K = get_int("Enter the number of books(K)(Not less than 1, not more than 100): ", 1, 100);
	int** catalog = new int* [M];
	std::string*** library = new std::string * *[M]; //генерация библитеки
	for (int i = 0; i < M; i++) {
		catalog[i] = new int[N * K];
		library[i] = new std::string * [N];
		for (int j = 0; j < N; j++)
			library[i][j] = new std::string[K];
	}
	student_thread(library, catalog, M, N, K);
	write_answer(library, catalog, M, N, K); //вывод ответа на задачу
}

