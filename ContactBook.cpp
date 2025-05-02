#include "ContactBook.h"


void ContactBook::AddContact(const Contact& contact)
{
	if (!IsDuplicate(contact))
	{
		contacts.push_back(contact);
	}
}

void ContactBook::DeleteContact(size_t index)
{
	if (index < contacts.size())
	{
		contacts.erase(contacts.begin() + index);
	}
}

void ContactBook::ToggleFavorite(size_t index)
{
	if (index < contacts.size())
	{
		contacts[index].isFavorite = !contacts[index].isFavorite;
	}
}

void ContactBook::FilterContacts(const std::string& filter)
{
	currentFilter = filter;
}

void ContactBook::SortContacts(int sortBy)
{
	currentSort = sortBy;
	switch (sortBy)
	{
	case 0: // Date
		/*   std::sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b) {
			   return CompareSystemTime(&a.addedDate, &b.addedDate) > 0;
			   });*/
		break;
	case 1: // Name
		std::sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b)
			{
				return a.name < b.name;
			});
		break;
	case 2: // Country
		std::sort(contacts.begin(), contacts.end(), [](const Contact& a, const Contact& b)
			{
				return a.country < b.country;
			});
		break;
	}
}

bool ContactBook::ValidatePhone(const std::string& phone)
{
	// Phone validation implementation
	return phone.length() >= 7 &&
		std::count_if(phone.begin(), phone.end(), isdigit) >= 7;
}

bool ContactBook::IsDuplicate(const Contact& contact)
{
	return std::any_of(contacts.begin(), contacts.end(), [&](const Contact& c)
		{
			return c.name == contact.name && c.phone == contact.phone;
		});
}

const std::vector<Contact>& ContactBook::GetContacts()
{
	return contacts;
}

const Contact& ContactBook::GetContact(size_t index)
{
	static Contact empty;
	return index < contacts.size() ? contacts[index] : empty;
}

size_t ContactBook::GetContactCount()
{
	return contacts.size();
}