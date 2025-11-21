#include <iostream>

#include "Employee_register.h"

// Демонстрация работы приложения. 
//Доступные опции (Значения mod):
// 1 - создать таблицу Employee
// 2 - добавить сотрудника в таблицу Employee
// 3 - показать поля таблицы Employee
// 4 - заполнить таблицу случайными сотрудниками
// 5 - вывести всех сотрудников у которых фамилия начинается на F и пол мужской
int main()
{
	Employee_register official_list{};

	while (true)
	{
		std::cout << "DatabaseApp.exe -> ";
		int mod{ 0 }; std::cin >> mod;

		switch (mod)
		{
		case 1: official_list.create_table();      break;
		case 2: official_list.insert_employee();   break;
		case 3: official_list.show_all_employee(); break;
		case 4: official_list.random_fill();       break;
		case 5: official_list.specific_select();   break;
		default: std::cerr << "Incorrect mod!\n";  break;
		}
	}

	return 0;

}
