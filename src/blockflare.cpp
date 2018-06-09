//
// Created by kenta on 6/9/18.
//

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>

using namespace eosio;
using namespace std;

//@abi table accounts i64
struct Account {
    account_name owner;
    uint64_t balance = 0;

    uint64_t primary_key() const {
        return owner;
    }

    EOSLIB_SERIALIZE(Account, (owner)(balance))
};

//@abi table endpoints i64
struct Endpoint {
    string url;

    uint64_t primary_key() const {
        return N(url);
    }

    EOSLIB_SERIALIZE(Endpoint, (url))
};

//@abi table reqs i64
struct Request {
    uint64_t id;
    string url;
    account_name sender;
    string request;
    string response;
    vector<account_name> relayers;

    uint64_t primary_key() const {
        return id;
    }

    EOSLIB_SERIALIZE(Request, (id)(url)(sender)(request)(response)(relayers))
};

class blockflare : contract {

public:
    blockflare(account_name owner) : contract(owner), accounts(_self, _self), endpoints(_self, _self),
                                     requests(_self, _self) {}

    //@abi action
    void request(account_name sender, string url, string message, string nonce, string proof) {
        auto endpoint = endpoints.get(N(url), "Endpoint is not registered in Blockflare.");
        string serialized = message + nonce;

        checksum256 checksum;
        sha256(const_cast<char *>(serialized.c_str()), serialized.length(), &checksum);
        checksum256 proof_checksum = decode_checksum(proof);

        eosio_assert(checksum == proof_checksum, "Checksum of message is not equivalent to proof of work.");

        auto diff_prefix = difficulty_prefix();
        auto proof_prefix = proof.substr(0, difficulty);

        eosio_assert(proof_prefix == diff_prefix, "Proof of work does not match up to current contract's difficulty.");

        create_account(sender);

        requests.emplace(_self, [&](Request &req) {
            req.id = requests.available_primary_key();
            req.sender = sender;
            req.url = url;
            req.request = message;
        });

        print("Success.");
    }

    //@abi action
    void newendpoint(account_name creator, string url) {
        eosio_assert(endpoints.find(N(url)) == endpoints.end(), "Endpoint already exists.");

        create_account(creator);

        auto endpoint = endpoints.emplace(_self, [&](Endpoint &endpoint) {
            endpoint.url = url;
        });
    }

    //@abi action
    void delendpoint(string url) {
        eosio_assert(endpoints.find(N(url)) != endpoints.end(), "Endpoint doesn't exist.");

        endpoints.erase(endpoints.find(N(url)));
    }

    //@abi action
    void assign(account_name relayer) {
        create_account(relayer);

        for (const Request& req : requests) {
            // Get all requests with less than 3 relayers, and an empty response.
            if (req.relayers.size() < 3 && req.response.empty()) {
                requests.modify(req, _self, [&](Request & req) {
                    req.relayers.push_back(relayer);
                });
                break;
            }
        }
    }

    //@abi action
    void respond(account_name relayer, uint64_t requestId, string response) {
        auto request = requests.get(requestId, "Unable to find request to respond to.");

        requests.modify(request, _self, [&](Request &req) {
            req.response = response;
        });
    }


private:
    int difficulty = 4;

    typedef multi_index<N(accounts), Account> accounts_index;
    typedef multi_index<N(endpoints), Endpoint> endpoints_index;
    typedef multi_index<N(reqs), Request> requests_index;

    accounts_index accounts;
    endpoints_index endpoints;
    requests_index requests;


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

    void create_account(account_name owner) {
        if (accounts.find(owner) == accounts.end()) {
            accounts.emplace(owner, [&](Account &account) {
                account.owner = owner;
            });
        }
    }
};

EOSIO_ABI(blockflare, (request)(newendpoint)(delendpoint)(assign)(respond))