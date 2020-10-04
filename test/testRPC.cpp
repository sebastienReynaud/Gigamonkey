// Copyright (c) 2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/coinbase.hpp>
#include "gtest/gtest.h"

namespace Gigamonkey::bitcoind::rpc::getminingcandidate {

    TEST(RPCTest, TestGetminingcandidate) {
        // TODO : get a real getminingcandidate, read it, take it back to json
        // get and check merkle path
        // and target
        
        struct test_case{
            string RPCResponse;
        };
        
        std::vector<test_case> test_cases{{
            R"json({ 
                "id": "a5f1f38b-2a00-4913-833a-bbcbb39d5d2c", 
                "prevHash": "0000000020493e205694c9fcb42f7d4ce5d85e230d52fccc90a6354e13940396", 
                "coinbase": "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0503878b1300ffffffff01c5a4a80400000000232103b8310da7c413106c6ce63814dbcd366c55e8ae39c8c43c1fdaeb76df56e4ff7dac00000000", 
                "coinbaseValue":23333, 
                "version": 536870912, 
                "nBits": "1c4877e8", 
                "time": 1548132190, 
                "height": 1280903, 
                "merkleProof": [ 
                    "497d51f3a933dd6e933cd37a4a5799066086d4ff45dce23f0819c7a6c7174ccb", 
                    "c2de445eda326b4afcec1291fc0dad3c526ddb551cbb01e2e10a10ebe79d2482", 
                    "7f417e9de2e8c37566141e3057eec37747a924117413ee7c2b8f902dd81b095f", 
                    "b25810a0b826ea8bf848d6e3f98f6c0bf4d097f0d1854d50c6e12988f29757d6" 
                ] 
            })json"_json}};
        
        EXPECT_FALSE(response{}.valid());
            
        for (test_case t : test_cases) {
            
            string j = t.RPCResponse;
            response c(j);
            EXPECT_TRUE(c.valid());
            
        }
    }

}

