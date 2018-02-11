#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

namespace Tau1 {

class noncopyable
{
public:
    noncopyable() = default;
    ~noncopyable() = default;
    noncopyable( const noncopyable& ) = delete;
    noncopyable& operator=( const noncopyable& ) = delete;
};

}//Tau1

#endif // NONCOPYABLE_HPP
