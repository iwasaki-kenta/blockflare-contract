cleos wallet unlock --password "PW5K8pRqhwJ49fhMLP8228n9QB4Qnj1evBCFcSAMR2QPWiQQWoopV"

eosiocpp -o blockflare/blockflare.wast src/*.cpp
eosiocpp -g blockflare/blockflare.abi src/*.cpp

cleos set contract blockflare blockflare/ -p blockflare