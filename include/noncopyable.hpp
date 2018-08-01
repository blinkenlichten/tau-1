#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

namespace tau1 {

class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;
public:
    noncopyable( const noncopyable& ) = delete;
    noncopyable& operator=( const noncopyable& ) = delete;
};

}//tau1

#endif // NONCOPYABLE_HPP
