#!/usr/bin/env python

import matplotlib.pyplot as plt

from mine.helpers import *
from mine.models import *

def language_query(language_name, private):
    return (
        Language
        .select()
        .join(Repo)
        .where(
            Language.name == language_name,
            Repo.private == private
        )
    )


def language_count(language_name, private):
    return language_query(language_name, private).count()


def language_data(language_name):
    return (
        language_name,
        language_count(language_name, True),
        language_count(language_name, False)
    )


def plot_top_n(n, languages, private, public):
    width = 0.75

    y = range(n)
    yticks = [yi + width / 2 for yi in y]

    plt.xkcd()

    fig = plt.figure(figsize=[8, 8])
    ax = fig.add_subplot(1, 1, 1)
    format_plot_axes(ax)

    plt.barh(y, private[-n:], width, color="r", label="Private")
    plt.barh(y, public[-n:], width, color="b", label="Public")

    plt.title("Number of repos by language and type (top {0})".format(n))
    plt.xlabel("Number of repos")
    plt.legend(loc="lower right")
    plt.yticks(yticks, languages[-n:])

    plt.tight_layout()


def plot_and_save_data(filename, data):
    data.sort(key=(lambda x: x[1] + x[2]))
    data_lists = map(list, zip(*data))

    plot_top_n(10, *data_lists)
    plt.savefig(filename)


if __name__ == "__main__":
    languages = set([language.name for language in Language.select()])
    data = [language_data(language) for language in languages]
    plot_and_save_data("languages.png", data)
