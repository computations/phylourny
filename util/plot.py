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
args = parser.parse_args()

df = pandas.read_csv(args.csv)
llh_percentile = numpy.percentile(df["llh"], args.percentile)
reduced_df = df.loc[df["llh"] > llh_percentile]

if reduced_df.empty:
    print(
        "Warning, dataframe is empty after reduction, maybe too high of a percentile?"
    )

# seaborn.set(rc={"figure.figsize": (9, 12)})
seaborn.set_style("ticks")

fig = seaborn.boxplot(data=reduced_df, x=args.field, y="team", palette="deep")
fig.figure.set_size_inches(8, 5)
fig.figure.savefig(args.output, bbox_inches="tight")
