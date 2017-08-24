#!/usr/bin/env python

from github.GithubException import GithubException, UnknownObjectException

from mine.helpers import print_action
from mine.models import Commit, Language, Repo
from mine.settings import *

CHUNK_SIZE = 30 # match pagination size for Github API

@print_action("Creating database")
def create_tables(database):
    database.create_tables([Commit, Language, Repo], safe=True)


def fetch_repo_languages(repo):
    try:
        return repo.get_languages()
    except UnknownObjectException:
        return {}


def fetch_repo_initial_commit(repo):
    try:
        return repo.get_commits().reversed[0].commit
    except GithubException:
        return None


def repo_data(repo):
    return (
        repo,
        fetch_repo_languages(repo),
        fetch_repo_initial_commit(repo)
    )


@print_action("Fetching data about repos from GitHub")
def fetch_repos_data(client, organization):
    return [
        repo_data(repo)
        for repo in client.get_organization(organization).get_repos()
    ]


def repos_list(data):
    return [Repo.as_dict(repo) for (repo, _, _) in data]


def languages_list(data):
    return [
        {
            "repo": repo.id,
            "name": language,
            "size_in_bytes": size,
        }
        for (repo, languages, _) in data
        for (language, size) in languages.iteritems()
    ]


def initial_commits_list(data):
    return [
        {
            "committed_at": commit.committer.date,
            "committed_by": commit.committer.name,
            "message": commit.message,
            "repo": repo.id,
            "sha": commit.sha
        }
        for (repo, _, commit) in data
        if commit is not None
    ]


@print_action("Populating database")
def populate_database(database, data):
    with database.atomic():
        for index in range(0, len(data), CHUNK_SIZE):
            chunk_slice = slice(index, index + CHUNK_SIZE)

            repos_chunk = repos_list(data[chunk_slice])
            Repo.insert_many(repos_chunk).execute()

            languages_chunk = languages_list(data[chunk_slice])
            Language.insert_many(languages_chunk).execute()

            commits_chunk = initial_commits_list(data[chunk_slice])
            Commit.insert_many(commits_chunk).execute()


if __name__ == "__main__":
    DATABASE.connect()
    create_tables(DATABASE)
    data = fetch_repos_data(GITHUB_CLIENT, GITHUB_ORGANIZATION)
    populate_database(DATABASE, data)
