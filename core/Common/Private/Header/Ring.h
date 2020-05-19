#pragma once
#include <vector>
#include <cstdint>

template<class T>
class CircularBuffer
{
private:
    std::vector<T> m_buffer;
    uint64_t m_start;
    uint64_t m_end;
    uint64_t m_capacity;
    uint64_t m_size;

public:

    uint64_t size() const 
    {
        // s = &i[start]
        // e = (&i[end] + end)
        // delta = (e - s) % s
        return m_size;
        //return (((m_end + m_capacity) - m_start) % m_capacity);
    };


    uint64_t capacity() const
    {
        return m_capacity;
    };


    //void push_back(T val);
    void push_back(T val)
    {
        m_buffer[m_end] = val;
        m_end = (m_end + 1) % m_capacity;
        ++m_size;
    };

    void insert(const T* pBuffer, const uint64_t uSize)
    {
        // Memory Copy
        // Does a single copy suffice?
        uint64_t dEndContiguous = (m_capacity - m_end);
        
        if(uSize <= dEndContiguous) // Yes, one memcpy works
        {
            memcpy(&m_buffer[m_end], pBuffer, uSize);
            m_end = m_end + (uSize);
        }
        else
        {
            // We have to split the memcpy
            memcpy(&m_buffer[m_end], pBuffer, dEndContiguous * sizeof(T));

            
            // Second
            memcpy(&m_buffer[0], pBuffer + dEndContiguous, (uSize - dEndContiguous) * sizeof(T));
            m_end = 0 + (uSize - dEndContiguous);
        }

        m_size += uSize;

    };

    bool empty() const noexcept
    {
        return size() == 0;
    };

    bool full() const noexcept
    {
        return size() == capacity();
    };

    uint64_t reserve() const noexcept
    {
        return capacity() - size();
    };

    void clear() noexcept
    {
        m_start = 0;
        m_end=0;
        m_size=0;
    }

    T& operator[](const uint64_t index)
    {
        return m_buffer[(m_start + index) % m_capacity];
    };

    void remove(uint64_t nElements) noexcept
    {
        m_start = (m_start + nElements) % m_capacity;
        m_size -= nElements;
    };

    //T& operator[](uint64_t index);

    //void insert( );

    CircularBuffer() : m_start(0), m_end(0),m_capacity(0), m_size(0)
    {
    };


    explicit CircularBuffer(uint64_t uSize) :
        m_buffer(uSize),
        m_start(0),
        m_end(0),
        m_capacity(uSize),
        m_size(0)

    {
    };


    ~CircularBuffer() = default;
};