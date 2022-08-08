#!/usr/bin/env python3

import ijson
import csv
import argparse
import json

parser = argparse.ArgumentParser()
parser.add_argument("--results", type=str, required=True)
parser.add_argument("--teams", type=str, required=True)
parser.add_argument("--output", type=str, required=True)
parser.add_argument("--field", type=str, required=True)
args = parser.parse_args()

teams_json = json.load(open(args.teams))

inv_name_map = {v: k for k, v in teams_json["team-name-map"].items()}

results_json_file = open(args.results)

writer = csv.DictWriter(open(args.output, "w"), ["llh", "team", args.field])

writer.writeheader()

for iters, item in enumerate(ijson.items(results_json_file, "item")):
    for i, wp in enumerate(item[args.field]):
        name = inv_name_map[i]

        row = {
            "llh": item["llh"],
            "team": name,
            args.field: 0.0 if wp is None else float(wp),
        }
        writer.writerow(row)
