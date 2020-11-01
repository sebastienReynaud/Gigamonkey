// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_MERKLE
#define GIGAMONKEY_MERKLE

#include "hash.hpp"

namespace Gigamonkey::Merkle {
        
    inline digest256 hash_concatinated(const digest256& a, const digest256& b) {
        return Bitcoin::hash256(write(64, a, b));
    }
    
    // all hashes for the leaves of a given tree in order starting from zero.
    using leaf_digests = list<digest256>;
    
    digest256 root(leaf_digests l);
    
    struct path {
        uint32 Index;
        list<digest256> Digests;
        
        path();
        path(uint32 i, list<digest256> p);
        
        bool valid() const;
        
        digest256 derive_root(const digest256& leaf) const;
    };
    
    struct leaf {
        digest256 Digest;
        uint32 Index;
        
        leaf();
        leaf(digest256 d, uint32 i);
        leaf(digest256 d) : leaf(d, 0) {}
        
        bool valid() const;
    };
    
    struct branch {
        leaf Leaf;
        list<digest256> Digests;
        
        branch();
        branch(leaf);
        branch(leaf, list<digest256>);
        branch(const digest256& d, path p) : branch{leaf{d, p.Index}, p.Digests} {}
        
        bool valid() const;
        
        bool empty() const;
        
        leaf first() const;
        
        operator leaf() const;
        
        operator path() const {
            return path{Leaf.Index, Digests};
        }
        
        branch rest() const;
        
        digest256 root() const;
        
        operator data::entry<uint32, list<digest256>>() const {
            return data::entry<uint32, list<digest256>>{Leaf.Index, Digests << Leaf.Digest};
        }
    };
    
    inline digest256 path::derive_root(const digest256& leaf) const {
        return branch{leaf, *this}.root();
    }
    
    struct proof {
        branch Branch;
        digest256 Root;
    
        proof();
        proof(const digest256& root);
        proof(branch p, const digest256& root);
        
        bool valid() const;
    };
    
    inline digest256 root(const proof& p) {
        return p.Root;
    }
    
    // efficiently check that all paths have the same root.
    // return that root if they do. returned root hash will
    // be an invalid value if not all paths are valid
    // and have the same root. 
    bool check_proofs(list<proof>);
    
    struct tree : data::tree<digest256> {
        uint32 Width;
        uint32 Height;
        
        static tree make(leaf_digests);
        
        tree();
        tree(const digest256& root);
        tree(leaf_digests h) : tree{make(h)} {}
        
        bool valid() const;
        
        list<proof> proofs() const;
        
        proof operator[](uint32 i) const;
        
    private:
        tree(data::tree<digest256> t) : data::tree<digest256>{t} {}
    };
    
    inline digest256 root(const tree t) {
        return t.root();
    }
    
    // dual to the Merkle tree. Prunable. 
    // would be good in a wallet. 
    class dual {
        map<uint32, list<digest256>> Paths;
        
        dual insert(const branch&) const;
        
    public:
        digest256 Root;
        uint32 Width;
        uint32 Height;
        
        dual() : Paths{}, Root{}, Width{0}, Height{0} {}
        dual(map<uint32, list<digest256>> m, digest256 root) : Paths{m}, Root{root} {}
        dual(digest256 root) : dual{{}, root} {}
        
        dual(const proof& b);
        dual(const tree& t);
        
        list<proof> proofs() const;
        
        bool valid() const {
            return Root.valid() && Paths.valid() && check_proofs(proofs());
        }
        
        bool contains(uint32) const;
        
        proof operator[](uint32 b) const {
            auto e = Paths[b];
            if (!e.valid()) return proof{};
            return proof{branch{leaf{e.first(), b}, e.rest()}, Root};
        }
        
        list<leaf> leaves() const;
        
        dual operator+(const dual& d) const;
        
        bool operator==(const dual& d) const {
            return Width == d.Width && Height == d.Height && Root == d.Root && Paths == d.Paths;
        }
    };
    
    inline dual operator+(const proof& a, const proof& b) {
        return dual{a} + b;
    }
    
    // for serving branches. Would be on a miner's computer. 
    class server {
        cross<digest256> Digests;
        
        server() : Digests{}, Width{0}, Height{0} {}
        
    public:
        uint32 Width;
        uint32 Height;
        
        server(leaf_digests);
        server(const tree&);
        
        bool valid() const;
        
        operator tree() const;
        
        operator dual() const;
        
        digest256 root() const;
        
        proof operator[](uint32 index) const;
        
        bool operator==(const server& s) const {
            return Width == s.Width && Height == s.Height && Digests == s.Digests;
        }
    };
    
    inline std::ostream& operator<<(std::ostream& o, const path& p) {
        return o << "path{" << p.Index << ", " << p.Digests << "}";
    }
    
    inline bool operator==(const path& a, const path& b) {
        return a.Index == b.Index && a.Digests == b.Digests;
    }
    
    inline bool operator==(const leaf& a, const leaf& b) {
        return a.Digest == b.Digest && a.Index == b.Index;
    }
    
    inline bool operator!=(const leaf& a, const leaf& b) {
        return a.Digest != b.Digest || a.Index != b.Index;
    }
    
    inline bool operator==(const branch& a, const branch& b) {
        return a.Leaf == b.Leaf && a.Digests == b.Digests;
    }
    
    inline bool operator==(const proof& a, const proof& b) {
        return a.Root == b.Root && a.Branch == b.Branch;
    }
    
    inline bool operator==(const tree& a, const tree& b) {
        return a.Width == b.Width && a.Height == b.Height && static_cast<data::tree<digest256>>(a) == static_cast<data::tree<digest256>>(b);
    }
    
    inline path::path() : Index{0}, Digests{} {}
    
    inline path::path(uint32 i, list<digest256> p) : Index{i}, Digests{} {}
        
    inline bool path::valid() const {
        return Digests.valid();
    }
        
    inline leaf::leaf() : Digest{}, Index{0} {}
    
    inline leaf::leaf(digest256 d, uint32 i) : Digest{d}, Index{i} {}
    
    inline bool leaf::valid() const {
        return Digest.valid();
    }
    
    inline branch::branch(): Leaf{}, Digests{} {}
    
    inline branch::branch(leaf l, list<digest256> p) : Leaf{l}, Digests{p} {}
    
    inline branch::branch(leaf l) : Leaf{l}, Digests{} {}
        
    inline bool branch::valid() const {
        return Leaf.valid() && Digests.valid();
    }
        
    inline bool branch::empty() const {
        return Digests.empty();
    }
        
    inline leaf branch::first() const {
        return Leaf;
    }
    
    inline branch::operator leaf() const {
        return Leaf;
    }
    
    inline proof::proof() : Branch{}, Root{} {}
    
    inline proof::proof(branch p, const digest256& root) : Branch{p}, Root{root} {}
    
    inline proof::proof(const digest256& root) : Branch{}, Root{root} {}
    
    inline bool proof::valid() const {
        return Branch.valid() && Root == Branch.root();
    }
    
    inline tree::tree() : data::tree<digest256>{}, Width{0}, Height{0} {}
    
    inline tree::tree(const digest256& root) : data::tree<digest256>{root}, Width{1}, Height{1} {}
    
    inline server::operator dual() const {
        return dual{operator tree()};
    }
    
    inline digest256 server::root() const {
        return Digests[Digests.size() - 1];
    }
}

#endif
