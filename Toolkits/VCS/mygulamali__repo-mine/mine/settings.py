import os

from dotenv import load_dotenv, find_dotenv
from github import Github
from peewee import SqliteDatabase

load_dotenv(find_dotenv())

DATABASE = SqliteDatabase(os.getenv("DATABASE", None))
GITHUB_CLIENT = Github(os.getenv("GITHUB_API_TOKEN", None))
GITHUB_ORGANIZATION = os.getenv("GITHUB_ORGANIZATION", None)
