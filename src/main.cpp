#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;
using namespace std;

class blockflare : contract {
public:
    blockflare(account_name owner) : contract(owner) {}

    void request(string data, string nonce, string proof) {
        string message = data + nonce;

        checksum256 checksum;
        sha256(const_cast<char *>(message.c_str()), message.length(), &checksum);
        checksum256 proof_checksum = decode_checksum(proof);

        eosio_assert(checksum == proof_checksum, "Checksum of message is not equivalent to proof of work.");

        auto diff_prefix = difficulty_prefix();
        auto proof_prefix = proof.substr(0, difficulty);

        print_f("Difficulty: % - Message: % - Proof: % - Proof prefix: % - Generated prefix: % - ",
                difficulty,
                message,
                proof, proof_prefix,
                diff_prefix);

        eosio_assert(proof_prefix == diff_prefix, "Proof of work does not match up to current contract's difficulty.");

        request_queue.push_back(queued_request{data});
        print("Success.");
    }

    void relayjoin(account_name sender) {
        relayers[sender] = relayer{};
    }

    void relayleave(account_name sender) {
        relayers.erase(sender);
    }

    // Called by a relayer. Delegate a random available queued request to him.
    void relayassign(account_name sender) {
        auto user = relayers.find(sender);
        eosio_assert(user == relayers.end(), "Relayer is not registered in the system.");

        for (int i = 0; i < request_queue.size(); i++) {
            // Maximum of 3 relayers.
            if (request_queue[i].assignees.size() < 3) {
                request_queue[i].assignees.push_back(user->second);
                print("Assigned to request ", i);
                return;
            }
        }
    }

private:
    struct relayer {
        uint64_t balance = 0;
    };

    struct queued_request {
        // JSON request.
        string req;

        // JSON response.
        string res;

        // Relayers working on the task.
        vector<relayer> assignees;
    };

    vector<queued_request> request_queue;
    map<account_name, relayer> relayers;
    int difficulty = 3;

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

EOSIO_ABI(blockflare, (request)(relayjoin)(relayleave)(relayassign))