#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <sqlite3.h>

#pragma warning(disable:4996) 

std::string tolower(std::string word)
{
	for (char& symbol : word) symbol = std::tolower(symbol);
	return word;
}
std::string clear_word(const std::string& word)
{
	std::string result{ "" };
	for (const char& symbol : word) ((symbol != '\"') ? result += symbol : result += "");
	return result;
}

struct Date
{
	Date() : year{ 2000 }, month{ 1 }, day{ 1 } {}
	Date(int y, int m, int d) : year{ y }, month{ m }, day{ d } {}

	int year{}, month{}, day{};
};
bool is_correct_date(const Date& date)
{
	if ((date.year < 0) || (date.month < 1 || date.month > 12) || (date.day < 1 || date.day > 31)) return false;
	return true;
}

class Employee
{
public:
	Employee() : full_name{ "" }, date_of_birth{ Date() }, gender{ false } {}
	Employee(const std::string& fn, const Date& dob, const bool& g) : full_name{ fn }, date_of_birth{ dob }, gender{ g }
	{
		if (!is_correct_date(date_of_birth)) std::cerr << "Incorrect date in employee: " << full_name << "\n\n";
	}

	const std::string& get_full_name() const { return full_name; }
	const Date& get_date_of_birth()    const { return date_of_birth; }
	const bool& get_gender()           const { return gender; }

	void set_full_name(const std::string& fn) { full_name = fn; }
	void set_date_of_birth(const Date& dob)
	{
		if (!is_correct_date(dob)) std::cerr << "Incorrect date in employee: " << full_name << "\n\n";
		date_of_birth = dob;
	}
	void set_gender(const bool& g) { gender = g; }

	void insert_into_table(sqlite3* db, const std::string& table_name) const
	{
		if ((full_name == "") || (!is_correct_date(date_of_birth))) return;

		std::stringstream employee_values{""};
		employee_values << full_name << "', '" << date_of_birth.year << '-' << date_of_birth.month << '-' << date_of_birth.day << "', '" << (gender ? "Male" : "Female");
		std::string sql_command{ "INSERT INTO " + table_name + " (Full_name, Date_of_birth, Gender) VALUES ('" + employee_values.str() + "');" };

		char* error_message{ nullptr };
		if ((sqlite3_exec(db, sql_command.c_str(), 0, 0, &error_message)) != SQLITE_OK)
		{
			std::cerr << "SQL error: " << error_message << '\n';
			sqlite3_free(error_message);
		}
		else std::cout << "Employee inserted succesefully in table " << table_name << "\n";
	}

	const int calculate_age() const
	{
		time_t theTime = time(NULL);
		struct tm* aTime = localtime(&theTime);
		return ((aTime->tm_year + 1900) - date_of_birth.year);
	}
private:
	std::string full_name{ "" };
	Date date_of_birth{};
	bool gender{};
};

std::ostream& operator << (std::ostream& out, const Date& date)
{
	return out << date.year << '-' << std::setw(2) << std::setfill('0') << date.month << '-' << std::setw(2) << std::setfill('0') << date.day << std::setfill(' ');
}
std::ostream& operator << (std::ostream& out, const Employee& employee)
{
	out << "| " << std::setw(35) << employee.get_full_name() << " | " << employee.get_date_of_birth() << " | " << std::setw(8);
	return (employee.get_gender() ? out << "Male |" : out << "Female |");
}

std::istream& operator >> (std::istream& in, Date& date)
{
	char separator{ ' ' };
	int year{ 2000 }, month{ 1 }, day{ 1 };
	in >> year >> separator >> month >> separator >> day;

	date = Date(year, month, day);
	return in;
}
std::istream& operator >> (std::istream& in, Employee& employee)
{
	std::string surname{ "" }, name{ "" }, patronymic{ "" };
	Date date{};
	std::string gender{ "" };

	in >> surname >> name >> patronymic >> date >> gender;

	employee = Employee(clear_word(surname) + " " + name + " " + clear_word(patronymic), date, ((tolower(gender) == "male") ? true : false));
	return in;
}

#endif