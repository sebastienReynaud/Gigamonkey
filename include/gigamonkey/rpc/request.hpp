// Copyright (c) 2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_RPC_REQUEST
#define GIGAMONKEY_RPC_REQUEST

#include <gigamonkey/timechain.hpp>

namespace Gigamonkey::bitcoind::rpc {
    enum call {
        invalid, 
        getminingcandidate, 
    };
    
    struct request {};
}

#endif
