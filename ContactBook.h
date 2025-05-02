#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>

struct Contact
{
	std::string name;
	std::string phone;
	std::string country;
	std::string address;
	std::string tags;
	SYSTEMTIME addedDate;
	bool isFavorite = false;
};

class ContactBook
{
public:
	ContactBook() {};

	// Core contact management
	void AddContact(const Contact& contact);
	void DeleteContact(size_t index);
	void ToggleFavorite(size_t index);
	void FilterContacts(const std::string& filter);
	void SortContacts(int sortBy);

	// Getters
	const std::vector<Contact>& GetContacts();
	const Contact& GetContact(size_t index);
	size_t GetContactCount();

	// Validation
	bool ValidatePhone(const std::string& phone);
	bool IsDuplicate(const Contact& contact);

private:
	std::vector<Contact> contacts;
	int currentSort;
	std::string currentFilter;
};