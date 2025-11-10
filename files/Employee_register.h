#ifndef EMPLOYEE_REGISTER_H
#define EMPLOYEE_REGISTER_H

#include <iostream>
#include <string>
#include <sstream>
#include <sqlite3.h>

#include "Employee.h"

const std::string surnames[26]
{ "Aaronson", "Backer", "Chalkley", "Dalman", "Edwards", "Flynn", "Godwin", "Harward", "Innes", "Jantz",
  "Kovachev", "Letchford", "Matterson", "Nathaniel", "Overstreet", "Perkins", "Quickenden", "Rakestraw", "Sharman", "Twyman",
  "Upshaw", "Vannoy", "Waering", "Xylander", "Yarranton", "Zachry"};
const std::string names[20]
{ "Maria", "Ekaterina", "Eva", "Anna", "Elizaveta", "Yasmina", "Victoria", "Svetlana", "Alexandra", "Maryana",
"Artyom", "Ilya", "Timofey", "Svyatoslav", "Sergey", "Grigory", "Roman", "German", "Boris", "Stepan" };
const std::string patronymics[10]
{ "Ilyinichna", "Romanovna", "Glebovna", "Nikolaevna", "Eliseevna",
"Daniilovich", "Yanovich", "Vladimirovich", "Arsentievich", "Tikhonovich" };

std::string random_full_name(const int gender)
{
	return std::string((surnames[std::rand() % 26]) + " " + (names[(gender * 10) + std::rand() % 10]) + " " + (patronymics[(gender * 5) + std::rand() % 5]));
}
std::string special_full_name()
{
	return std::string(surnames[5] + " " + names[10 + (std::rand() % 10)] + " " + patronymics[5 + (std::rand() % 5)]);
}

Date random_date() { return Date(1990 + (std::rand() % 16), (std::rand() % 12) + 1, (std::rand() % 31) + 1); }

int callback(void* notUsed, int colCount, char** columns, char** colNames)
{
	std::stringstream data{ "" };
	Employee employee{ };

	for (int i = 0; i < colCount; i++) data << std::string(columns[i] ? columns[i] : "NULL") + " ";
	data >> employee;

	std::cout << employee << " " << employee.calculate_age() << " |\n";

	return 0;
}

class Employee_register
{
public:
	Employee_register() { open_data_base("Employee_register.db"); }
	Employee_register(const std::string& db_name) { open_data_base(db_name + ".db"); }

	void create_table(const std::string& table_name = "Employee")
	{
		std::string sql_command{ "CREATE TABLE " + table_name + " (Id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, Full_name TEXT NOT NULL, Date_of_birth TEXT NOT NULL, Gender TEXT NOT NULL);" };
		char* error_message{ nullptr };

		if ((sqlite3_exec(db, sql_command.c_str(), 0, 0, &error_message)) != SQLITE_OK)
		{
			std::cerr << "SQL error: " << error_message << "\n\n";
			sqlite3_free(error_message);
		}
		else std::cout << "Table " << table_name << " created succesefully\n\n";
	}

	void insert_employee(const std::string& table_name = "Employee")
	{
		Employee employee{};
		char exit{};

		while (std::cin >> exit && exit != '0')
		{
			std::cin.putback(exit);
			std::cin >> employee;
			employee.insert_into_table(db, table_name);
		}
		clear_duplicates(table_name);
		std::cout << '\n';
	}

	void show_all_employee(const std::string& table_name = "Employee")
	{
		std::string sql_command{ "SELECT Full_name, Date_of_birth, Gender FROM " + table_name + " GROUP BY Full_name, Date_of_birth" };
		char* error_message{ nullptr };

		if ((sqlite3_exec(db, sql_command.c_str(), callback, 0, &error_message)) != SQLITE_OK)
		{
			std::cerr << "SQL error: " << error_message << "\n\n";
			sqlite3_free(error_message);
		}
		else std::cout << "The result above this statement\n\n";
	}

	void random_fill(const std::string& table_name = "Employee")
	{
		std::srand(std::time({}));

		const unsigned size{ 1000100 };
		Employee* employees{ new Employee[size] };

		for (int size1 = 0; size1 < 100; size1++)
		{
		    int size2{ 0 + (10000 * size1) };
			employees[size2] = Employee(special_full_name(), random_date(), true);
		    
			for (size2++; size2 < 10000 + (10000 * size1) + (1 * size1) + 1; size2++) 
			{
				const int gender{ std::rand() % 2 };
				employees[size2] = Employee(random_full_name(gender), random_date(), (gender ? true : false));
			}
		}

		batch_insertion(table_name, employees, size);

		delete[] employees;
	}

	void specific_select(const std::string& table_name = "Employee")
	{
		std::string sql_command{ "SELECT Full_name, Date_of_birth, Gender FROM " + table_name + " WHERE Full_name LIKE 'F%' AND Gender == 'Male';"};
		char* error_message{ nullptr };

		time_t start{}, end{};
		time(&start);

		if ((sqlite3_exec(db, sql_command.c_str(), callback, 0, &error_message)) != SQLITE_OK)
		{
			std::cerr << "SQL error: " << error_message << "\n\n";
			sqlite3_free(error_message);
		}
		else
		{
			time(&end);
			std::cout << "The result of custom select is above this statement and the time is - " << double(difftime(end, start)) << " seconds\n\n";
		}
	}

	void batch_insertion(const std::string& table_name, const Employee* employees, const unsigned& size)
	{
		std::stringstream sql_command{ "" };
		char* error_message{ nullptr };

		sql_command << "BEGIN TRANSACTION; ";
		for (unsigned counter = 0; counter < size; counter++)
		{
			sql_command << "INSERT INTO " + table_name + " (Full_name, Date_of_birth, Gender) VALUES ('"
				        << employees[counter].get_full_name() << "', '"
				        << employees[counter].get_date_of_birth().year << '-' << employees[counter].get_date_of_birth().month << '-' << employees[counter].get_date_of_birth().day << "', '"
				        << (employees[counter].get_gender() ? "Male" : "Female")
				        << "'); ";
		}
		sql_command << "COMMIT;";

		if ((sqlite3_exec(db, sql_command.str().c_str(), 0, 0, &error_message)) != SQLITE_OK)
		{
			std::cerr << "SQL error: " << error_message << "\n\n";
			sqlite3_free(error_message);
		}
		else
		{
			std::cout << "Batch insertion in table " << table_name << " completed succesefully\n";
			clear_duplicates(table_name);
		}
	}

	~Employee_register() { sqlite3_close(db); }
private:
	sqlite3* db{};

	void open_data_base(const std::string& db_name)
	{
		if ((sqlite3_open(db_name.c_str(), &db)) != SQLITE_OK) std::cerr << "Error open DB: " << sqlite3_errmsg(db) << "\n\n";
		else std::cout << "Database " << db_name << " opened succesefully\n\n";
	}

	void clear_duplicates(const std::string& table_name = "Employee")
	{
		std::string sql_command{ "DELETE FROM " + table_name + " WHERE rowid NOT IN (SELECT MIN(rowid) FROM " + table_name + " GROUP BY Full_name);" };
		char* error_message{ nullptr };

		if ((sqlite3_exec(db, sql_command.c_str(), 0, 0, &error_message)) != SQLITE_OK)
		{
			std::cerr << "SQL error: " << error_message << "\n\n";
			sqlite3_free(error_message);
		}
		else std::cout << "Duplicates in table " << table_name << " deleted succesefully\n\n";
	}
};

#endif