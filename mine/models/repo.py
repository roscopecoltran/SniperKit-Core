from peewee import (
    BooleanField,
    CharField,
    DateTimeField,
    ForeignKeyField,
    IntegerField
)

from mine.models.base import Base

class Repo(Base):
    created_at = DateTimeField()
    description = CharField(null=True)
    fork = BooleanField(default=False)
    forks_count = IntegerField(default=0)
    has_downloads = BooleanField(default=False)
    has_issues = BooleanField(default=False)
    has_wiki = BooleanField(default=False)
    html_url = CharField()
    id = IntegerField(primary_key=True, unique=True)
    name = CharField(unique=True)
    open_issues_count = IntegerField(default=0)
    private = BooleanField(default=False)
    pushed_at = DateTimeField()
    size = IntegerField(default=0)
    stargazers_count = IntegerField(default=0)
    updated_at = DateTimeField()
    url = CharField()
    watchers_count = IntegerField(default=0)
