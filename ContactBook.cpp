#define _CRT_SECURE_NO_WARNINGS

#include "ContactBook.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <locale>
#include <codecvt>

namespace ContactBookControl
{
	ContactBook::ContactBook() : capacity(10), count(0), savedCount(0)
	{
		contacts = new Contact[capacity];
		savedContacts = new Contact[capacity];
	}

	ContactBook::~ContactBook()
	{
		delete[] contacts;
		delete[] savedContacts;
	}

	void ContactBook::ResizeArray(size_t newCapacity)
	{
		Contact* newContacts = new Contact[newCapacity];
		Contact* newSavedContacts = new Contact[newCapacity];

		// Copy existing elements
		for (size_t i = 0; i < count; i++)
			newContacts[i] = contacts[i];

		for (size_t i = 0; i < savedCount; i++)
			newSavedContacts[i] = savedContacts[i];

		delete[] contacts;
		delete[] savedContacts;

		contacts = newContacts;
		savedContacts = newSavedContacts;
		capacity = newCapacity;
	}

	void ContactBook::AddContact(const Contact& contact)
	{
		if (count >= capacity)
			ResizeArray(capacity * 2);

		contacts[count++] = contact;
	}

	void ContactBook::DeleteContact(size_t index)
	{
		if (index >= count) return;

		for (size_t i = index; i < count - 1; i++)
			contacts[i] = contacts[i + 1];

		count--;
	}

	bool ContactBook::DeleteContactByReference(const Contact& contactToDelete)
	{
		for (size_t i = 0; i < count; i++)
		{
			if (contacts[i].id == contactToDelete.id)
			{
				for (size_t i = 0; i < count; i++)
				{
					if (contacts[i].id == contactToDelete.id)
					{
						for (size_t j = i; j < count - 1; j++)
						{
							contacts[j] = contacts[j + 1];
						}
						count--;
						break;
					}
				}

				return true;
			}
		}
		return false;
	}

	void ContactBook::ToggleFavorite(const Contact& contact)
	{
		for (size_t i = 0; i < count; i++)
		{
			if (contacts[i].id == contact.id)
			{
				contacts[i].isFavorite = !contacts[i].isFavorite;
				break;
			}
		}
	}

	Contact* ContactBook::FilterContacts(const std::wstring& filter, size_t& filteredContactsCount)
	{
		currentFilter = filter;

		// Temporary array for filtered contacts - filter from currently displayed contacts
		Contact* filtered = new Contact[capacity];
		size_t filteredCount = 0;

		// Filter currently displayed contacts
		for (size_t i = 0; i < count; i++)
		{
			if (contacts[i].tags.find(filter) != std::wstring::npos)
			{
				filtered[filteredCount++] = contacts[i];
			}
		}

		filteredContactsCount = filteredCount;
		return filtered;
	}

	void ContactBook::SortContacts(SortType type)
	{
		currentSortType = type;

		switch (type)
		{
		case SortType::Favorite:
			std::sort(contacts, contacts + count, [](const Contact& a, const Contact& b)
				{
					// Sort favorites first
					if (a.isFavorite != b.isFavorite)
					{
						return a.isFavorite > b.isFavorite;
					}
					// If same favorite status, sort by name
					return a.name < b.name;
				});
			break;

		case SortType::Name:
			std::sort(contacts, contacts + count, [](const Contact& a, const Contact& b)
				{
					return a.name < b.name;
				});
			break;

		case SortType::Phone:
			std::sort(contacts, contacts + count, [](const Contact& a, const Contact& b)
				{
					return a.phone < b.phone;
				});
			break;

		case SortType::Country:
			std::sort(contacts, contacts + count, [](const Contact& a, const Contact& b)
				{
					return a.country < b.country;
				});
			break;

		case SortType::Address:
			std::sort(contacts, contacts + count, [](const Contact& a, const Contact& b)
				{
					return a.address < b.address;
				});
			break;

		case SortType::AddedTime:
			std::sort(contacts, contacts + count, [](const Contact& a, const Contact& b)
				{
					return CompareFileTime(
						&(FILETIME&)a.addedDate,
						&(FILETIME&)b.addedDate
					) > 0;
				});
			break;

		case SortType::Tags:
			std::sort(contacts, contacts + count, [](const Contact& a, const Contact& b)
				{
					return a.tags < b.tags;
				});
			break;
		}
	}

	bool ContactBook::IsDuplicate(const Contact& contact)
	{
		for (size_t i = 0; i < count; i++)
		{
			if (contacts[i].name == contact.name && contacts[i].phone == contact.phone)
				return true;
		}
		return false;
	}

	ContactBook::SortType ContactBook::GetCurrentSortType()
	{
		return currentSortType;
	}

	bool ContactBook::SaveToFile(const std::wstring& filename)
	{
		// Open file in binary mode to prevent any character conversion
		std::wofstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file.is_open())
		{
			return false;
		}

		// Apply UTF-8 conversion facet
		std::locale utf8_locale(file.getloc(),
			new std::codecvt_utf8<wchar_t, 0x10FFFF, std::generate_header>);
		file.imbue(utf8_locale);

		// Write each contact
		for (size_t i = 0; i < count; i++)
		{
			const Contact& c = contacts[i];
			file << c.name << L"|"
				<< c.phone << L"|"
				<< c.country << L"|"
				<< c.address << L"|"
				<< c.tags << L"|"
				<< c.addedDate.wYear << L"-"
				<< c.addedDate.wMonth << L"-"
				<< c.addedDate.wDay
				<< L"|"
				<< (c.isFavorite ? L"1" : L"0") << L"\n";
		}

		// Check for errors
		if (file.fail())
		{
			file.close();
			return false;
		}

		CommitChanges();
		currentFilename = filename;
		file.close();
		return true;
	}

	bool ContactBook::SaveToCurrentFile()
	{
		if (!currentFilename.empty())
		{
			return SaveToFile(currentFilename);
		}
		return false;
	}

	bool ContactBook::ParseContactLine(const std::wstring& line, Contact& contact)
	{
		const size_t MAX_PARTS = 7;
		std::wstring parts[MAX_PARTS];
		size_t partCount = 0;

		std::wstringstream ss(line);
		std::wstring part;

		while (std::getline(ss, part, L'|') && partCount < MAX_PARTS)
			parts[partCount++] = part;

		if (partCount != MAX_PARTS)
			return false;

		contact.name = parts[0];
		contact.phone = parts[1];
		contact.country = parts[2];
		contact.address = parts[3];
		contact.tags = parts[4];
		contact.addedDate = StringToSystemTime(parts[5]);
		contact.isFavorite = (parts[6] == L"1");

		return true;
	}

	bool ContactBook::LoadFromFile(const std::wstring& filename)
	{
		std::wifstream file(filename, std::ios::in | std::ios::binary);
		if (!file.is_open())
			return false;

		// Check for UTF-8 BOM and skip it
		wchar_t bom = 0;
		file.read(&bom, 1);
		if (bom != 0xFEFF)
			file.seekg(0); // No BOM, reset to start

		file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t>));

		Contact tempContacts[1000]; // Adjust size as needed
		size_t tempCount = 0;
		bool success = true;
		std::wstring line;

		while (std::getline(file, line) && tempCount < 1000)
		{
			if (ParseContactLine(line, tempContacts[tempCount]))
			{
				tempCount++;
			}
			else
			{
				// Try reading as ANSI if UTF-8 fails
				file.close();
				file.open(filename, std::ios::in | std::ios::binary);
				file.imbue(std::locale(""));
				tempCount = 0;

				while (std::getline(file, line) && tempCount < 1000)
				{
					if (ParseContactLine(line, tempContacts[tempCount]))
					{
						tempCount++;
					}
					else
					{
						success = false;
						break;
					}
				}
				break;
			}
		}

		file.close();

		if (success && tempCount > 0)
		{
			// Resize arrays if needed
			if (tempCount > capacity)
				ResizeArray(tempCount * 2);

			// Copy to saved contacts first
			savedCount = tempCount;
			for (size_t i = 0; i < savedCount; i++)
				savedContacts[i] = tempContacts[i];

			// Then copy to working contacts
			count = savedCount;
			for (size_t i = 0; i < count; i++)
				contacts[i] = savedContacts[i];

			currentFilename = filename;
		}

		return success && tempCount > 0;
	}

	std::wstring ContactBook::SystemTimeToString(const SYSTEMTIME& st) const
	{
		wchar_t buffer[64];
		swprintf(buffer, 64, L"%04d-%02d-%02d %02d:%02d:%02d",
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond);
		return buffer;
	}

	SYSTEMTIME ContactBook::StringToSystemTime(const std::wstring& str) const
	{
		SYSTEMTIME st = { 0 };
		int year, month, day, hour = 0, minute = 0, second = 0;

		if (swscanf(str.c_str(), L"%d-%d-%d %d:%d:%d",
			&year, &month, &day, &hour, &minute, &second) >= 3)
		{
			st.wYear = year;
			st.wMonth = month;
			st.wDay = day;
			st.wHour = hour;
			st.wMinute = minute;
			st.wSecond = second;
		}
		else if (swscanf(str.c_str(), L"%d-%d-%d", &year, &month, &day) == 3)
		{
			st.wYear = year;
			st.wMonth = month;
			st.wDay = day;
		}
		else
		{
			// If parsing fails, use current time
			GetLocalTime(&st);
		}

		return st;
	}

	void ContactBook::DiscardChanges()
	{
		// Restore from saved contacts
		count = savedCount;
		for (size_t i = 0; i < count; i++)
			contacts[i] = savedContacts[i];
	}

	void ContactBook::CommitChanges()
	{
		// Save current state
		savedCount = count;
		for (size_t i = 0; i < savedCount; i++)
			savedContacts[i] = contacts[i];
	}

	size_t ContactBook::GetContactCount() const
	{
		return count;
	}

	Contact& ContactBook::GetContact(size_t index) const
	{
		static Contact empty;
		return index < count ? contacts[index] : empty;
	}

	const Contact* ContactBook::GetContacts() const
	{
		return contacts;
	}
}