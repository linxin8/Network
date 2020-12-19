
#include "serialization.h"
#include "../../../src/log.h"
#include <algorithm>
#include <bit>
#include <charconv>

template <typename T>
constexpr T localToNet(T x)
{
    T ret = 0;
    if constexpr (std::endian::native == std::endian::little)
    {
        for (int i = 0; i < sizeof(T); i++)
        {
            ret |= (x >> ((sizeof(T) - 1 - i) * 8) & 0xff) << (i * 8);
        }
    }
    return ret;
}

double localToNet(double x)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        char* begin = (char*)&x;
        char* last  = begin + sizeof(x) - 1;
        while (begin < last)
        {
            char temp = *begin;
            *begin    = *last;
            *last     = temp;
            begin++;
            last--;
        }
    }
    return x;
}

template <typename T>
constexpr T netToLocal(T x)
{
    T ret = 0;
    if constexpr (std::endian::native == std::endian::little)
    {
        for (int i = 0; i < sizeof(T); i++)
        {
            ret |= (x >> ((sizeof(T) - 1 - i) * 8) & 0xff) << (i * 8);
        }
    }
    return ret;
}

double netToLocal(double x)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        char* begin = (char*)&x;
        char* last  = begin + sizeof(x) - 1;
        while (begin < last)
        {
            char temp = *begin;
            *begin    = *last;
            *last     = temp;
            begin++;
            last--;
        }
    }
    return x;
}

void SerializationCoder::push(int64_t value)
{
    value = localToNet(value);
    append(Int, &value, sizeof(value));
}
void SerializationCoder::push(const std::string& value)
{
    append(String, value.length(), value.c_str(), value.length());
}

void SerializationCoder::push(double value)
{
    value = localToNet(value);
    append(Double, &value, sizeof(value));
}
std::string SerializationCoder::extractData()
{
    return std::move(_data);
}

void SerializationCoder::append(SerializationType type,
                                const void*       data,
                                size_t            size)
{
    uint32_t uniformType = localToNet<uint32_t>(type);
    _data.append((char*)&uniformType, sizeof(uniformType));
    _data.append((const char*)data, size);
}

void SerializationCoder::append(SerializationType type,
                                uint32_t          head,
                                const void*       data,
                                size_t            size)
{
    uint32_t uniformType = localToNet<uint32_t>(type);
    uint32_t headByte    = localToNet<uint32_t>(head);
    _data.append((char*)&uniformType, sizeof(uniformType));
    _data.append((char*)&headByte, sizeof(headByte));
    _data.append((const char*)data, size);
}

SerializationDecoder::SerializationDecoder(std::string string) :
    _data{std::move(string)}, _readIndex{0}
{
}

SerializationType SerializationDecoder::peakNextType() const
{
    LOG_ASSERT(_readIndex + sizeof(uint32_t) < _data.size());
    uint32_t type = *(uint32_t*)&(_data[_readIndex]);
    type          = netToLocal(type);
    LOG_ASSERT(SerializationType::Min < type && type < SerializationType::Max);
    return SerializationType(type);
}

bool SerializationDecoder::hasNext()
{
    return _readIndex < _data.length();
}

int64_t SerializationDecoder::getNextInt()
{
    LOG_ASSERT(peakNextType() == SerializationType::Int);
    _readIndex += sizeof(SerializationType);
    int64_t ret;
    memcpy(&ret, _data.c_str() + _readIndex, sizeof(ret));
    _readIndex += sizeof(ret);
    ret = netToLocal(ret);
    return ret;
}

double SerializationDecoder::getNextDouble()
{
    LOG_ASSERT(peakNextType() == SerializationType::Double);
    _readIndex += sizeof(SerializationType);
    double ret;
    memcpy(&ret, _data.c_str() + _readIndex, sizeof(ret));
    _readIndex += sizeof(ret);
    ret = netToLocal(ret);
    return ret;
}

std::string SerializationDecoder::getNextString()
{
    LOG_ASSERT(peakNextType() == SerializationType::String);
    _readIndex += sizeof(SerializationType);
    uint32_t head;
    memcpy(&head, _data.c_str() + _readIndex, sizeof(head));
    _readIndex += sizeof(head);
    head = netToLocal(head);
    LOG_ASSERT(_readIndex + head <= _data.length());
    std::string ret{_data.c_str() + _readIndex, head};
    _readIndex += head;
    return std::move(ret);
}