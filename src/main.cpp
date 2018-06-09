#include <eosiolib/eosio.hpp>

using namespace eosio;
using namespace std;

class blockflare : contract {
public:
    blockflare(account_name owner) : contract(owner) {}

    void request(string data, string proof) {
        auto prefix = difficulty_prefix();
        auto proofPrefix = proof.substr(0, difficulty);

        print_f("Difficulty: % - Proof: % - Proof prefix: % - Generated prefix: % - ", difficulty, proof, proofPrefix, prefix);

        if (proofPrefix == prefix) {
            request_queue.push_back(queued_request{data});
            print("Proof is correct. Queued up request for relayers.");
        } else {
            print("Proof is incorrect. Failed to queue up request for relayers.");
        }
    }

private:
    struct queued_request {
        string data;
    };
    vector<queued_request> request_queue;
    int difficulty = 1;

    string difficulty_prefix() {
        string prefix;
        for (int i = 0; i < difficulty; i++) prefix += "0";
        return prefix;
    }
};

EOSIO_ABI(blockflare, (request))