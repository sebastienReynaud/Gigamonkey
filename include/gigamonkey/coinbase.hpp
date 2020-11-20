// Copyright (c) 2019 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef GIGAMONKEY_COINBASE
#define GIGAMONKEY_COINBASE

#include <gigamonkey/timechain.hpp>
#include <gigamonkey/address.hpp>
#include <gigamonkey/stratum/session_id.hpp>
#include <gigamonkey/rpc/getminingcandidate.hpp>
#include <gigamonkey/rpc/submitminingsolution.hpp>
#include <gigamonkey/work/proof.hpp>

namespace Gigamonkey::Bitcoin::mining {
    
    struct coinbase : transaction {
        bool bip44(int32 height) const;
        
        bool valid() const;
        
        coinbase(const Bitcoin::transaction& t) : transaction{t} {}
        coinbase(int32 height, Stratum::session_id nonce1, uint64_big nonce2, list<Bitcoin::output> outputs);
        
        static bytes part_1(int32 height);
        static bytes part_2(list<Bitcoin::output> outputs);
        
        struct incomplete {
            bytes Part1;
            bytes Part2;
            
            coinbase complete(Stratum::session_id nonce1, uint64_big nonce2) const {
                return coinbase{transaction::read(Gigamonkey::write(Part1.size() + Part2.size() + 12, Part1, nonce1, nonce2, Part2))};
            }
        };
        
        struct constructor {
            virtual list<Bitcoin::output> outputs(satoshi coinbase_value) const = 0;
            
            incomplete operator()(const bitcoind::rpc::getminingcandidate::response& r) const {
                return { 
                    part_1(r.height()), 
                    part_2(outputs(r.coinbaseValue())) 
                };
            }
        };
        
        struct standard : constructor {
            address MinerAddress;
            list<Bitcoin::output> outputs(satoshi coinbase_value) const override;
        };
        
    };
    
    struct puzzle {
        bitcoind::rpc::getminingcandidate::response GetMiningCandidateResponse;
        coinbase::incomplete IncompleteCoinbase;
        
        puzzle(const bitcoind::rpc::getminingcandidate::response& r, const coinbase::constructor& x) : 
            GetMiningCandidateResponse{r}, IncompleteCoinbase{x(r)} {}
        
        explicit operator work::puzzle() const;
        
        bool valid() const {
            return GetMiningCandidateResponse.valid() && IncompleteCoinbase.complete(0, 0).valid();
        }
    };
    
    struct proof : work::proof {
        const bitcoind::rpc::submitminingsolution::request SubmitMiningSolutionRequest;
        
        proof(const puzzle& p, const work::solution& x) : 
            work::proof{work::puzzle(p), x}, 
            SubmitMiningSolutionRequest{
                p.GetMiningCandidateResponse.id(), 
                x.Share.Nonce, 
                p.IncompleteCoinbase.complete(x.ExtraNonce1, x.Share.ExtraNonce2), 
                x.Share.Timestamp, 
                work::proof::Puzzle.Candidate.Category} {}
    };
    
    bool inline coinbase::valid() const {
        return transaction::valid() && transaction::Inputs.size() == 1 && transaction::Inputs.first().Outpoint == outpoint::coinbase();
    }
    
    inline puzzle::operator work::puzzle() const {
        return work::puzzle{work::candidate(GetMiningCandidateResponse), IncompleteCoinbase.Part1, IncompleteCoinbase.Part2};
    }
}

#endif
