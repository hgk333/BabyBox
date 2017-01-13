//
//  ServerSocket.hpp
//  OpencvTest
//
//  Created by Tony Kwon on 2016. 3. 31..
//  Copyright © 2016년 Tony Kwon. All rights reserved.
//

#ifndef ServerSocket_hpp
#define ServerSocket_hpp

#include <stdio.h>

#include "Socket.hpp"


class ServerSocket : private Socket
{
public:
    
    ServerSocket ( int port );
    ServerSocket (){};
    virtual ~ServerSocket();
    
    const ServerSocket& operator << ( const std::string& ) const;
    const ServerSocket& operator >> ( std::string& ) const;
    
    void accept ( ServerSocket& );
    
};

#endif /* ServerSocket_hpp */
