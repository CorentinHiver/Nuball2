#ifndef HITBUFFER_HPP
#define HITBUFFER_HPP

/**
 * @brief Hits container
 * @details HitBuffer
 */
class HitBuffer
{
public:
  HitBuffer(size_t size = 100) : m_max_size(size) {m_buffer.resize(size);}
  
  void push_back(Hit const & hit) {m_buffer[m_size++] = hit;}
  void clear() {m_size = 0; m_buffer.clear(); m_buffer.resize(m_max_size); m_step = 0; m_nb_clear++;}

  auto const & operator[] (size_t const & i) const {return m_buffer[i];}
  auto operator[] (size_t const & i) {return m_buffer[i];}
  
  auto at(size_t const & i) 
  {
    if(i>m_size)
    {
      print("Exceeding size of HitBuffer");
      return m_buffer[m_size];
    }
    else return m_buffer[i];
  }

  auto const & size() const {return m_size;}
  auto max_size() const {return m_max_size;} 

  auto begin()       {return m_buffer.begin();}
  auto begin() const {return m_buffer.begin();}
  auto end  ()       {return m_buffer.end  ();}
  auto end  () const {return m_buffer.end  ();}

  /// @brief Are all the slots used ?
  bool isFull() const {return m_size+2 > m_max_size;}

  auto const & nbClear() const {return m_nb_clear;}
  auto       & nbClear()       {return m_nb_clear;}

  void setStep(int const & step) {m_step = step;}
  auto const & step() const {return m_step;}
  auto       & step()       {return m_step;}

  /**
   * @brief DEPRECATED Shifts the buffer of hits by a certain amount
   * @todo Make it functionnal maybe ?
   * 
   * @param n:
   *    if n = 0 nothing happens
   * 
   *    if n>0 shifts the buffer to the right by n indexes, expands the size if needed. Creates empty slots at beginning.
   * 
   *    if n<0 moves the n last cells to the beginning, size stays the same. Overflow logic : the first hits are moved to the end.
   * 
   * @details
   * Example : 
   * HitBuffer buffer = {hit1, hit2, hit3, hit4};
   * buffer.shift( 2); // buffer = {empty1, empty2, hit1, hit2, hit3, hit4}
   * buffer.shift(-2); // buffer = {hit3, hit4, hit1, hit2}
   * Carefull : for n>0 , need to copy twice the data, so it can be very long for big buffers
   */
  void shift(int n = 0)
  {
    if (n == 0) return;
    else if (n>0)
    {
      std::vector<Hit> temp;
      temp.reserve(m_size+n);
      for (int i = 0; i<n; i++) temp[i] = Hit(); // Fill the nth first hits with empty hits
      for (size_t i = 0; i<m_size; i++)
      {// Shifts the other hits
        temp[i+n] = m_buffer[i];
      }
      m_buffer = temp;// Copy back to the first container
      m_size += n;// Update size
    }
    else if (n>0)
    {
      for (int i = 0; i<n; i++)
      {
        m_buffer[i] = m_buffer[m_size-n+i];
      }
      m_size = n;
    }
  }

private:
  size_t m_max_size = 0;
  size_t m_size = 0;
  int m_step = 0;
  size_t m_nb_clear = 0;
  std::vector<Hit> m_buffer;
};

#endif //HITBUFFER_HPP
