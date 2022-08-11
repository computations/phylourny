#!/usr/bin/env python3

import numpy
import seaborn
import pandas
import ijson
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--csv", type=str, required=True)
parser.add_argument("--field", type=str, required=True)
parser.add_argument("--output", type=str, required=True)
parser.add_argument("--percentile", type=float, default=99.9)
parser.add_argument("--size", nargs="+", default=[9, 5], type=float)
parser.add_argument("--title", type=str)
parser.add_argument("--x-label", type=str)
parser.add_argument("--y-label", type=str)
args = parser.parse_args()

df = pandas.read_csv(args.csv)
llh_percentile = numpy.percentile(df["llh"], args.percentile)
reduced_df = df.loc[df["llh"] > llh_percentile]

if reduced_df.empty:
    print(
        "Warning, dataframe is empty after reduction, maybe too high of a percentile?"
    )

teams = reduced_df["team"].unique()

sorted_teams = sorted(
    teams,
    key=lambda a: reduced_df.loc[reduced_df.team == a, args.field].median(),
    reverse=True,
)


# seaborn.set(rc={"figure.figsize": (9, 12)})
seaborn.set_style("ticks")

fig = seaborn.boxplot(
    data=reduced_df,
    x=args.field,
    y="team",
    palette="deep",
    order=sorted_teams,
    showfliers=True,
    fliersize=2.5,
)
fig.figure.set_size_inches(*args.size)
if not args.title is None:
    fig.set(title=args.title)

if not args.x_label is None:
    fig.set(xlabel=args.x_label)

if not args.y_label is None:
    fig.set(ylabel=args.y_label)

fig.figure.savefig(args.output, bbox_inches="tight", dpi=1000)
