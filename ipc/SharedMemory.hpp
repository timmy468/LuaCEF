#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

#include "SharedFrame.hpp"

namespace ipc
{

// Управление shared memory для фреймов браузера
class SharedMemory
{
  public:
    SharedMemory();
    ~SharedMemory();

    // Создать shared memory (сервер)
    bool Create(const std::string& name = "LuaCEF_SharedMemory");

    // Открыть существующую shared memory (клиент)
    bool Open(const std::string& name = "LuaCEF_SharedMemory");

    // Закрыть
    void Close();

    // Получить указатель на буфер
    void* GetBuffer() const
    {
        return buffer_ptr_;
    }

    // Получить размер
    size_t GetSize() const
    {
        return sizeof(SharedFrame);
    }

    // Проверить открыто ли
    bool IsOpen() const
    {
        return handle_ != nullptr;
    }

  private:
    HANDLE handle_;
    void* buffer_ptr_;
    std::string name_;
};

} // namespace ipc