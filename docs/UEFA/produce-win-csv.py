import json
import csv
import itertools
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--json", type=str, required=True)
parser.add_argument("--teams", type=str, required=True)
parser.add_argument("--output", type=str, required=True)
args = parser.parse_args()

with open(args.json, 'r') as f:
    data = json.load(f)

with open(args.teams, 'r') as f:
    knockout_teams = f.read().split('\n')

all_teams = data['teams']

rows = []

for t1, t2 in itertools.product(enumerate(all_teams), enumerate(all_teams)):
    if t1[1] not in knockout_teams or t2[1] not in knockout_teams:
        continue

    if t1[1] == t2[1]:
        continue

    win_prob = data['data'][t1[0]][t2[0]]

    if win_prob < .5:
        continue

    rows.append({
        'team1': t1[1],
        'team2': t2[1],
        'prob-win-team1': win_prob,
    })

with open(args.output, 'w') as f:
    csv_writer = csv.DictWriter(f, ['team1', 'team2', 'prob-win-team1'])
    csv_writer.writeheader()
    for r in rows:
        csv_writer.writerow(r)
