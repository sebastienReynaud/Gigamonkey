// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/merkle.hpp>

namespace Gigamonkey::Merkle {
    
    leaf_digests round(leaf_digests l) {
        leaf_digests r{};
        while(l.size() >= 2) {
            r = r << hash_concatinated(l.first(), l.rest().first());
            l = l.rest().rest();
        }
        if (l.size() == 1) r = r << hash_concatinated(l.first(), l.first());
        return r;
    }
    
    tree tree::make(leaf_digests h) {
        if (h.size() == 0) tree{};
        if (h.size() == 1) return tree{h.first()};
        
        leaf_digests next_round = round(h);
        
        using trees = list<data::tree<digest256>>;
        trees Trees{};
        
        leaf_digests last = h;
        leaf_digests next = next_round;
        while (!next.empty()) {
            if (last.size() >= 2) {
                Trees = Trees << data::tree<digest256>{next.first(), 
                    data::tree<digest256>{last.first()}, 
                    data::tree<digest256>{last.rest().first()}};
                last = last.rest().rest();
            } else {
                Trees = Trees << data::tree<digest256>{next.first(), 
                    data::tree<digest256>{last.first()}, 
                    data::tree<digest256>{}};
                last = last.rest();
            }
            next = next.rest();
        }
        
        while(next_round.size() >= 1) {
            trees last_trees = Trees;
            Trees = trees{};
            next_round = round(next_round);
            next = next_round;
            
            while(!next.empty()) {
                if (last_trees.size() >= 2) {
                    Trees = Trees << data::tree<digest256>{next.first(), 
                        data::tree<digest256>{last_trees.first()}, 
                        data::tree<digest256>{last_trees.rest().first()}};
                    last_trees = last_trees.rest().rest();
                } else {
                    Trees = Trees << data::tree<digest256>{next.first(), 
                        data::tree<digest256>{last_trees.first()}, 
                        data::tree<digest256>{}};
                    last_trees = last_trees.rest();
                }
                next = next.rest();
            }
        }
        
        // trees should have size 1 by this point. 
        return Trees.first();
    }
    
    digest256 root(list<digest256> l) {
        if (l.size() == 0) return {};
        while (l.size() > 1) l = round(l); 
        return l.first();
    }
        
    branch branch::rest() const {
        if (Digests.empty()) return *this;
        digest256 next;
        if (Leaf.Index & 1) next = hash_concatinated(Digests.first(), Leaf.Digest);
        else next = hash_concatinated(Leaf.Digest, Digests.first());
        return branch{leaf{next, Leaf.Index >> 1}, Digests.rest()};
    }
        
    digest256 branch::root() const {
        branch p = *this;
        leaf l = Leaf;
        while(!p.Digests.empty()) {
            p = p.rest();
            if (l != p.Leaf) return {};
        }
        return l.Digest;
    }
    
    uint32 height(data::tree<digest256>) {
        throw 0;
    }
    
    bool tree::valid() const {
        if (Height == 0 || Width == 0 || !data::tree<digest256>::valid() || Height != height(*this)) return false;
        list<proof> p = proofs();
        return Width == p.size() && check_proofs(proofs());
    }
    
    proof tree::operator[](uint32 i) const {
        if (i >= Width) return {};
        
        uint32 max_index = Width - 1;
        data::tree<digest256> t = *this;
        uint32 next_height = Height - 1;
        list<digest256> digests;
        
        while (next_height > 0) {
            digests = digests << t.root();
            
            if ((i >> next_height) & 0 || i == max_index) {
                t = t.right();
            } else {
                t = t.left();
            }
            
            next_height--;
        }
        
        return proof{branch{leaf{t.root(), i}, digests}, data::tree<digest256>::root()};
    }
    
    list<proof> tree::proofs() const {
        throw 0;
    }
    
    dual::dual(const proof& b) : dual{} {
        if (!b.valid()) return;
        Paths = Paths.insert(b.Branch);
        Root = b.Root;
    }
    
    list<proof> dual::proofs() const {
        list<proof> p;
        for (const entry<uint32, list<digest256>>& e : Paths.values()) {
            if (e.Value.size() == 0) return {};
            p = p << proof{branch{leaf{e.Value.first(), e.Key}, e.Value.rest()}, Root};
        }
        return p;
    }
    
    bool check_proofs(list<proof> x) {
        if (x.size() == 0) return false;
        dual d{};
        do {
            d = d + x.first();
            if (!d.valid()) return false;
            x = x.rest();
        } while (x.size() > 0);
        return true;
    }
    
    dual::dual(const tree& t) {
        list<proof> p = t.proofs();
        if (p.size() == 0) return;
        Width = t.Width;
        Height = t.Height;
        Root = t.root();
        for (const proof& x : p) Paths = Paths.insert(x.Branch);
    }
    
    dual dual::insert(const branch& p) const {
        /*
        if (!p.valid()) return *this;
        
        if (contains(p.Leaf.Index) && operator[](p.Leaf.Index).Branch == p) return *this;
        else return dual{};*/
        
        throw 0; // TODO
    }
    
    dual dual::operator+(const dual& d) const {
        if (!valid()) return d;
        if (Root != d.Root) return {};
        dual a = *this;
        for (const proof& x : d.proofs()) a = a.insert(x.Branch);
        return a;
    }
    
    server::server(const tree& t) {
        throw 0;
    }
    
    server::operator tree() const {
        throw 0;
    }
    
    server::server(leaf_digests l) : server{} {
        if (l.size() == 0) return;
        
        Width = l.size();
        Height = 1;
        
        uint32 width = Width;
        uint32 total = width;
        while (width > 1) {
            width = (width + 1) / 2;
            total += width;
            Height++;
        } 
        Digests.resize(total);
        
        leaf_digests v = l;
        uint32 i = 0;
        
        while (true) {
            leaf_digests x = v;
            while (!x.empty()) {
                Digests[i] = x.first();
                x = x.rest();
                i++;
            }
            if (i == total) return;
            v = round(v);
        } 
        
    }
    
    proof server::operator[](uint32 index) const {
        if (index >= Width) return {};
        
        list<digest256> p;
        uint32 i = index;
        uint32 cumulative = 0;
        size_t width = Width;
        
        while (width > 1) {
            p = p << Digests[cumulative + i + (i & 1 ? - 1 : i == width - 1 ? 0 : 1)];
            cumulative += width;
            width = (width + 1) / 2;
            i >>= 1;
        }
        
        return proof{branch{leaf{Digests[index], index}, p}, root()};
    }
    
}
