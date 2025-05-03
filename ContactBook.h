#pragma once
#include <windows.h>
#include <string>

struct Contact
{
    std::wstring name;
    std::wstring phone;
    std::wstring country;
    std::wstring address;
    std::wstring tags;
    SYSTEMTIME addedDate;
    bool isFavorite = false;
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
        Tags
    };

    // Core contact management
    void AddContact(const Contact& contact);
    void DeleteContact(size_t index);
    void ToggleFavorite(size_t index);
    void FilterContacts(const std::wstring& filter);
    void SortContacts(SortType type);
    bool IsDuplicate(const Contact& contact);

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