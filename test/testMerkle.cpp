// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/merkle.hpp>
#include "gtest/gtest.h"

namespace Gigamonkey::Merkle {
    
    TEST(MerkleTest, TestMerkle) {
        EXPECT_FALSE(leaf{}.valid());
        EXPECT_FALSE(path{}.valid());
        EXPECT_FALSE(proof{}.valid());
        EXPECT_FALSE(tree{}.valid());
        EXPECT_FALSE(dual{}.valid());
        
        list<string> transactions("a", "b", "c", "d", "e", "f", "g", "h");
        
        list<digest256> leaves = for_each([](const string x) -> digest256 {
            return Bitcoin::hash256(x);
        }, transactions);
        
        digest256 fail = Bitcoin::hash256("Z");
        
        for (int i = 1; i <= leaves.size(); i++) {
            leaf_digests l = take(leaves, i);
            
            tree Tree{l};
            
            EXPECT_TRUE(Tree.valid());
            
            // check that the root function will calculate the same value as the tree root. 
            EXPECT_EQ(Tree.root(), root(l));
            
            // construct the dual tree from the tree. 
            dual Dual{Tree};
            
            EXPECT_TRUE(Dual.valid());
            
            server Server{l};
            
            EXPECT_TRUE(Dual.valid());
            
            EXPECT_EQ(server(Tree), Server);
            
            EXPECT_EQ(Tree, tree(Server));
            
            EXPECT_EQ(Dual, dual(Server));
            
            dual ReconstructedLeft{};
            dual ReconstructedRight{};
            
            for (uint32 j = 0; j < i; j++) {
                proof p = Dual[j];
                proof q = Server[j];
                
                EXPECT_FALSE(ReconstructedLeft[i].valid());
                
                ReconstructedLeft = ReconstructedLeft + p;
                
                EXPECT_TRUE(ReconstructedLeft[i].valid());
        
                EXPECT_TRUE(p.valid());
                EXPECT_EQ(p, q);
                p.Root = fail;
                EXPECT_FALSE(p.valid());
            }
            
            uint32 j = i;
            while (j > 0) {
                j--;
                
                proof p = Dual[j];
                
                EXPECT_FALSE(ReconstructedRight[i].valid());
                
                ReconstructedRight = ReconstructedRight + p;
                
                EXPECT_TRUE(ReconstructedRight[i].valid());
            }
            
            EXPECT_EQ(Dual, ReconstructedLeft);
            EXPECT_EQ(Dual, ReconstructedRight);
            
            Dual.Root = fail;
            EXPECT_FALSE(Dual.valid());
        }
    }
}
