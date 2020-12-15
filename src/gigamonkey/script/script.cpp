// Copyright (c) 2019-2020 Daniel Krawisz
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#include <gigamonkey/script/script.hpp>

#include <sv/script/interpreter.h>
#include <sv/taskcancellation.h>
#include <sv/streams.h>
#include <sv/policy/policy.h>
#include <sv/version.h>
#include <sv/script_config.h>

namespace Gigamonkey::Bitcoin {
    
    struct script_config : CScriptConfig {
        uint64_t GetMaxOpsPerScript(bool isGenesisEnabled, bool isConsensus) const {
            throw 0;
        }
        
        uint64_t GetMaxScriptNumLength(bool isGenesisEnabled, bool isConsensus) const {
            throw 0;
        }
        
        uint64_t GetMaxScriptSize(bool isGenesisEnabled, bool isConsensus) const {
            throw 0;
        }
        
        uint64_t GetMaxPubKeysPerMultiSig(bool isGenesisEnabled, bool isConsensus) const {
            throw 0;
        }
        
        uint64_t GetMaxStackMemoryUsage(bool isGenesisEnabled, bool isConsensus) const {
            throw 0;
        }
        
        script_config() {
            throw 0;
        }
    };
    
    evaluated evaluate_script(const script& unlock, const script& lock, const BaseSignatureChecker& checker) {
        evaluated Response;
        std::optional<bool> response = VerifyScript(
            script_config{}, // Config. 
            false, // true for consensus rules, false for policy rules.  
            task::CCancellationSource::Make()->GetToken(), 
            CScript(unlock.begin(), unlock.end()), 
            CScript(lock.begin(), lock.end()), 
            StandardScriptVerifyFlags(true, true), // Flags. I don't know what these should be. 
            checker, 
            &Response.Error);
        if (response.has_value()) {
            Response.Return = *response;
        } 
        return Response;
    }
    
    class DummySignatureChecker : public BaseSignatureChecker {
    public:
        DummySignatureChecker() {}

        bool CheckSig(const std::vector<uint8_t> &scriptSig,
                    const std::vector<uint8_t> &vchPubKey,
                    const CScript &scriptCode, bool enabledSighashForkid) const override {
            return true;
        }
    };
    
    evaluated evaluate_script(const script& unlock, const script& lock) {
        return evaluate_script(unlock, lock, DummySignatureChecker{});
    }
    
    evaluated evaluate_script(const script& unlock, const script& lock, const input_index& transaction) {
        // transaction needs to be made into some stream but I don't know what that is. It's a
        // template parameter in this constructor. 
        CDataStream stream{static_cast<const std::vector<uint8_t>&>(transaction.Transaction), SER_NETWORK, PROTOCOL_VERSION};
        CTransaction tx{deserialize, stream}; 
        return evaluate_script(lock, unlock, TransactionSignatureChecker(&tx, transaction.Index, Amount(int64(transaction.Output.Value))));
    }

}

