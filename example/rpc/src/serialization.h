#include <string>

enum SerializationType : uint32_t
{
    Min = 0,
    Int = 1,
    Double,
    String,
    Max,
};

class SerializationCoder
{
public:
    void        push(int64_t value);
    void        push(const std::string& value);
    void        push(double value);
    std::string extractData();

private:
    void append(SerializationType type, const void* data, size_t size);
    void append(SerializationType type,
                uint32_t          head,
                const void*       data,
                size_t            size);

private:
    std::string _data;
};

class SerializationDecoder
{
public:
    SerializationDecoder(std::string string);
    bool              hasNext();
    SerializationType peakNextType() const;
    int64_t           getNextInt();
    double            getNextDouble();
    std::string       getNextString();

private:
    std::string _data;
    size_t      _readIndex;
};