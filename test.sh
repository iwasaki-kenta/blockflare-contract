cleos wallet unlock --password "PW5K8pRqhwJ49fhMLP8228n9QB4Qnj1evBCFcSAMR2QPWiQQWoopV"

cleos push action blockflare delendpoint '["google.com"]' -p blockflare
cleos push action blockflare newendpoint '["blockflare", "google.com"]' -p blockflare
#cleos push action blockflare relayjoin '["blockflare"]' -p blockflare
cleos push action blockflare request '["blockflare", "google.com", "this_is_test_data", "98556171739.85435", "0000ae441ff5845dc75dbc33f0eee774b3ef0b55204c4b66337580b3abbe97e5"]' -p blockflare
cleos push action blockflare assign '["blockflare"]' -p blockflare
