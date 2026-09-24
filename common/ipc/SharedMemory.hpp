#pragma once

#include <Windows.h>
#include <cstdint>
#include <format>
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
    bool Create(const std::string& name, uint32_t width, uint32_t height);

    // Открыть существующую shared memory (клиент)
    bool Open(const std::string& name, uint32_t width, uint32_t height);

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
        return size_;
    }

    // Проверить открыто ли
    bool IsOpen() const
    {
        return handle_ != nullptr;
    }

    static size_t CalculateSharedMemorySize(uint32_t width, uint32_t height)
    {
        return sizeof(SharedFrame) + static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    }

    // Единое имя для host и plugin. Обе стороны обязаны использовать его,
    // иначе после ресайза (пересоздания) отображения разъедутся.
    static std::string MakeName(uint32_t parentPid, uint32_t id)
    {
        return std::format("LuaCEF_SharedMemory_{}_{}", parentPid, id);
    }

  private:
    HANDLE handle_;
    void* buffer_ptr_;
    std::string name_;
    size_t size_;
};

} // namespace ipc