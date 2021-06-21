# Instructions on how to run Phylourny

To use Phylourny to predict the UEFA 2020 tournament, follow these steps:

1. Write the teams down in a `teams.ini`:
  - The teams need to be in "bracket order". This means that if the tournament tree is ((a,b),(c,d)), the teams should
    be listed as `a, b, c, d`.
2. Use the `teams.ini` to produce the pairwise win probability matrix:
  - Do this with the python script in `docs/UEFA`
  - This script takes the `teams.ini` and the `formatted.json` on the command line with the flags `--teams` and `--json`
    respectively.
  - The output is specified with the `--output` flag. The output will be a CSV file that can be passed to Phylourny
3. Write down the match history in a `matches.csv`:
  - There is an example named `match-history-example.csv` in the `examples` folder.
4. Run Phylourny:
  - The required arguments are `--teams` and `--prefix`
  - `--teams` takes the `teams.ini` file from earlier.
  - **The prefix does not work with directories right now.**
  - Relevant optional arguments are:
    - `--matches`, which takes the match history from earlier.
    - `--probs`, which takes the csv file produced by the python script from step 2.
  - There will be 3 files for output:
    - `<PREFIX>.mlp.json` which contains the output of the ML prediction.
    - `<PREFIX>.mmpp.json` which contains the output of the maximum marginal posterior prediction.
    - `<PREFIX>.probs.json` which contains the output of the tournament evaluation using the provided `probs` file
  - Each output file will be a WPV with entries in the order of `teams.ini`.
