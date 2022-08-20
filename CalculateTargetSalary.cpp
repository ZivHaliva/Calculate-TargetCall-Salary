#include <iostream>
#include <fstream>
#include <time.h>
#include <ctime>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>
#include <iomanip>

#pragma warning(disable : 4996)

using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using std::cin;

#define MANUAL 1
#define FILE 2
#define INVALID 0
#define VALID 1

int printUsage();
double manual();
double file();
int getNumberOfDays();
int checkInvalidTime(int hours, int minutes);
double calculateDailySalary(int startHours, int startMinutes, int endHours, int endMinutes);
double calculateShiftDuration(int startHours, int startMinutes, int endHours, int endMinutes);
std::pair<string, string> extractHoursFromLine(string line);

ifstream getFile();

int main()
{
	int choice = printUsage();
	double salary = 0.0;
	string flag = "";
	if (choice == MANUAL)
	{
		salary = manual();
	}
	else if (choice == FILE)
	{
		salary = file();
	}
	cout << "Your salary is: " << std::fixed << std::setprecision(2) << salary << endl << "Enter any key to exit: ";
	cin >> flag;
}

int printUsage()
{
	int choice = 0;
	cout << "You can enter manually your shifts or you can enter a .csv file with all your shifts already in it\n**Please notice to enter the full path of the file**" << endl;
	do
	{
		cout << "Enter 1 to enter manually, enter 2 to enter file: ";
		cin >> choice;
		while (std::cin.fail()) {
			std::cout << "Error, Not a number" << std::endl;
			cout << "Enter 1 to enter manually, enter 2 to enter file: ";
			std::cin.clear();
			std::cin.ignore(256, '\n');
			std::cin >> choice;
		}

		if (choice != MANUAL && choice != FILE)
		{
			cout << "Only 1 or 2! Don't try to be smart motherfucker" << endl;
		}
	} while (choice != MANUAL && choice != FILE);
	return choice;
}

double manual()
{
	int days = getNumberOfDays();
	int startHours = 0, startMinutes = 0, endHours = 0, endMinutes = 0;
	double salary = 0.0;

	cout << "enter invalid hours to stop(like 25:70 for example), otherwise it will get all the days of the current month" << endl;
	for (int i = 0; i < days; i++)
	{
		do
		{
			cout << "day " << i + 1 << ":" << endl;
			cout << "Enter the exact time you started the shift (HH:MM format): ";
			scanf_s("%02d:%02d", &startHours, &startMinutes);
			if (checkInvalidTime(startHours, startMinutes) == INVALID) { goto breaking; }
			cout << "Enter the exact time you finished the shift (HH:MM format): ";
			scanf_s("%02d:%02d", &endHours, &endMinutes);
			if (checkInvalidTime(endHours, endMinutes) == INVALID) { goto breaking; }

			if ((startHours > endHours) || (startHours == endHours && startMinutes > endMinutes))
			{
				cout << "The starting hour cannot be larger than the ending hour!" << endl;
			}
		} while ((startHours > endHours) || (startHours == endHours && startMinutes > endMinutes));

		salary += calculateDailySalary(startHours, startMinutes, endHours, endMinutes);
	}

	breaking:
		return salary;
}

double file()
{
	ifstream file = getFile();
	std::string line = "", startTime = "", endTime = "", startHoursStr = "", startMinutesStr = "", endHoursStr = "", endMinutesStr = "";
	double salary = 0.0;
	int startHours = 0, startMinutes = 0, endHours = 0, endMinutes = 0;
	std::pair<string, string> hours;

	while (std::getline(file, line))
	{
		if (line.length() > 12 || line.length() < 10)
		{
			cerr << "Skipping line: " << line << " because length is invalid.\nThe line should contain 9 characters in the format of: 00:00,00:00" << endl;
			continue;
		}
		//extract hours and minutes:
		hours = extractHoursFromLine(line);
		startTime = hours.first;
		endTime = hours.second;
		startHoursStr = startTime.substr(0, 2);
		startMinutesStr = startTime.substr(3, 2);
		endHoursStr = endTime.substr(0, 2);
		endMinutesStr = endTime.substr(3, 2);

		startHours = atoi(startHoursStr.c_str());
		startMinutes = atoi(startMinutesStr.c_str());
		endHours = atoi(endHoursStr.c_str());
		endMinutes = atoi(endMinutesStr.c_str());

		if ((checkInvalidTime(endHours, endMinutes) == INVALID) || (checkInvalidTime(startHours, startMinutes) == INVALID))
		{
			cerr << "Skipping line: " << line << " beacuse time is invalid." << endl;
			continue;
		}

		if ((startHours > endHours) || (startHours == endHours && startMinutes > endMinutes))
		{
			cout << "The starting hour cannot be larger than the ending hour!" << endl;
			continue;
		}

		salary += calculateDailySalary(startHours, startMinutes, endHours, endMinutes);
	}
	return salary;
}

//function will return total number of days
int getNumberOfDays()
{
	time_t theTime = time(NULL);
	struct tm* aTime = localtime(&theTime);

	int month = aTime->tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12 concept
	int year = aTime->tm_year + 1900; // Year is # years since 1900
	//leap year condition, if month is 2
	if (month == 2)
	{
		if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0))
			return 29;
		else
			return 28;
	}
	//months which has 31 days
	else if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8
		|| month == 10 || month == 12)
	{
		return 31;
	}
	else
	{
		return 30;
	}
}

int checkInvalidTime(int hours, int minutes)
{
	if ((hours < 0 || hours > 23) || (minutes < 0 || minutes > 59))
	{
		return INVALID;
	}
	else
	{
		return VALID;
	}
}

double calculateDailySalary(int startHours, int startMinutes, int endHours, int endMinutes)
{
	double shiftDuration = 0.0;
	double pay = 0.0;
	
	if (startHours >= 15 || endHours < 15)
	{
		
		shiftDuration = calculateShiftDuration(startHours, startMinutes, endHours, endMinutes);
		if (startHours >= 15)
		{
			return shiftDuration * 32;
		}
		else if (endHours < 15)
		{
			return shiftDuration * 30;
		}
	}
	else if (startHours < 15 && endHours > 15)
	{
		
		shiftDuration = calculateShiftDuration(15, 0, endHours, endMinutes);
		pay = shiftDuration * 32;

		shiftDuration = calculateShiftDuration(startHours, startMinutes, 15, 0);
		pay += shiftDuration * 30;
		return pay;
	}
}

double calculateShiftDuration(int startHours, int startMinutes, int endHours, int endMinutes)
{
	tm start, end;
	start.tm_hour = startHours;
	start.tm_min = startMinutes;
	end.tm_hour = endHours;
	end.tm_min = endMinutes;

	auto startDuration = std::chrono::hours(start.tm_hour) + std::chrono::minutes(start.tm_min);
	auto endDuration = std::chrono::hours(end.tm_hour) + std::chrono::minutes(end.tm_min);
	auto shift = endDuration - startDuration;

	auto hours = std::chrono::duration_cast<std::chrono::hours>(shift);
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(shift - hours);
	return hours.count() + (double)minutes.count() / 60.0;
}

ifstream getFile()
{
	ifstream file;
	string fileName = "";
	std::filesystem::path pathExtension;
	do
	{
		cout << "Enter file name (.csv type) or EXIT to exit: ";
		cin >> fileName;
		if (fileName == "EXIT")
		{
			exit(0);
		}
		pathExtension = std::filesystem::path(fileName).extension();
		if (pathExtension != ".csv")
		{
			cout << "Only .csv files!" << endl;
		}
	} while (pathExtension != ".csv");

	file.open(fileName);
	if (!file)
	{
		cerr << "Couldn't open file!" << endl << "Enter any key to exit: ";
		cin >> fileName;
		exit(1);
	}
	return file;
}

std::pair<string, string> extractHoursFromLine(string line)
{
	std::pair<string, string> hours;
	int i = 0, pos = 0;
	string start = "", end = "";

	for (i; i < line.length(); i++)
	{
		if (line[i] == ',') { break; }
		start += line[i];
	}

	i++; //pos is start of ending hour
	pos = i;
	for (i; i <= pos + 5; i++)
	{
		if (line[i] == '\n' || line[i] == EOF) { break; }
		end += line[i];
	}

	pos = start.find(":");
	if (pos == 1)
	{
		start = "0" + start;
	}

	hours.first = start;
	hours.second = end;
	return hours;
}