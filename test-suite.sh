#!/bin/bash
#

set -xue

test -f "hn_logs.tsv" || ! echo "file not found: hn_logs.tsv" || exit 1

ok() {
	echo "========== [OK:$@] =========="
	echo
}

hash_string() {
	md5sum | cut -f1 -d' '
}

# Eror cases
[ "$(./hnStat 2>&1)" == "missing argument" ]
[ "$(./hnStat top 2>&1)" == "missing argument" ]
[ "$(./hnStat distinct 2>&1)" == "missing argument" ]
[[ "$(./hnStat --hello-world 2>&1)" =~ "invalid option" ]]
[[ "$(./hnStat top 100 /dev/null --from lala 2>&1)" =~ "malformed value" ]]
[[ "$(./hnStat top 100 /dev/null --to lala 2>&1)" =~ "malformed value" ]]
ok "BAD ARGUMENTS"

# Ensure a non-existing file do not cause any issues (crashes...)
[[ "$(./hnStat distinct /dev/notfound 2>&1)" =~ 'No such file' ]]
[[ "$(./hnStat top 3 /dev/notfound 2>&1)" =~ 'No such file' ]]
ok "NOT FOUND"

# Ensure an empty file do not cause any issues
./hnStat distinct /dev/null >/dev/null
./hnStat top 10 /dev/null >/dev/null
ok "EMPTY FILE"

# Validate we basically have the expected output format (and result) as the example
expected="$(cat << EOF | hash_string
http%3A%2F%2Fwww.getsidekick.com%2Fblog%2Fbody-language-advice 6675
http%3A%2F%2Fwebboard.yenta4.com%2Ftopic%2F568045 4652
http%3A%2F%2Fwebboard.yenta4.com%2Ftopic%2F379035%3Fsort%3D1 3100
EOF
)"

got="$(./hnStat top 3 hn_logs.tsv | hash_string)"

[ "$got" == "$expected" ]
ok "BASIC TEST"

# Now a unit test
cat << EOF > test-sample
42	hello
50	world
50	one
50	two
51	three
58	four
59	five
60	four
60	one
61	three
65	two
100	ten
101	two
102	two
102	twelve
150	four
151	two
200	four
200	three
EOF

[ "$(./hnStat distinct test-sample 2>/dev/null)" == "9" ]
[ "$(./hnStat distinct --from 30 --to 300 test-sample 2>/dev/null)" == "9" ]
[ "$(./hnStat distinct --from 42 --to 200 test-sample 2>/dev/null)" == "9" ]
[ "$(./hnStat distinct --from 50 test-sample 2>/dev/null)" == "8" ]
[ "$(./hnStat distinct --from 51 test-sample 2>/dev/null)" == "7" ]
[ "$(./hnStat distinct --from 51 --to 51 test-sample 2>/dev/null)" == "1" ]
[ "$(./hnStat distinct --from 51 --to 61 test-sample 2>/dev/null)" == "4" ]

[ "$(./hnStat top 1 test-sample 2>/dev/null)" == "two 5" ]
[ "$(./hnStat top 2 test-sample 2>/dev/null | tail -n +2)" == "four 4" ]
[ "$(./hnStat top 3 test-sample 2>/dev/null | tail -n +3)" == "three 3" ]

ok "UNIT TEST"

# Valgrind simple validation
valgrind --quiet --track-origins=yes --leak-check=full --show-leak-kinds=all --error-exitcode=2 ./hnStat top 100 test-sample >/dev/null
ok "VALGRIND"

# Validate with alternate implementation in python
[ "$(./hnStat top 10 hn_logs.tsv 2>/dev/null | hash_string)" == "$(python parser.py top 10 hn_logs.tsv 2>/dev/null | hash_string)" ]
alt=1
ok "ALTERNATE IMPLEMENTATION - $((alt++))"

[ "$(./hnStat distinct hn_logs.tsv 2>/dev/null)" == "$(python parser.py distinct hn_logs.tsv 2>/dev/null)" ]
ok "ALTERNATE IMPLEMENTATION - $((alt++))"

from=$((1438387423+280113/3))
to=$(($from+280113/3))
[ "$(./hnStat distinct --from $from --to $to hn_logs.tsv 2>/dev/null)" == "$(python parser.py distinct --from $from --to $to hn_logs.tsv 2>/dev/null)" ]
ok "ALTERNATE IMPLEMENTATION - $((alt++))"

from=$((1438387423+280113/3))
to=$(($from+280113/3))
[ "$(./hnStat top 10 --from $from --to $to hn_logs.tsv 2>/dev/null | hash_string)" == "$(python parser.py top 10 --from $from --to $to hn_logs.tsv 2>/dev/null | hash_string)" ]
ok "ALTERNATE IMPLEMENTATION - $((alt++))"

from=$((1438387423+280113/2))
to=$(($from+280113/3))
[ "$(./hnStat top 10 --from $from --to $to hn_logs.tsv 2>/dev/null | hash_string)" == "$(python parser.py top 10 --from $from --to $to hn_logs.tsv 2>/dev/null | hash_string)" ]
ok "ALTERNATE IMPLEMENTATION - $((alt++))"

# Torture tests: give fat binaries and not expect a crash
for f in /usr/lib/x86_64-linux-gnu/*.so; do
	./hnStat top 10 "$f" >/dev/null 2>/dev/null
done
echo "TORTURE TESTS"

#rm -f test-sample
ok "ALL PASSED"
