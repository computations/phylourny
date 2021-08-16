#/usr/bin/env python3

import os
import math
import string
import io
import numpy
import argparse
import subprocess
import itertools
import copy
import random
import csv

parser = argparse.ArgumentParser()
parser.add_argument('--iters', default=100)
parser.add_argument('--teams', required=True)
parser.add_argument('--win-probs', required=True)
parser.add_argument('--exp-prefix', required=True)
parser.add_argument('--program', required=True)
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

    def __init__(self, teams, win_probs_csv):
        self._teams = teams
        self._winprobs = numpy.zeros((len(self._teams), len(self._teams)))
        for row in win_probs_csv:
            t1_index = self._teams.lookup(row['team1'])
            t2_index = self._teams.lookup(row['team2'])
            wp = float(row['prob-win-team1'])

            self._winprobs[t1_index][t2_index] = wp
            self._winprobs[t2_index][t1_index] = 1 - wp

    def perturb(self, sigma=0.1):
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

    def makedir(self):
        os.makedirs(self._path)

    def write(self):
        with open(self.teams_file_path, 'w') as teamsfile:
            self._winprobs.teams.to_buffer(teamsfile)

        with open(self.win_probs_file_path, 'w') as wpfile:
            self._winprobs.to_buffer(wpfile)

    def setup(self):
        self._winprobs.perturb()


class ExperimentList:
    def __init__(self, wp, iters, prefix):
        self._experiments = [
            Experiment(wp, os.path.join(prefix, name))
            for name in base26_generator(iters)
        ]

    def _makedirs(self):
        for e in self._experiments:
            e.makedir()

    def _runall(self, prog):
        for e in self._experiments:
            prog.run(e)

    def _writeall(self):
        self._makedirs()
        for e in self._experiments:
            e.write()

    def _setup(self):
        for e in self._experiments:
            e.setup()

    def run(self, prog):
        self._setup()
        self._writeall()
        self._runall(prog)


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

    def run(self, exp):
        args = [self._program_path]
        args.extend(self._teams.format(exp.teams_file_path).split())
        args.extend(self._prefix.format(exp.pyhlourny_prefix).split())
        args.extend(self._probs.format(exp.win_probs_file_path).split())

        with open(exp.phylourny_logfile_path, 'w') as logfile:
            subprocess.run(args, stdout=logfile, stderr=logfile)


with open(args.teams) as infile:
    teams = []
    for line in infile:
        teams.append(line.strip())
    teams = Teams(teams)

with open(args.win_probs) as infile:
    reader = csv.DictReader(infile)
    wp = WinProbs(teams, reader)

args.exp_prefix = os.path.abspath(args.exp_prefix)
args.program = os.path.abspath(args.program)

prog = Phylourny(args.program)

el = ExperimentList(wp, 10, args.exp_prefix)
el.run(prog)
