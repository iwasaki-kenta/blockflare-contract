eosiocpp -o blockflare/blockflare.wast src/*.cpp
eosiocpp -g blockflare/blockflare.abi src/*.cpp

cleos set contract blockflare blockflare/ -p blockflare