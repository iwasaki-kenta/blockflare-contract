#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;
using namespace std;

class blockflare : contract {
public:
    blockflare(account_name owner) : contract(owner) {}

    void request(string data, string message, string proof) {
        checksum256 checksum;
        sha256(const_cast<char *>(message.c_str()), message.length(), &checksum);
        checksum256 proof_checksum = decode_checksum(proof);

        eosio_assert(checksum == proof_checksum, "Checksum of message is not equivalent to proof of work.");

        auto dfficulty_prefix = difficulty_prefix();
        auto proof_prefix = proof.substr(0, difficulty);

        print_f("Difficulty: % - % - Proof: % - Proof dfficulty_prefix: % - Generated dfficulty_prefix: % - ",
                difficulty,
                proof, proof_prefix,
                dfficulty_prefix);

        if (proof_prefix == dfficulty_prefix) {
            request_queue.push_back(queued_request{data});
            print("Proof is correct. Queued up request for relayers.");
        } else {
            print("Proof is incorrect. Failed to queue up request for relayers.");
        }
    }

    void register_relayer(account_name sender) {
        relayers[sender] = relayer{};
    }

private:
    struct queued_request {
        string data;
    };

    struct relayer {
        uint64_t balance = 0;
    };

    vector<queued_request> request_queue;
    map<account_name, relayer> relayers;
    int difficulty = 1;

    string difficulty_prefix() {
        string prefix;
        for (int i = 0; i < difficulty; i++) prefix += "0";
        return prefix;
    }

    checksum256 decode_checksum(string hex) {
        checksum256 buffer;

        int index = 0;
        while (index < hex.length()) {
            char c = hex[index];

            int value = 0;
            if (c >= '0' && c <= '9')
                value = (c - '0');
            else if (c >= 'A' && c <= 'F')
                value = (10 + (c - 'A'));
            else if (c >= 'a' && c <= 'f')
                value = (10 + (c - 'a'));

            buffer.hash[(index / 2)] += value << (((index + 1) % 2) * 4);
            index++;
        }

        return buffer;
    }
};

EOSIO_ABI(blockflare, (request))