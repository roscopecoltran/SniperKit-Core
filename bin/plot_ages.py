#!/usr/bin/env python

import matplotlib.pyplot as plt
from numpy import append, arange

from mine.helpers import *
from mine.models import *

def age_of(repo):
    dt = repo.pushed_at - repo.commits[0].committed_at
    return (dt.days + dt.seconds / 86400.0) / 365.2425


def repo_ages(private):
    return [
        age_of(repo)
        for repo in Repo.select().join(Commit).where(Repo.private == private)
    ]


def year_bins(years, per_year=0):
    if per_year == 0:
        return range(years + 1)
    else:
        return append(
            arange(years * per_year) / float(per_year), years
        ).tolist()


if __name__ == "__main__":
    plt.xkcd()
    fig = plt.figure(figsize=[8, 8])

    ax = fig.add_subplot(2, 1, 1)
    format_plot_axes(ax)
    plt.hist(repo_ages(False), bins=year_bins(10, 2), facecolor="b")
    plt.title("Number of repos by age and type")
    plt.ylabel("Number of public repos")

    ax = fig.add_subplot(2, 1, 2)
    format_plot_axes(ax)
    plt.hist(repo_ages(True), bins=year_bins(10, 2), facecolor="r")
    plt.xlabel("Years")
    plt.ylabel("Number of private repos")

    plt.tight_layout()
    plt.savefig("ages.png")
