#/usr/bin/env python3

import os
import math
import string
import io
import math
import numpy
import scipy
import argparse
import subprocess
import itertools
import copy
import random
import csv
import json
from multiprocessing.pool import ThreadPool
import multiprocessing
import pandas
import seaborn
import matplotlib

parser = argparse.ArgumentParser()
parser.add_argument('--iters', default=1000, type=int)
parser.add_argument('--teams')
parser.add_argument('--win-probs')
parser.add_argument('--exp-prefix', required=True)
parser.add_argument('--program', required=True)
parser.add_argument('--matches')
parser.add_argument('--single', action='store_true', default=False)
args = parser.parse_args()


def base26_generator(maximum):
    if maximum == 1:
        yield 'a'
    iters = math.ceil(math.log(maximum, 26))
    for index in range(maximum):
        bases = [
            string.ascii_lowercase[(index % (26**(e + 1))) // (26**e)]
            for e in range(iters)
        ]
        bases.reverse()
        yield ''.join(bases)


class WinProbs:
    _teams = None
    _winprobs = None
    _params = None

    def __init__(self, **kwargs):
        if 'teams' in kwargs and 'win_probs_csv' in kwargs:
            self._init_from_csv(kwargs['teams'], kwargs['win_probs_csv'])
        elif 'params' in kwargs:
            self._init_from_model(kwargs['teams'], kwargs['params'])

    def _init_from_model(self, teams, params):
        self._teams = teams
        self._winprobs = numpy.zeros((len(self._teams), len(self._teams)))
        self._params = params
        for t1, t2 in itertools.combinations(self._teams, 2):
            t1_index = self._teams.lookup(t1)
            t2_index = self._teams.lookup(t2)
            r1 = params[t1_index]
            r2 = params[t2_index]

            l1 = numpy.exp(r1 - r2)
            l2 = numpy.exp(r2 - r1)

            t1_prob = scipy.stats.skellam.cdf(-1, l2, l1)
            t2_prob = scipy.stats.skellam.cdf(-1, l1, l2)

            tie_prob = 1 - t1_prob - t2_prob

            t1_prob += tie_prob / 2.0
            t2_prob += tie_prob / 2.0

            self._winprobs[t1_index][t2_index] = t1_prob
            self._winprobs[t2_index][t1_index] = t2_prob

    def _init_from_csv(self, teams, win_probs_csv):
        self._teams = teams
        self._winprobs = numpy.zeros((len(self._teams), len(self._teams)))
        for row in win_probs_csv:
            t1_index = self._teams.lookup(row['team1'])
            t2_index = self._teams.lookup(row['team2'])
            wp = float(row['prob-win-team1'])

            self._winprobs[t1_index][t2_index] = wp
            self._winprobs[t2_index][t1_index] = 1 - wp

    def perturb(self, sigma=0.01):
        if not self._params is None:
            self._perturb_params(sigma)
        else:
            self._perturb_win_probs(sigma)

    def _perturb_params(self, sigma=0.1):
        params = []
        for p in self._params:
            delta = numpy.random.randn() * sigma
            params.append(p + delta)
        self._init_from_model(teams=self._teams, params=params)

    def _perturb_win_probs(self, sigma=0.1):
        for t1_index, t2_index in itertools.combinations(
                range(len(self._teams)), 2):
            delta = numpy.random.randn() * sigma
            self._winprobs[t1_index][t2_index] += delta
            self._winprobs[t2_index][t1_index] -= delta

    def iter_index(self):
        for t1_index, t2_index in itertools.combinations(
                range(len(self._teams)), 2):
            yield (t1_index, t2_index, self._winprobs[t1_index][t2_index],
                   self._winprobs[t2_index][t1_index])

    def iter_string(self):
        for tp1, tp2 in itertools.combinations(enumerate(self._teams), 2):
            t1_index, t1 = tp1
            t2_index, t2 = tp2
            yield (t1, t2, self._winprobs[t1_index][t2_index],
                   self._winprobs[t2_index][t1_index])

    def check(self):
        for t1_index, t2_index, wp1, wp2 in self.iter_index():
            if not abs(wp1 + wp2 - 1.0) < 1e-10:
                return False
        return True

    def clone(self):
        return copy.deepcopy(self)

    @property
    def teams(self):
        return self._teams

    def to_buffer(self, filebuffer):
        writer = csv.DictWriter(
            filebuffer, fieldnames=['team1', 'team2', 'prob-win-team1'])
        writer.writeheader()
        for t1, t2, wp, _ in self.iter_string():
            writer.writerow({"team1": t1, "team2": t2, "prob-win-team1": wp})

    def to_string(self):
        string_buffer = io.StringIO()
        self.to_buffer(string_buffer)
        return string_buffer.getvalue()


class Teams:
    _teams = None

    def __init__(self, teams):
        self._teams = teams

    def randomize(self):
        return Teams(random.sample(self._teams, len(self._teams)))

    def __getitem__(self, index):
        return self._teams[index]

    def __len__(self):
        return len(self._teams)

    def append(self, team):
        self._teams.append(team)

    def lookup(self, team):
        for i, t in enumerate(self):
            if t == team:
                return i
        return None

    def to_string(self):
        string_buffer = io.StringIO()
        self.to_buffer(string_buffer)
        return string_buffer.getvalue()

    def to_buffer(self, filebuffer):
        for t in self._teams:
            filebuffer.write(t + "\n")

    def __repr__(self):
        return repr(self._teams)


class Experiment:
    _team_filename = "teams.ini"
    _win_probs_filename = "win-probs.csv"

    def __init__(self, wp, path):
        self._winprobs = wp.clone()
        self._path = path

    @property
    def teams_file_path(self):
        return os.path.join(self._path, self._team_filename)

    @property
    def win_probs_file_path(self):
        return os.path.join(self._path, self._win_probs_filename)

    @property
    def pyhlourny_prefix(self):
        return os.path.join(self._path, "experiment")

    @property
    def phylourny_logfile_path(self):
        return os.path.join(self._path, "output.log")

    @property
    def phylourny_results_path(self):
        return self.pyhlourny_prefix + ".dynamic.probs.json"

    @property
    def path(self):
        return self._path

    def makedir(self):
        os.makedirs(self._path)

    def write(self):
        with open(self.teams_file_path, 'w') as teamsfile:
            self._winprobs.teams.to_buffer(teamsfile)

        with open(self.win_probs_file_path, 'w') as wpfile:
            self._winprobs.to_buffer(wpfile)

    def setup(self):
        self._winprobs.perturb()

    def read_time(self):
        with open(self.phylourny_logfile_path) as logfile:
            for line in logfile:
                if "Run Finished, time:" in line:
                    return self._parse_time(line)

    def read_results(self):
        with open(self.phylourny_results_path) as results_file:
            results_json = json.load(results_file)
            results = {}
            for t, wp in zip(self._winprobs.teams, results_json):
                results[t] = wp

            return results

    @staticmethod
    def _parse_time(line):
        time_string = line.split()[-1].strip()
        return float(time_string[:-1])


class ExperimentList:
    def __init__(self, wp, iters, prefix):
        self._experiments = [
            Experiment(wp, os.path.join(prefix, name))
            for name in base26_generator(iters)
        ]

    def _makedirs(self):
        for e in self._experiments:
            e.makedir()

    def _makedirs_mp(self):
        with ThreadPool(6) as p:
            p.map(Experiment.makedir, self._experiments)

    def _runall(self, prog):
        for e in self._experiments:
            prog.run(e)

    def _runall_mp(self, prog):
        with multiprocessing.Pool() as p:
            p.starmap(Phylourny.run,
                      itertools.product([prog], self._experiments))

    def _writeall(self):
        self._makedirs()
        for e in self._experiments:
            e.write()

    def _writeall_mp(self):
        self._makedirs_mp()
        with ThreadPool() as p:
            p.map(Experiment.write, self._experiments)

    def _setup(self):
        for e in self._experiments:
            e.setup()

    def run(self, prog):
        self._setup()
        self._writeall_mp()
        self._runall_mp(prog)

    def results(self):
        results = []
        for e in self._experiments:
            exp_dict = {
                'time': e.read_time(),
                'result': e.read_results(),
                'path': e.path
            }
            results.append(exp_dict)

        return results

    def summary(self):
        results = {}
        times = []
        for e in self._experiments:
            times.append(e.read_time())
            e_results = e.read_results()
            for k, v in e_results.items():
                if not k in results:
                    results[k] = []
                results[k].append(v)

        means = {}
        std = {}
        for k, v in results.items():
            means[k] = numpy.mean(v)
            std[k] = numpy.std(v)

        return {
            'average': means,
            'std': std,
            'average-time': numpy.mean(times)
        }

    def pandas_df(self):
        df = pandas.DataFrame()
        for e in self._experiments:
            exp_dict = {'time': e.read_time(), 'path': e.path}
            results = e.read_results()
            for k, v in results.items():
                exp_dict[k] = v

            df = df.append(exp_dict, ignore_index=True)

        return df


class Phylourny:
    _teams = "--teams {}"
    _prefix = "--prefix {}"
    _seed = "--seed {}"
    _matches = "--matches {}"
    _odds = "--odds {}"
    _probs = "--probs {}"
    _single = "--single"
    _sim = "--sim"
    _sim_iters = "--sim-iters {}"
    _dynamic = "--dynamic {}"
    _samples = "--samples {}"
    _burnin = "--burnin {}"
    _dummy = "--dummy {}"
    _debug = "--debug {}"

    def __init__(self, program_path):
        self._program_path = program_path
        self._args = [self._program_path]

    def run(self, exp):
        args = copy.deepcopy(self._args)
        args.extend(self._teams.format(exp.teams_file_path).split())
        args.extend(self._prefix.format(exp.pyhlourny_prefix).split())
        args.extend(self._probs.format(exp.win_probs_file_path).split())

        with open(exp.phylourny_logfile_path, 'w') as logfile:
            subprocess.run(args, stdout=logfile, stderr=logfile)

    def set_dynamic(self, on):
        self._args.extend(self._dynamic.format("on" if on else "off").split())

    def set_single(self):
        self._args.append(self._single)


class Match:
    _team1 = None
    _team2 = None
    _team1_index = None
    _team2_index = None
    _goals_team1 = None
    _goals_team2 = None

    def __init__(self, match_row, teams):
        self._team1 = match_row['team1'].strip()
        if not self._team1 in teams:
            teams.append(self._team1)

        self._team2 = match_row['team2'].strip()
        if not self._team2 in teams:
            teams.append(self._team2)

        self._team1_index = teams.lookup(self._team1)
        self._team2_index = teams.lookup(self._team2)
        self._goals_team1 = int(match_row['team1-goals'])
        self._goals_team2 = int(match_row['team2-goals'])

    def sublh(self, params):
        p1 = params[self._team1_index]
        p2 = params[self._team2_index]
        lambda_left = numpy.exp(p1 - p2)
        lambda_right = numpy.exp(p2 - p1)

        term_left = (
            (lambda_left**self._goals_team1) /
            math.factorial(self._goals_team1)) * numpy.exp(-lambda_left)

        term_right = (
            (lambda_right**self._goals_team2) /
            math.factorial(self._goals_team2)) * numpy.exp(-lambda_right)

        return term_left * term_right


class MatchList:
    _matches = None
    _teams = None

    def __init__(self, match_file):
        self._matches = []
        self._teams = Teams([])
        for line in match_file:
            self._matches.append(Match(line, self._teams))

    def llh(self, params):
        llh = 0
        for m in self._matches:
            llh += math.log(m.sublh(params))
        return llh

    @property
    def teams(self):
        return self._teams


class PoissonModel:
    _params = None
    _match_list = None

    def __init__(self, match_file):
        self._match_list = MatchList(match_file)
        # We have the constraint that the parameters sum to 0. This makes it a
        # translated simplex, so we can reduce the number of parameters by 1.
        self._params = numpy.zeros(len(self._match_list.teams) - 1)

    @staticmethod
    def expand_params(params):
        expanded_params = []
        reg = 0.0
        for p in params:
            expanded_params.append(p)
            reg += p

        expanded_params.append(-reg)

        return numpy.array(expanded_params)

    def select_params(self, select_teams):
        selected_params = []
        expanded_params = self.expand_params(self._params)
        for t in select_teams:
            index = self._match_list.teams.lookup(t)
            selected_params.append(expanded_params[index])

        return selected_params

    def optimize(self):
        opt_fun = lambda p: -self._match_list.llh(self.expand_params(p))

        result = scipy.optimize.minimize(opt_fun, self._params)
        self._params = result.x

    def win_probs(self, selected_teams):
        return WinProbs(teams=selected_teams,
                        params=self.select_params(selected_teams))


def read_teams_file(teams_file_path):
    with open(teams_file_path) as infile:
        teams = []
        for line in infile:
            teams.append(line.strip())
        teams = Teams(teams)
    return teams


def perturb_win_probs(args):
    teams = read_teams_file(args.teams)

    with open(args.win_probs) as infile:
        reader = csv.DictReader(infile)
        wp = WinProbs(teams, reader)

    args.exp_prefix = os.path.abspath(args.exp_prefix)
    args.program = os.path.abspath(args.program)

    prog = Phylourny(args.program)

    el = ExperimentList(wp, args.iters, args.exp_prefix)
    el.run(prog)

    with open(os.path.join(args.exp_prefix, "results.json"),
              'w') as results_file:
        json.dump(el.results(), results_file)

    with open(os.path.join(args.exp_prefix, "summary.json"),
              'w') as results_file:
        json.dump(el.summary(), results_file)

    df = el.pandas_df()

    df_rank = df.loc[:, teams[0]:teams[-1]].rank(axis=1, ascending=False)

    tmp = []

    for index, row in df_rank.iterrows():
        for k, v in row.items():
            tmp.append({'team': k, 'rank': v})

    df_plots_rank = pandas.DataFrame(tmp)

    tmp = []

    for index, row in df.loc[:, teams[0]:teams[-1]].iterrows():
        for k, v in row.items():
            tmp.append({'team': k, 'prob': v})

    df_plots_prob = pandas.DataFrame(tmp)

    seaborn.set(rc={'figure.figsize': (10, 14)})

    plot = seaborn.boxplot(data=df_plots_rank, x='team', y='rank')
    matplotlib.pyplot.xticks(rotation=90)
    plot.set_yticks(range(1, 17))
    plot.set_yticklabels([str(i) for i in range(1, 17)])
    plot.set_xlabel("Team")
    plot.set_ylabel("Rank")
    plot.figure.savefig(os.path.join(args.exp_prefix, "rank.boxplot.png"))

    matplotlib.pyplot.clf()

    plot = seaborn.violinplot(data=df_plots_prob, x='team', y='prob')
    matplotlib.pyplot.xticks(rotation=90)
    plot.set_xlabel("Team")
    plot.set_ylabel("Probability")
    plot.figure.savefig(os.path.join(args.exp_prefix, "prob.violinplot.png"))


def perturb_model(args):
    with open(args.matches) as match_file:
        reader = csv.DictReader(match_file)
        model = PoissonModel(reader)

    teams = read_teams_file(args.teams)

    model.optimize()
    wp = model.win_probs(teams)

    args.exp_prefix = os.path.abspath(args.exp_prefix)
    args.program = os.path.abspath(args.program)

    prog = Phylourny(args.program)
    if (args.single):
        prog.set_dynamic(False)
        prog.set_single()

    el = ExperimentList(wp, args.iters, args.exp_prefix)
    el.run(prog)

    with open(os.path.join(args.exp_prefix, "results.json"),
              'w') as results_file:
        json.dump(el.results(), results_file)

    with open(os.path.join(args.exp_prefix, "summary.json"),
              'w') as results_file:
        json.dump(el.summary(), results_file)

    df = el.pandas_df()

    df_rank = df.loc[:, teams[0]:teams[-1]].rank(axis=1, ascending=False)

    tmp = []

    for index, row in df_rank.iterrows():
        for k, v in row.items():
            tmp.append({'team': k, 'rank': v})

    df_plots_rank = pandas.DataFrame(tmp)

    tmp = []

    for index, row in df.loc[:, teams[0]:teams[-1]].iterrows():
        for k, v in row.items():
            tmp.append({'team': k, 'prob': v})

    df_plots_prob = pandas.DataFrame(tmp)

    seaborn.set(rc={'figure.figsize': (10, 14)})

    plot = seaborn.boxplot(data=df_plots_rank, x='team', y='rank')
    matplotlib.pyplot.xticks(rotation=90)
    plot.set_yticks(range(1, 17))
    plot.set_yticklabels([str(i) for i in range(1, 17)])
    plot.set_xlabel("Team")
    plot.set_ylabel("Rank")
    plot.figure.savefig(os.path.join(args.exp_prefix, "rank.boxplot.png"))

    matplotlib.pyplot.clf()

    plot = seaborn.violinplot(data=df_plots_prob, x='team', y='prob')
    matplotlib.pyplot.xticks(rotation=90)
    plot.set_xlabel("Team")
    plot.set_ylabel("Probability")
    plot.figure.savefig(os.path.join(args.exp_prefix, "prob.violinplot.png"))


if __name__ == "__main__":
    perturb_model(args)