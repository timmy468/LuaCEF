#include "SharedMemory.hpp"
#include <iostream>

#include "Logger.hpp"

namespace ipc
{

SharedMemory::SharedMemory() : handle_(nullptr), buffer_ptr_(nullptr), size_(0)
{
}

SharedMemory::~SharedMemory()
{
    Close();
}

bool SharedMemory::Create(const std::string& name, uint32_t width, uint32_t height)
{
    name_ = name;

    size_ = CalculateSharedMemorySize(width, height);

    // Создать файл в памяти для shared memory
    handle_ =
        CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(size_), name.c_str());

    if (handle_ == nullptr)
    {
        Logger::Instance().Log("Failed to create shared memory: {}", GetLastError());
        return false;
    }

    // Отобразить в адресное пространство
    buffer_ptr_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size_);

    if (buffer_ptr_ == nullptr)
    {
        Logger::Instance().Log("Failed to map shared memory: {}", GetLastError());
        CloseHandle(handle_);
        handle_ = nullptr;
        return false;
    }

    Logger::Instance().Log("Shared memory created: {} ({:.2f} MB)", name_,
                           static_cast<float>(size_) / (1024.0f * 1024.0f));

    return true;
}

bool SharedMemory::Open(const std::string& name, uint32_t width, uint32_t height)
{
    name_ = name;
    size_ = CalculateSharedMemorySize(width, height);

    // Открыть существующий файл в памяти
    handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name_.c_str());

    if (handle_ == nullptr)
    {
        Logger::Instance().Log("Failed to open shared memory: {}", GetLastError());
        return false;
    }

    // Отобразить в адресное пространство
    buffer_ptr_ = MapViewOfFile(handle_, FILE_MAP_ALL_ACCESS, 0, 0, size_);

    if (buffer_ptr_ == nullptr)
    {
        Logger::Instance().Log("Failed to map shared memory: {}", GetLastError());
        CloseHandle(handle_);
        handle_ = nullptr;
        return false;
    }

    Logger::Instance().Log("Shared memory opened: {} ({:.2f} MB), width: {}, height: {}", name_,
                           static_cast<float>(size_) / (1024.0f * 1024.0f), width, height);
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