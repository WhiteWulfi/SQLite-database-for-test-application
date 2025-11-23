#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <time.h>
#include <sqlite3.h>

#pragma warning(disable:4996) 

// Вспомогательная функция принимающая слово затем возвращающая это же слово, но в нижнем регистре. 
std::string tolower(const std::string& word)
{
	std::string result{ "" };
	for (const char& symbol : word) result += std::tolower(symbol);
	return result;
}
// Вспомогательная функция принимающее слово и исключённый символ затем возвращающая вариант слова, но без исключённого символа.
std::string clear_word(const std::string& word, const char& banned)
{
	std::string result{ "" };
	for (const char& symbol : word) ((symbol == banned) ? result += "" : result += symbol);
	return result;
}

// Простая структура данных олицетворяющая дату организованную по шаблону: yyyy.mm.dd (Значение по умолчанию 2000.01.01.)
struct Date
{
	Date() : year{ 2000 }, month{ 1 }, day{ 1 } {}
	Date(int y, int m, int d) : year{ y }, month{ m }, day{ d } {}

	int year{}, month{}, day{};
};
// Вспомогательная функция проверяющая дату на логическую верность (Не учитывает високосный год)
bool is_correct_date(const Date& date)
{
	// условия для того, чтобы дата была верной: год > 0 и месяц > 0, 12 >= месяц и день > 0, 31 >= день
	if ((date.year < 0) || (date.month < 1 || date.month > 12) || (date.day < 1 || date.day > 31)) return false;
	return true;
}

// Класс олицетворяющий сущность типа 'сотрудник' (То есть единицу данных в таблице по учёту сотрудников) 
class Employee
{
public:
	Employee() : full_name{ "" }, date_of_birth{ Date() }, gender{ false } {}
	Employee(const std::string& fn, const Date& dob, const bool& g) : full_name{ fn }, date_of_birth{ dob }, gender{ g }
	{
		if (!is_correct_date(date_of_birth)) std::cerr << "Incorrect date in employee: " << full_name << "\n\n";
	}

	const std::string& get_full_name() const { return full_name;     }
	const Date& get_date_of_birth()    const { return date_of_birth; }
	const bool& get_gender()           const { return gender;        }

	void set_full_name(const std::string& fn) { full_name = fn; }
	void set_date_of_birth(const Date& dob)
	{
		if (!is_correct_date(dob)) std::cerr << "Incorrect date in employee: " << full_name << "\n\n";
		date_of_birth = dob;
	}
	void set_gender(const bool& g)           { gender = g;      }

	// Метод рассчитывающий возраст сотрудника
	const int calculate_age() const
	{
		time_t theTime = time(NULL);
		struct tm* aTime = localtime(&theTime);
		return ((aTime->tm_year + 1900) - date_of_birth.year);
	}

	// Метод отправляющий данного сотрудника в конкретную таблицу находящуюся в заданной базе данных
	void insert_into_table(sqlite3* db, const std::string& table_name) const
	{
		// Проверка на правильность входных данных, иначе сотрудник не добавляется
		if ((full_name == "") || (!is_correct_date(date_of_birth))) return;

		// Запрос на добавление сотрудника пошагово собирается на языке SQL
		std::stringstream employee_values{""};
		employee_values << full_name << "', '" << date_of_birth.year << '-' << date_of_birth.month << '-' << date_of_birth.day << "', '" << (gender ? "Male" : "Female");
		std::string sql_command{ "INSERT INTO " + table_name + " (Full_name, Date_of_birth, Gender) VALUES ('" + employee_values.str() + "');" };
		// Пример результата INSERT INTO Employee (Full_name, Date_of_birth, Gender) VALUES (Иванов Иван Иванович, 2000-01-01, Male);

		// Попытка выполнить составленный запрос
		char* error_message{ nullptr };
		if ((sqlite3_exec(db, sql_command.c_str(), 0, 0, &error_message)) != SQLITE_OK)
		{
			// Если отправка завершилась ошибкой, то выводится сообщение об ошибке
			std::cerr << "SQL error: " << error_message << '\n';
			sqlite3_free(error_message);
		}
		// Если отправка завершилась успешно, то сотрудник добавлен и выводится сообщение об успехе операции
		else std::cout << "Employee inserted succesefully in table " << table_name << "\n";
	}
private:
	// Сущность 'сотрудник' состоит из полей full_name (ФИО), date_of_birth (Дата рождения) и gender (Пол)
	std::string full_name{ "" };
	Date date_of_birth{};
	bool gender{};
};

// Перегрузка оператора вывода << для класса Date по шаблону: yyyy-mm-dd
std::ostream& operator << (std::ostream& out, const Date& date)
{
	return out << date.year << '-' << std::setw(2) << std::setfill('0') << date.month << '-' << std::setw(2) << std::setfill('0') << date.day << std::setfill(' ');
}
// Перегрузка оператора вывода << для класса Employee по шаблону: | full_name | date_of_birth | Male/Female |
std::ostream& operator << (std::ostream& out, const Employee& employee)
{
	out << "| " << std::setw(35) << employee.get_full_name() << " | " << employee.get_date_of_birth() << " | " << std::setw(8);
	return (employee.get_gender() ? out << "Male |" : out << "Female |");
}

// Перегрузка оператора ввода >> для классов Date по шаблону: yyyy-mm-dd, где '-' любой символ
std::istream& operator >> (std::istream& in, Date& date)
{
	char separator{ ' ' };
	int year{ 2000 }, month{ 1 }, day{ 1 };
	in >> year >> separator >> month >> separator >> day;

	date = Date(year, month, day);
	return in;
}
// Перегрузка оператора ввода >> для класса Employee по шаблону: "full_name" date_of_birth Male/Female
std::istream& operator >> (std::istream& in, Employee& employee)
{
	std::string surname{ "" }, name{ "" }, patronymic{ "" };
	Date date{};
	std::string gender{ "" };

	in >> surname >> name >> patronymic >> date >> gender;

	employee = Employee(clear_word(surname, '\"') + " " + name + " " + clear_word(patronymic, '\"'), date, ((tolower(gender) == "male") ? true : false));
	return in;
}

#endif


