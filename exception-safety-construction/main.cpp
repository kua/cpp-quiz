///////////////////////// header //////////////////////////////////////////////////////////

#include <iostream>
#include <assert.h>
#include <algorithm> // std::copy
#include <cstddef>
#include <memory>

//using namespace std;

static int g_instance_counter = 0;
static int g_memory_usage = 0;
static bool g_throw_on_constructor = false;

///////////////////////// code //////////////////////////////////////////////////////////

template<typename T>
class Array
{
public:
  // (default) constructor
  Array(const size_t size = 0)
    : m_size(size)
    , m_array(m_size ? new T[m_size]() : nullptr)
  {
  }

  // unsafe version
/*  Array& operator=(const Array& other)
  {
    if(&other != this)
    {
      delete [] m_array;
      m_size = other.m_size;
      m_array = new T[m_size];
      std::copy(other.m_array, other.m_array + m_size, m_array);
    }
    return *this;
  }*/

  // safe version
  Array& operator=(Array other)
  {
    swap(*this, other);
    return *this;
  }

  // move constructor
  Array(Array&& other)
    : Array()
  {
    swap(*this, other);
  }

  // copy-constructor
  Array(const Array& other)
    : m_size(other.m_size),
      m_array(m_size ? new T[m_size] : nullptr)
  {
    //std::copy(other.m_array.get(), other.m_array.get() + m_size, m_array.get());

    try
    {
      std::copy(other.m_array, other.m_array + m_size, m_array);
    }
    catch(...)
    {
      delete [] m_array;
      throw;
    }
  }

  // destructor
  ~Array()
  {
    delete [] m_array;
  }

  void swap(Array& first, Array& second) // nothrow
  {
    std::swap(first.m_size, second.m_size);
    std::swap(first.m_array, second.m_array);
  }

  const size_t size() const
  {
    return m_size;
  }

  T& operator [](const size_t index)
  {
    assert(index >= 0 && index < m_size);

    return m_array[index];
  }

private:
  size_t m_size;
  T* m_array;
  //std::unique_ptr<T[]> m_array;
};

///////////////////////// footer //////////////////////////////////////////////////////////

struct Foo
{
  Foo(int data = 5)
    : m_data(data)
  {
    if(g_throw_on_constructor)
      throw std::runtime_error("operation failed");

    ++g_instance_counter;
  }

  ~Foo()
  {
    --g_instance_counter;
  }

  void reset(const int data)
  {
    m_data = data;
  }

  Foo& operator = (const Foo& other)
  {
    throw std::runtime_error("operation failed");
  }

  bool operator == (const Foo& other)
  {
    return m_data == other.m_data;
  }

  void* operator new[](std::size_t sz)
  {
    ++g_memory_usage;
    return ::operator new[](sz);
  }
  void operator delete[](void* ptr) noexcept
  {
    --g_memory_usage;
    ::operator delete[](ptr);
  }

  int m_data;
};



void safetyTest(bool throwOnConstuctor = false)
{
  const size_t SOURCE_SIZE = 10;
  const size_t DIST_SIZE = 5;

  Array<Foo> source(SOURCE_SIZE);
  Array<Foo> dist(DIST_SIZE);

  for(size_t i = 0; i < dist.size(); ++i)
    dist[i].reset(i);

  g_throw_on_constructor = throwOnConstuctor;

  if(!g_memory_usage)
  {
    std::cout << "Array is not allocated on the heap." << std::endl;
    exit(EXIT_SUCCESS);
  }

  try
  {
    dist = source;
  }
  catch(const std::exception& ex)
  {
    if(dist.size() != DIST_SIZE)
    {
      std::cout << "In case of an assignment operator failure, array size is changed." << std::endl;
      exit(EXIT_SUCCESS);
    }

    for(size_t i = 0; i < dist.size(); ++i)
      if(dist[i].m_data != i)
      {
        std::cout << "In case of an assignment operator failure, array data is changed." << std::endl;
        exit(EXIT_SUCCESS);
      }
  }
}

void checkObjectsDestruction()
{
  if(g_instance_counter || g_memory_usage)
  {
    std::cout << "Test does not destroy all the objects that it creates." << std::endl;

    exit(EXIT_SUCCESS);
  }
}

int main(int argc, char *argv[])
try
{
  safetyTest();
  checkObjectsDestruction();

  safetyTest(true);
  checkObjectsDestruction();

  return EXIT_SUCCESS;
}
catch (const std::exception& error)
{
  std::cout << "An error occurred while running the tests: " << error.what() << std::endl;

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cout << "An error occurred while running the tests: " << std::endl;

  return EXIT_SUCCESS;
}
