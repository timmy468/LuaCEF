#include "SharedMemory.hpp"
#include <iostream>

namespace ipc
{

SharedMemory::SharedMemory() : handle_(nullptr), buffer_ptr_(nullptr)
{
}

SharedMemory::~SharedMemory()
{
    Close();
}

bool SharedMemory::Create(const std::string& name)
{
    name_ = name;

    // Создать файл в памяти для shared memory
    handle_ = CreateFileMappingA(INVALID_HANDLE_VALUE, // Использовать системный page file
                                 nullptr, PAGE_READWRITE,
                                 0,                   // High order размер
                                 sizeof(SharedFrame), // Low order размер
                                 name_.c_str());

    if (handle_ == nullptr)
    {
        std::cerr << "Failed to create shared memory: " << GetLastError() << std::endl;
        return false;
    }

    // Отобразить в адресное пространство
    buffer_ptr_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedFrame));

    if (buffer_ptr_ == nullptr)
    {
        std::cerr << "Failed to map shared memory: " << GetLastError() << std::endl;
        CloseHandle(handle_);
        handle_ = nullptr;
        return false;
    }

    std::cout << "Shared memory created: " << name_ << " (" << sizeof(SharedFrame) / (1024 * 1024) << " MB)"
              << std::endl;
    return true;
}

bool SharedMemory::Open(const std::string& name)
{
    name_ = name;

    // Открыть существующий файл в памяти
    handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name_.c_str());

    if (handle_ == nullptr)
    {
        std::cerr << "Failed to open shared memory: " << GetLastError() << std::endl;
        return false;
    }

    // Отобразить в адресное пространство
    buffer_ptr_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedFrame));

    if (buffer_ptr_ == nullptr)
    {
        std::cerr << "Failed to map shared memory: " << GetLastError() << std::endl;
        CloseHandle(handle_);
        handle_ = nullptr;
        return false;
    }

    std::cout << "Shared memory opened: " << name_ << " (" << sizeof(SharedFrame) / (1024 * 1024) << " MB)"
              << std::endl;
    return true;
}

void SharedMemory::Close()
{
    if (buffer_ptr_ != nullptr)
    {
        UnmapViewOfFile(buffer_ptr_);
        buffer_ptr_ = nullptr;
    }

    if (handle_ != nullptr)
    {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
}

} // namespace ipc