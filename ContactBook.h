#pragma once
#include <windows.h>
#include <string>

class Contact
{
public:
	std::wstring id;
	std::wstring name;
	std::wstring phone;
	std::wstring country;
	std::wstring address;
	std::wstring tags;
	SYSTEMTIME addedDate;
	bool isFavorite;

	Contact()
	{
		name = L"";
		phone = L"";
		country = L"";
		address = L"";
		tags = L"";
		isFavorite = false;

		// Generate unique ID when contact is created
		GUID guid;
		CoCreateGuid(&guid);
		WCHAR guidStr[40];
		StringFromGUID2(guid, guidStr, 40);
		id = guidStr;
	}
};

class ContactBook
{
public:
	ContactBook();
	~ContactBook();

	static enum SortType
	{
		NotSorted,
		Name,
		Phone,
		Country,
		Address,
		AddedTime,
		Tags,
		Favorite
	};

	// Core contact management
	void AddContact(const Contact& contact);
	void DeleteContact(size_t index);
	bool DeleteContactByReference(const Contact& contactToDelete);

	void ToggleFavorite(const Contact& contact);
	Contact* FilterContacts(const std::wstring& filter, size_t& filteredContactsCount);
	void SortContacts(SortType type);
	bool IsDuplicate(const Contact& contact);
	SortType GetCurrentSortType();

	// File operations
	bool SaveToFile(const std::wstring& filename);
	bool SaveToCurrentFile();
	bool LoadFromFile(const std::wstring& filename);
	void DiscardChanges();
	void CommitChanges();

	// Getters
	size_t GetContactCount() const;
	Contact& GetContact(size_t index) const;
	const Contact* GetContacts() const;

private:
	void ResizeArray(size_t newCapacity);
	bool ParseContactLine(const std::wstring& line, Contact& contact);
	std::wstring SystemTimeToString(const SYSTEMTIME& st) const;
	SYSTEMTIME StringToSystemTime(const std::wstring& str) const;

	Contact* contacts = nullptr;
	Contact* savedContacts = nullptr;
	size_t count = 0;
	size_t savedCount = 0;
	size_t capacity = 0;
	SortType currentSortType = SortType::NotSorted;
	std::wstring currentFilter;

	std::wstring currentFilename;
};