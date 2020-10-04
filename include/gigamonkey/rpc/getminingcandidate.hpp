// Copyright (c) 2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_RPC_GETMININGCANDIDATE
#define GIGAMONKEY_RPC_GETMININGCANDIDATE

#include <gigamonkey/timechain.hpp>
#include <gigamonkey/work/proof.hpp>

namespace Gigamonkey::bitcoind::rpc::getminingcandidate {
    
    using request = bool;

    struct response : json {
        
        struct deserialized;
        
        static void to_json(json& j, const deserialized&);
        static void from_json(const json&, deserialized&);
        
        static response serialize(const deserialized& d) {
            response e;
            to_json(static_cast<json&>(e), d);
            return e;
        }
        
        static deserialized deserialize(const json& j) {
            deserialized d;
            from_json(j, d);
            return d;
        }
        
        struct deserialized {
        
            // explanations from from https://wiki.bitcoinsv.io/index.php/Getminingcandidate 
            // The assigned id from Bitcoind. This must be provided with submitting a potential solution
            string id;
            
            // The block version
            int32_little version;
            
            // Big Endian hash of the previous block
            digest256 prevhash;
            
            // Compressed hexadecimal difficulty
            work::compact nBits;
            
            // Current block time
            Bitcoin::timestamp time; 
            
            // The candidate block height
            int height;
            
            // Merkle branch/path for the block, used to calculate the Merkle Root of the block candidate.
            // This is a list of Little-Endian hex strings ordered from top to bottom
            Merkle::path merkleProof;
            
            uint64 num_tx;
            
            uint64 sizeWithoutCoinbase;
            
            // Total funds available for the coinbase tx (in Satoshis)
            satoshi coinbaseValue;
            
            // Suggested coinbase transaction is provided. Miner is free to supply their own or alter the
            // supplied one. Altering will require parsing and splitting the coinbase in order to splice
            // in/out data as required. Requires Wallet to be enabled. Only returned when provide_coinbase_tx
            // argument is set to true. Returns error if Wallet is not enabled. 
            optional<Bitcoin::transaction> coinbase;
            
            bool valid() const {
                return id != "" && prevhash.valid() && version >= 2 && coinbaseValue >= 0 && nBits.valid() && time.valid() && 
                    height > 0 && merkleProof.valid() && merkleProof.Index == 0 && num_tx >= 1 && (!coinbase || coinbase->valid());
            }
            
            bool operator==(const deserialized& j) {
                return id == j.id && prevhash == j.prevhash && coinbase == j.coinbase && 
                    version == j.version && coinbaseValue == j.coinbaseValue && nBits == j.nBits && 
                    time == j.time && height == j.height && merkleProof == j.merkleProof && num_tx == j.num_tx && 
                    sizeWithoutCoinbase == j.sizeWithoutCoinbase;
            }
            
            bool operator!=(const deserialized& j) {
                return !(*this == j);
            }
        
            response serialize() {
                return response::serialize(*this);
            }
            
            explicit operator work::candidate() const {
                return {version, prevhash, nBits, merkleProof};
            }
            
        private:
            deserialized() : id{}, version{}, prevhash{}, nBits{}, time{}, height{}, 
                merkleProof{}, num_tx{0}, sizeWithoutCoinbase{0}, coinbaseValue{0}, coinbase{} {}
            
            deserialized(const string& i, int32_little vx, const digest256 prev, work::compact n, 
                Bitcoin::timestamp t, int h, Merkle::digests mp, uint64 nt, uint64 swc, 
                satoshi v, const Bitcoin::transaction& cb) : id{i}, version{vx}, prevhash{prev}, 
                nBits{n}, time{t}, height{h}, merkleProof{0, mp}, num_tx{nt}, sizeWithoutCoinbase{swc}, 
                coinbaseValue{v}, coinbase{cb} {}
            
            deserialized(const string& i, int32_little vx, const digest256 prev, work::compact n, 
                Bitcoin::timestamp t, int h, Merkle::digests mp, uint64 nt, uint64 swc, 
                satoshi v) : id{i}, version{vx}, prevhash{prev}, nBits{n}, 
                time{t}, height{h}, merkleProof{0, mp}, num_tx{nt}, sizeWithoutCoinbase{swc}, 
                coinbaseValue{v} {}
                
            friend struct response;
            friend void from_json(const json&, deserialized&);
        };
        
        deserialized deserialize() const {
            return deserialize(*this);
        }
        
        response() : json{} {}
        
        response(const json& j) : json{j} {}
        
        response(const string& json_string) : response{json::parse(json_string)} {}
        
        response(const deserialized& d) : response{serialize(d)} {}
        
        response(const string& id, int32_little version, const digest256 prevhash, work::compact nBits, 
            Bitcoin::timestamp time, int height, Merkle::digests merkleProof, uint64 num_tx, 
            uint64 sizeWithoutCoinbase, satoshi coinbaseValue, const Bitcoin::transaction& coinbase) : 
            response{deserialized{id, version, prevhash, nBits, time, height, merkleProof, num_tx, sizeWithoutCoinbase, coinbaseValue, coinbase}} {}
            
        response(const string& id, int32_little version, const digest256 prevhash, work::compact nBits, 
            Bitcoin::timestamp time, int height, Merkle::digests merkleProof, uint64 num_tx, 
            uint64 sizeWithoutCoinbase, satoshi coinbaseValue) : 
            response{deserialized{id, version, prevhash, nBits, time, height, merkleProof, num_tx, sizeWithoutCoinbase, coinbaseValue}} {}
        
        static bool valid(const json& j) {
            return deserialize(j).valid();
        }
        
        static string id(const json& j) {
            return deserialize(j).id;
        }
        
        static int32_little version(const json& j) {
            return deserialize(j).version;
        }
        
        static digest256 prevhash(const json& j) {
            return deserialize(j).prevhash;
        }
        
        static work::compact nBits(const json& j) {
            return deserialize(j).nBits;
        }
        
        static Bitcoin::timestamp time(const json& j) {
            return deserialize(j).time;
        }
        
        static int32 height(const json& j) {
            return deserialize(j).height;
        }
        
        static Merkle::path merkleProof(const json& j) {
            return deserialize(j).merkleProof;
        }
        
        static optional<Bitcoin::transaction> coinbase(const json& j) {
            return deserialize(j).coinbase;
        }
        
        static satoshi coinbaseValue(const json& j) {
            return deserialize(j).coinbaseValue;
        }
        
        bool valid() const {
            return valid(*this);
        }
        
        string id() const {
            return id(*this);
        }
        
        digest256 prevhash() const {
            return prevhash(*this);
        }
        
        work::compact nBits() const {
            return nBits(*this);
        }
        
        Bitcoin::timestamp time() const {
            return time(*this);
        }
        
        int32 height() const {
            return height(*this);
        }
        
        Merkle::path merkleProof() const {
            return merkleProof(*this);
        }
        
        optional<Bitcoin::transaction> coinbase() const {
            return coinbase(*this);
        }
        
        satoshi coinbaseValue() const {
            return coinbaseValue(*this);
        }
        
        explicit operator work::candidate() const {
            return work::candidate(deserialize());
        }
    };
}

#endif
