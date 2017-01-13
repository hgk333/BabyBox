//
//  SocketException.h
//  OpencvTest
//
//  Created by Tony Kwon on 2016. 3. 31..
//  Copyright © 2016년 Tony Kwon. All rights reserved.
//

#ifndef SocketException_h
#define SocketException_h

#include <string>

class SocketException
{
public:
    SocketException ( std::string s ) : m_s ( s ) {};
    ~SocketException (){};
    
    std::string description() { return m_s; }
    
private:
    
    std::string m_s;
    
};

#endif /* SocketException_h */
